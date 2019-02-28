#ifndef report_h
#define report_h

#include"constant.h"

void   Timer_Init();
void   Timer_Start(int ID);
double Timer_Stop(int ID);
double Timer_Tot(int ID);
void   Timer_Clear(int ID);

pair<size_t,size_t> Memory_Info();

#endif //report_h