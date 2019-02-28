#include"report.h"
#include<windows.h>
#include<psapi.h>

LARGE_INTEGER nFreq;
LARGE_INTEGER nBeginTime[Max_Clock_Num];
LARGE_INTEGER nEndTime[Max_Clock_Num];
double Tot_Time[Max_Clock_Num];

void   Timer_Init(){QueryPerformanceFrequency(&nFreq);}
void   Timer_Start(int ID){QueryPerformanceCounter(&nBeginTime[ID]);}
double Timer_Stop(int ID)
{
	QueryPerformanceCounter(&nEndTime[ID]);
	double t=(double)(nEndTime[ID].QuadPart-nBeginTime[ID].QuadPart)/nFreq.QuadPart;
	Tot_Time[ID]+=t;
	return t;
}
double Timer_Tot(int ID){return Tot_Time[ID];}
void   Timer_Clear(int ID){Tot_Time[ID]=0;}

pair<size_t,size_t> Memory_Info()
{
	HANDLE handle = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(handle,&pmc,sizeof(pmc));
	return make_pair(pmc.PeakWorkingSetSize,pmc.PeakPagefileUsage);
}