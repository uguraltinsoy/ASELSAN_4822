import os
import re

with open("src/main.cpp", "r") as f:
    content = f.read()

# We will regex search for function definitions
# A simple way is to find "void FuncName" and the matching closing brace.
def extract_function(source, func_name):
    # Find the function start
    pattern = r'(void\s+' + func_name + r'\s*\([^)]*\)\s*\{)'
    match = re.search(pattern, source)
    if not match:
        pattern = r'(void\s+' + func_name + r'\s*\([^)]*\)\s*\n\s*\{)'
        match = re.search(pattern, source)
        
    if not match:
        return "", source
        
    start_idx = match.start()
    
    # find matching brace
    brace_count = 0
    in_string = False
    in_char = False
    escape = False
    
    for i in range(match.end() - 1, len(source)):
        c = source[i]
        if escape:
            escape = False
            continue
        if c == '\\':
            escape = True
            continue
        if c == '"' and not in_char:
            in_string = not in_string
        elif c == "'" and not in_string:
            in_char = not in_char
            
        if not in_string and not in_char:
            if c == '{':
                brace_count += 1
            elif c == '}':
                brace_count -= 1
                if brace_count == 0:
                    func_body = source[start_idx:i+1]
                    new_source = source[:start_idx] + source[i+1:]
                    return func_body, new_source
                    
    return "", source

aprs_funcs = ["set_nada_1200", "set_nada_2400", "set_nada", "calc_crc", "send_crc", "send_header", "send_payload", "send_char_NRZI", "send_string_len", "send_flag", "send_packet"]
eeprom_funcs = ["eeread_nbytes", "eeprom_readAPRS", "eewrite_nbytes", "eeprom_writeAPRS", "initialize_eeprom", "StoreFrequency", "GetMemoryChannel", "GetPrintMemoryChannelInfo", "StoreSpecialFrequency"]
display_funcs = ["InitLCD", "sendToLcd", "writeToLcd", "writeFRQToLcd", "scroll", "Greetings", "numberToFrequency", "write_SHIFTtoLCD", "write_TONEtoLCD"]
menu_funcs = ["PrintMenu", "commandYardim", "commandRadioType", "commandStartupMSG", "commandDumpConfig", "commandMemoryDump", "commandMemoryChannel", "commandAPRSTimeout", "commandAPRSMessage", "commandFrequencyLowerLimit", "commandFrequencyUpperLimit", "commandScanLowerLimit", "commandScanUpperLimit", "commandAprsFrequency", "commandISSFrequency", "commandVNALowerLimit", "commandVNAUpperLimit", "getEEPROMData", "commandAPRSmycall", "commandTogglePTT"]

aprs_code = '#include "main.h"\n\n'
eeprom_code = '#include "main.h"\n\n'
display_code = '#include "main.h"\n\n'
menu_code = '#include "main.h"\n\n'

for f in aprs_funcs:
    body, content = extract_function(content, f)
    aprs_code += body + "\n\n"

for f in eeprom_funcs:
    body, content = extract_function(content, f)
    eeprom_code += body + "\n\n"

for f in display_funcs:
    body, content = extract_function(content, f)
    display_code += body + "\n\n"

for f in menu_funcs:
    body, content = extract_function(content, f)
    menu_code += body + "\n\n"

with open("src/APRS.cpp", "w") as f:
    f.write(aprs_code)

with open("src/EEPROM_Manager.cpp", "w") as f:
    f.write(eeprom_code)

with open("src/Display.cpp", "w") as f:
    f.write(display_code)

with open("src/SerialMenu.cpp", "w") as f:
    f.write(menu_code)

with open("src/main.cpp", "w") as f:
    f.write(content)

