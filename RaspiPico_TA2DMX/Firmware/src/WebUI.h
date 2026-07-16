#ifndef WEBUI_H
#define WEBUI_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Updater.h>
#include "main.h"

extern String mycall; // From main.cpp
extern String APRS_Message;
extern unsigned int APRS_Timeout;
extern void eewrite_nbytes(String dst, uint8_t sz, uint16_t addr); // from main.cpp
extern String webLogBuffer;
extern String commandString;
extern bool commandComplete;

WebServer server(80);
bool wifiEnabled = false;

const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8"> 
    <title>ASELSAN 4822 TAMSAT Yönetimi</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #f4f4f9; color: #333; margin: 0; padding: 0; }
        .navbar { background-color: #0056b3; overflow: hidden; color: white; display: flex; justify-content: space-between; padding: 10px 20px; align-items: center;}
        .navbar h2 { margin: 0; font-size: 20px; }
        .nav-links { display: flex; gap: 5px; flex-wrap: wrap; }
        .nav-links button { background: none; border: 1px solid white; color: white; padding: 8px 15px; border-radius: 5px; cursor: pointer; transition: 0.3s; margin-top: 5px;}
        .nav-links button:hover, .nav-links button.active { background: white; color: #0056b3; }
        
        .container { max-width: 800px; margin: 20px auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }
        h1 { color: #0056b3; text-align: center; margin-top: 0;}
        .card { background: #e9ecef; padding: 15px; margin-bottom: 20px; border-radius: 5px; }
        button.btn-success { background: #28a745; color: white; border: none; padding: 10px 15px; border-radius: 5px; cursor: pointer; width: 100%; font-size: 16px; margin-top: 10px; }
        button.btn-success:hover { background: #218838; }
        input[type="text"], input[type="number"], input[type="file"] { width: 100%; padding: 8px; margin-top: 5px; margin-bottom: 10px; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; }
        .footer { text-align: center; margin-top: 20px; font-size: 12px; color: #666; }
        
        .tabcontent { display: none; }
        
        /* Table Styles for Chirp */
        table { width: 100%; border-collapse: collapse; margin-top: 10px; font-size: 14px;}
        th, td { border: 1px solid #ddd; padding: 8px; text-align: center; }
        th { background-color: #f2f2f2; color: #333; }
        td input { width: 100%; box-sizing: border-box; padding: 4px; border: 1px solid #ccc; text-align: center; margin: 0; font-family: monospace;}
        @media screen and (max-width: 600px) {
            .navbar { flex-direction: column; text-align: center; }
            .nav-links { justify-content: center; }
            table { font-size: 12px; }
            th, td { padding: 4px; }
        }
    </style>
</head>
<body>
    <div class="navbar">
        <h2>ASELSAN 4822 <span style="font-size: 12px; font-weight: normal;">%VERSION%</span></h2>
        <div class="nav-links">
            <button class="tablinks active" onclick="openTab(event, 'Ayarlar')">Cihaz Ayarları</button>
            <button class="tablinks" onclick="openTab(event, 'Kanallar')">Kanallar (CHIRP)</button>
            <button class="tablinks" onclick="openTab(event, 'Konsol')">Konsol</button>
            <button class="tablinks" onclick="openTab(event, 'OTA')">Güncelleme</button>
        </div>
    </div>

    <div class="container">
        
        <div id="Ayarlar" class="tabcontent" style="display:block;">
            <h3>Kişisel Ayarlar</h3>
            <form method='POST' action='/saveConfig'>
                <label>Çağrı İşareti (Maks 6 karakter):</label>
                <input type='text' id='callsign' name='callsign' maxlength='6'>
                
                <label>Açılış Mesajı (Maks 6 karakter):</label>
                <input type='text' id='startup' name='startup' maxlength='6'>
                
                <label>APRS Durumu:</label>
                <select id='aprs_enable' name='aprs_enable' style='width: 100%; padding: 8px; margin-bottom: 10px; border-radius: 4px; border: 1px solid #ccc;'>
                    <option value='0'>Pasif (Kapalı)</option>
                    <option value='1'>Aktif (Açık)</option>
                </select>

                <label>APRS Mesajı (Maks 28 karakter):</label>
                <input type='text' id='aprsmsg' name='aprsmsg' maxlength='28'>
                
                <label>APRS Gönderim Aralığı (Dakika):</label>
                <input type='number' id='aprstime' name='aprstime' min='1' max='99'>
                
                <button type='submit' class="btn-success">Ayarları Kaydet</button>
            </form>
            
            <hr style="margin-top:20px; border-top: 1px solid #ccc;">
            <h3 style="color:#dc3545;">Tehlikeli İşlemler</h3>
            <p style="font-size: 14px;">Cihaz kilitlenirse veya hatalı çalışırsa EEPROM'u sıfırlayabilirsiniz. Bu işlem kayıtlı her şeyi siler.</p>
            <form method='POST' action='/factoryReset' onsubmit="return confirm('Tüm kanallar ve ayarlar silinecek! Onaylıyor musunuz?');">
                <button type='submit' style="background-color: #dc3545; color: white; padding: 10px; border: none; border-radius: 4px; cursor: pointer; width: 100%;">EEPROM'u Sıfırla (Fabrika Ayarlarına Dön)</button>
            </form>
        </div>

        <div id="Kanallar" class="tabcontent">
            <h3>Kanal Listesi (Hafıza Yönetimi)</h3>
            <p>Frekansları <b>KHz</b> cinsinden giriniz (Örn: 145.500 MHz = 145500). Kaydetmek için en alttaki butona basınız.</p>
            <div style="overflow-x:auto;">
                <table>
                    <thead>
                        <tr>
                            <th>CH</th>
                            <th>Frekans (KHz)</th>
                            <th>Shift (KHz)</th>
                            <th>Ton İndeks</th>
                            <th>İsim (7 Karakter)</th>
                        </tr>
                    </thead>
                    <tbody id="channelBody">
                        <tr><td colspan="6">Kanallar yükleniyor...</td></tr>
                    </tbody>
                </table>
            </div>
            <div style="margin-top: 15px; display: flex; justify-content: space-between;">
                <button class="btn-success" onclick="addChannel()" style="background-color: #007bff;">+ Yeni Kanal Ekle</button>
                <button class="btn-success" onclick="saveChannels()" id="btnSave">Telsize Kaydet</button>
            </div>
            <p id="channelSaveStatus" style="color: green; text-align: center; font-weight: bold; height: 20px; margin-top: 10px;"></p>
        </div>

        <div id="OTA" class="tabcontent">
            <h3>OTA Yazılım Güncelleme</h3>
            <p>Lütfen PlatformIO'dan aldığınız <b>firmware.bin</b> dosyasını seçin.</p>
            <form method='POST' action='/update' enctype='multipart/form-data'>
                <input type='file' name='update' required>
                <button type='submit' class="btn-success">Güncelle (.bin)</button>
            </form>
        </div>

        <div id="Konsol" class="tabcontent">
            <h3>Seri Port Logları</h3>
            <textarea id="logArea" style="width:100%; height:200px; background:#000; color:#0f0; font-family:monospace;" readonly></textarea>
            <form onsubmit="sendCommand(event)">
                <input type="text" id="cmdInput" placeholder="Komut yazın (örn: Y) ve Enter'a basın" autocomplete="off" required>
                <button type="submit" class="btn-success">Gönder</button>
            </form>
        </div>

        <div class="footer">
            TAMSAT & TA2DMX &copy; 2026
        </div>
    </div>
    
    <script>
    document.addEventListener("DOMContentLoaded", function() {
        fetch('/getConfig')
            .then(res => res.json())
            .then(data => {
                document.getElementById('callsign').value = data.callsign;
                document.getElementById('startup').value = data.startup;
                document.getElementById('aprs_enable').value = data.aprs_enable;
                document.getElementById('aprsmsg').value = data.aprsmsg;
                document.getElementById('aprstime').value = data.aprstime;
            }).catch(e => console.log(e));
    });

    function openTab(evt, tabName) {
      var i, tabcontent, tablinks;
      tabcontent = document.getElementsByClassName("tabcontent");
      for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].style.display = "none";
      }
      tablinks = document.getElementsByClassName("tablinks");
      for (i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" active", "");
      }
      document.getElementById(tabName).style.display = "block";
      evt.currentTarget.className += " active";
      
      if(tabName === 'Kanallar') {
          loadChannels();
      }
    }
    
    function updateLogs() {
        if(document.getElementById('Konsol').style.display === 'block') {
            fetch('/getLogs').then(response => response.text()).then(data => {
                var logArea = document.getElementById('logArea');
                if(logArea.value !== data) {
                    logArea.value = data;
                    logArea.scrollTop = logArea.scrollHeight;
                }
            }).catch(err => console.log(err));
        }
    }
    setInterval(updateLogs, 1000);
    
    function sendCommand(e) {
        e.preventDefault();
        var cmd = document.getElementById('cmdInput').value;
        fetch('/sendCommand', {
            method: 'POST',
            headers: {'Content-Type': 'application/x-www-form-urlencoded'},
            body: 'cmd=' + encodeURIComponent(cmd)
        }).then(() => {
            document.getElementById('cmdInput').value = '';
            setTimeout(updateLogs, 500);
        });
    }

    let channelsData = [];
    function loadChannels() {
        fetch('/getChannels')
            .then(res => res.json())
            .then(data => {
                channelsData = data;
                renderChannels();
            })
            .catch(err => console.error("Kanallar alinamadi: ", err));
    }

    function createRow(i, ch) {
        let f = ch ? ch.f : 145500;
        let s = ch ? ch.s : 600;
        let t = ch ? ch.t : 8;
        let n = ch ? ch.n : ("CH-" + (i+1));
        return `<tr id="row_${i}">
            <td>${i+1}</td>
            <td><input type="number" id="f_${i}" value="${f}"></td>
            <td><input type="number" id="s_${i}" value="${s}"></td>
            <td><input type="number" id="t_${i}" value="${t}"></td>
            <td><input type="text" id="n_${i}" maxlength="7" value="${n}"></td>
            <td><button onclick="deleteRow(${i})" style="padding: 4px 8px; background: #dc3545; color: white; border: none; border-radius: 3px; cursor: pointer;">Sil</button></td>
        </tr>`;
    }

    function renderChannels() {
        let html = '';
        for(let i=0; i<channelsData.length; i++) {
            let ch = channelsData[i];
            // Consider empty if frequency is 0xFFFF * 25 (1638375) or 0
            if (ch.f !== 0 && ch.f !== 1638375 && ch.f !== 4294967295) {
                html += createRow(i, ch);
            }
        }
        if (html === '') {
            html = '<tr><td colspan="6" style="text-align:center;">Kayitli kanal yok. Lutfen ekleyin.</td></tr>';
        }
        document.getElementById('channelBody').innerHTML = html;
    }

    function addChannel() {
        let tbody = document.getElementById('channelBody');
        if (tbody.innerHTML.includes('Kayitli kanal yok')) {
            tbody.innerHTML = '';
        }
        for(let i=0; i<50; i++) {
            if (!document.getElementById(`row_${i}`)) {
                tbody.insertAdjacentHTML('beforeend', createRow(i, null));
                break;
            }
        }
    }

    function deleteRow(i) {
        let row = document.getElementById(`row_${i}`);
        if (row) row.remove();
    }

    function saveChannels() {
        let btn = document.getElementById('btnSave');
        btn.innerText = "Kaydediliyor...";
        btn.disabled = true;
        
        let params = new URLSearchParams();
        for(let i=0; i<50; i++) {
            let row = document.getElementById(`row_${i}`);
            if (row) {
                params.append('f_' + i, document.getElementById(`f_${i}`).value);
                params.append('s_' + i, document.getElementById(`s_${i}`).value);
                params.append('t_' + i, document.getElementById(`t_${i}`).value);
                params.append('n_' + i, document.getElementById(`n_${i}`).value);
            } else {
                params.append('f_' + i, '1638375');
                params.append('s_' + i, '0');
                params.append('t_' + i, '0');
                params.append('n_' + i, '');
            }
        }
        fetch('/saveChannels', {
            method: 'POST',
            body: params
        }).then(res => {
            let status = document.getElementById('channelSaveStatus');
            btn.innerText = "Telsize Kaydet";
            btn.disabled = false;
            if(res.ok) {
                status.innerText = "Başarılı! Değişikliklerin uygulanması için telsizi yeniden başlatın.";
                setTimeout(() => { status.innerText = ""; }, 5000);
            } else {
                status.style.color = "red";
                status.innerText = "Hata oluştu!";
                setTimeout(() => { status.innerText = ""; status.style.color = "green"; }, 3000);
            }
        });
    }
    </script>
</body>
</html>
)rawliteral";

void handleRoot() {
    String page = String(htmlPage);
    String versionString = "v" + String(SW_MAJOR) + "." + String(SW_MINOR);
    page.replace("%VERSION%", versionString);
    server.send(200, "text/html", page);
}

void initWebUI() {
    if (wifiEnabled) return;
    
    Serial.println("Starting WiFi AP...");
    String ssid = "ASELSAN_" + String(mycall);
    
    // Start SoftAP
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid.c_str(), ssid.c_str()); 
    
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP adresi: ");
    Serial.println(IP);

    server.on("/", HTTP_GET, handleRoot);

    server.on("/getConfig", HTTP_GET, []() {
        String startupMsg = "";
        for(int i=0; i<6; i++) {
            char c = EEPROM.read(9 + i);
            if(c >= 32 && c <= 126) startupMsg += c;
        }
        String json = "{";
        json += "\"callsign\":\"" + mycall + "\",";
        json += "\"startup\":\"" + startupMsg + "\",";
        json += "\"aprs_enable\":\"" + String(APRS_Timeout > 0 ? 1 : 0) + "\",";
        json += "\"aprsmsg\":\"" + APRS_Message + "\",";
        json += "\"aprstime\":" + String(APRS_Timeout > 0 ? APRS_Timeout : 10);
        json += "}";
        server.send(200, "application/json", json);
    });

    server.on("/saveConfig", HTTP_POST, []() {
        if (server.hasArg("callsign")) {
            mycall = server.arg("callsign");
            eewrite_nbytes(mycall, 6, 3);
        }
        if (server.hasArg("startup")) {
            String startup = server.arg("startup");
            eewrite_nbytes(startup, 6, 9);
        }
        if (server.hasArg("aprsmsg")) {
            APRS_Message = server.arg("aprsmsg");
            eewrite_nbytes(APRS_Message, 28, 20); 
        }
        
        if (server.hasArg("aprs_enable")) {
            if (server.arg("aprs_enable") == "0") {
                APRS_Timeout = 0;
            } else {
                if (server.hasArg("aprstime")) {
                    APRS_Timeout = server.arg("aprstime").toInt();
                    if (APRS_Timeout <= 0) APRS_Timeout = 10;
                }
            }
            EEPROM.write(66, APRS_Timeout); 
        }

        EEPROM.commit(); 
        
        server.sendHeader("Location", "/", true);
        server.send(302, "text/plain", "Ayarlar Kaydedildi!");
    });

    server.on("/factoryReset", HTTP_POST, []() {
        EEPROM.write(0, 129); // Change magic byte to force reset on boot
        EEPROM.commit();
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", "<html><head><meta charset='UTF-8'></head><body style='text-align:center; padding:50px; font-family:sans-serif;'><h2>EEPROM Sıfırlandı!</h2><p>Cihaz yeniden başlatılıyor ve fabrika ayarlarına dönülüyor...</p></body></html>");
        delay(1000);
        rp2040.reboot();
    });

    server.on("/getChannels", HTTP_GET, []() {
        String json = "[";
        for (int ch=0; ch<50; ch++) {
            memorych_t m;
            uint16_t loc = EEPROM_MEMDATA_BLCKSTART + (ch + 1) * EEPROM_CHNNL_SIZE;
            EEPROM.get(loc, m);
            json += "{\"f\":";
            json += String((uint32_t)m.frequency25 * 25);
            json += ",\"s\":";
            json += String((uint16_t)m.shift25 * 25);
            json += ",\"t\":";
            json += String(m.tone_position);
            json += ",\"n\":\"";
            for(int i=0; i<7; i++) {
                if(m.ChannelName[i] >= 32 && m.ChannelName[i] <= 126 && m.ChannelName[i] != '"' && m.ChannelName[i] != '\\') {
                    json += m.ChannelName[i];
                } else if (m.ChannelName[i] == 0) {
                    break;
                }
            }
            json += "\"}";
            if (ch < 49) json += ",";
        }
        json += "]";
        server.send(200, "application/json", json);
    });

    server.on("/saveChannels", HTTP_POST, []() {
        for(int i=0; i<50; i++) {
            String prefix = String(i);
            if(server.hasArg("f_" + prefix)) {
                uint32_t freq = server.arg("f_" + prefix).toInt();
                uint16_t shift = server.arg("s_" + prefix).toInt();
                uint8_t tone = server.arg("t_" + prefix).toInt();
                String name = server.arg("n_" + prefix);
                
                memorych_t m;
                // Make sure to read current values first so we don't overwrite SSTP incorrectly
                uint16_t loc = EEPROM_MEMDATA_BLCKSTART + (i + 1) * EEPROM_CHNNL_SIZE;
                EEPROM.get(loc, m);
                
                m.frequency25 = freq / 25;
                m.shift25 = shift / 25;
                m.tone_position = tone;
                
                for(int j=0; j<7; j++) {
                    if(j < name.length()) m.ChannelName[j] = name[j];
                    else m.ChannelName[j] = ' ';
                }
                
                EEPROM.put(loc, m);
            }
        }
        EEPROM.commit();
        server.send(200, "text/plain", "OK");
    });

    server.on("/update", HTTP_POST, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", (Update.hasError()) ? "Guncelleme BASARISIZ!" : "Guncelleme BASARILI! Cihaz yeniden baslatiliyor...");
        delay(1000);
        rp2040.reboot();
    }, []() {
        HTTPUpload& upload = server.upload();
        if (upload.status == UPLOAD_FILE_START) {
            Serial.printf("Update: %s\n", upload.filename.c_str());
            if (!Update.begin(upload.totalSize > 0 ? upload.totalSize : 2000000)) { 
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) { 
                Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
            } else {
                Update.printError(Serial);
            }
        }
    });

    server.begin();
    
    server.on("/getLogs", HTTP_GET, []() {
        server.send(200, "text/plain", webLogBuffer);
    });
    
    server.on("/sendCommand", HTTP_POST, []() {
        if(server.hasArg("cmd")) {
            commandString = server.arg("cmd") + "\n";
            commandComplete = true;
        }
        server.send(200, "text/plain", "OK");
    });
    
    Serial.println("HTTP sunucusu basladi.");
    wifiEnabled = true;
}

void handleWebUI() {
    if (wifiEnabled) {
        server.handleClient();
    }
}

#endif
