#ifndef RADIO_FUNCTIONS_H
#define RADIO_FUNCTINS_H



// radio_functions.cpp
void IRAM_ATTR onDacTimer();
void startDacTone(float frequency);
void stopDacTone();
void send_SPIBit(int Counter, byte length);
void send_SPIEnable();
void SetTone(int toneSTATE);
void Alert_Tone(int ToneType);
void SetPLLLock(uint32_t Frequency);
void write_FRQ(uint32_t Frequency);
void SetRFPower();
void readRfPower();


#endif