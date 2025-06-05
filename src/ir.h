#ifndef IR
#define IR

void setupIR(const int& pin, unsigned long& lastIRHigh);
void stopIR(const int& pin);
void onIRRising();

#endif
