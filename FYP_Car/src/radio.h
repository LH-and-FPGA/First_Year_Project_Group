#ifndef RADIO
#define RADIO

void setupRadio(const int& radioPin, int& radioFreq, unsigned long& lastRadioHigh);
void stopRadio(const int& radioPin);
void onRadioRising();

#endif