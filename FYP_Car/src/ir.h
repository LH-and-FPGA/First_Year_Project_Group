#ifndef IR
#define IR

void setupIR(const int& pin, unsigned long& lastHigh, int& freq);
void stopIR(const int& pin);
void onIRRising();

#endif