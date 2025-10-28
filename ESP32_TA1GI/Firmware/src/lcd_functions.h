#ifndef LCD_FUNCTIONS_H
#define LCD_FUNCTIONS_H


// lcd_functions.cpp
void InitLCD();
void sendToLcd(byte *data, byte position);
void writeToLcd(const char text[]);
void writeFRQToLcd(const char frq[9]);
void scroll(const char *text, int speed);
void displayAndScrollOnce(const char* text);
void Greetings();
void write_SHIFTtoLCD(uint16_t FRQshift);
void write_TONEtoLCD(unsigned long tone_pos);




#endif