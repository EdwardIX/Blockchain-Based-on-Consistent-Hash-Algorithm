#ifndef transmit_h
#define transmit_h

#include<winsock2.h>
#include"constant.h"

/************************in client.cpp************************/

void HttpRequestInit();
int MakePost(int client,int target,string filename);
int GetCount(int client);
void PrintRecord(int client,int pos,string filename);
void HttpRequestClean();

/************************in transmit.cpp************************/

void Transmit_Init(int _local_ID);
bool CheckMessage();
void GetMessage(const char *File);
void PostMessage(int _target_ID);
void EndPostMessage(const char *File);
void Transmit_End();

#endif 