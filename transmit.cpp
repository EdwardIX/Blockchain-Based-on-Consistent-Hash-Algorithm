#include"transmit.h"

int RevMsg,RevAnc,local_ID,target_ID;
int Status;

void Transmit_Init(int _local_ID)
{
	local_ID=_local_ID;
	RevMsg=RevAnc=0;
	HttpRequestInit();
}

bool CheckMessage()
{
	int NowMsg=GetCount(local_ID);
	if(RevMsg<NowMsg) {Status=1;return true;}
	int NowAnc=GetCount(Announce_ID);
	if(RevAnc<NowAnc) {Status=2;return true;}
	return false;
}

void GetMessage(const char *File)
{
	if(Status==1)      PrintRecord(local_ID,++RevMsg,File);
	else if(Status==2) PrintRecord(Announce_ID,++RevAnc,File);
	return;
}

void PostMessage(int _target_ID)
{
	target_ID=_target_ID;
	return;
}

void EndPostMessage(const char *File)
{
	MakePost(local_ID,target_ID,File);
}

void Transmit_End()
{
	HttpRequestClean();
}