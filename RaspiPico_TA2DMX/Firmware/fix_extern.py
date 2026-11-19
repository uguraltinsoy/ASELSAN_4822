import re

with open("src/main.h", "r") as f:
    lines = f.readlines()

new_lines = []
in_func = False
for line in lines:
    stripped = line.strip()
    
    if stripped.startswith("void ") or stripped.startswith("byte ") or stripped.startswith("/*") or stripped.startswith("//") or stripped.startswith("#"):
        new_lines.append(line)
        continue
    
    if "{" in stripped and "}" in stripped and "struct" not in stripped:
        pass # array initialization maybe
        
    if "void InitLCD()" in line or "void sendToLcd" in line:
        in_func = True
        
    if in_func:
        new_lines.append(line)
        continue
        
    if re.match(r"^(uint8_t|uint16_t|int|bool|boolean|float|long|char|unsigned|String|channel_t|freqLimits_t|const char|unsigned char)\s+", line) and not line.startswith("const char font_index[] ="):
        line = "extern " + line.split("=")[0].strip() + ";\n"
        # special cases
        if "font_index" in line:
            line = "extern const char font_index[];\n"
        elif "numbers" in line:
            line = "extern const char numbers[];\n"
        elif "keymap" in line:
            line = "extern const char* keymap[4];\n"
        elif "chr2wr" in line:
            line = "extern unsigned char chr2wr[3];\n"
        elif "matrix" in line:
            line = "extern unsigned char matrix[24];\n"
        elif "Position_Signs" in line:
            line = "extern uint8_t Position_Signs[8][3];\n"
        elif "FRQ[" in line:
            line = "extern char FRQ[9];\n"
        elif "FRQ_old[" in line:
            line = "extern char FRQ_old[9];\n"
        elif "ctcss_tone_list[" in line:
            line = "extern float ctcss_tone_list[TOTAL_TONES];\n"
            
    if line.startswith("const char font_index[] ="):
        line = "extern const char font_index[];\n"
        
    new_lines.append(line)

with open("src/main.h", "w") as f:
    f.writelines(new_lines)
