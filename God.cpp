#include"transmit.h"
#include"database.h"
#include"report.h"

const int Local_ID = God_ID;

int Tot_Trans;
vector<Trans*> V_Trans;

Trans *Make_Trans()
{
 	Trans *p=new Trans,*np;
 	p->ID=Tot_Trans++;
 	if(p->ID>=BlockSizeLimit) 
 	{
 		//rand PID
 		for(int i=0;i<Trans_PIDSize;i++)
 		{
 			int pre=rand()%(p->ID-BlockSizeLimit+Trans_PIDSize);
 			if(pre/BlockSizeLimit==p->ID/BlockSizeLimit) i--;
 			else if(!count(p->PID.begin(),p->PID.end(),pre))
 				p->PID.push_back(pre);
 			else i--;
 		}
 		//build Data
 		np=V_Trans[p->PID[0]];
 		p->Data=np->Data;
 		for(int i=1;i<Trans_PIDSize;i++)
 		{
 			np=V_Trans[p->PID[i]];
 			for(int j=0;j<Trans_DataSize;j++) 
 			{
 				if(i%3==1) p->Data[j]|=np->Data[j];
 				if(i%3==2) p->Data[j]&=np->Data[j];
 				if(i%3==0) p->Data[j]^=np->Data[j];
 			}
 		}
 	}
 	else
 	{
 		p->Data.resize(Trans_DataSize);
 		for(int i=0;i<Trans_DataSize;i++)
 		{
 			p->Data[i]=(rand()<<15)+rand();
 		}

 	}
 	V_Trans.push_back(p);
 	return p;
}

int main()
{
	int From_ID;string s;

	Transmit_Init(Local_ID);
	cerr<<"God:  Finish Initing (Mashine_Num = "<<Mashine_Num<<")\n";

	char c;
	cerr<<"*******Notice*******\n1.Config Your IP\n2.Config Local_ID\n3.Config Database\n4.Config Mashine_Num\n";
	cerr<<"God:  Are you going to start progress?(Y/N)\n";
	cin>>c;
	if(c!='Y' && c!='y') {cerr<<"Abort\n";system("pause");exit(0);}
	
	PostMessage(Announce_ID);
	ofstream fout(God_File);
	fout<<Local_ID<<" StartRunning\n";
	fout.close();EndPostMessage(God_File);

	srand(time(0));
	for(int i=0;i<=DataScale;i++)
	{
		if(!i) cerr<<"God:   Start to post NewBlock: 0\n";
		if(i && !(i%BlockSizeLimit))
		{
			while(1)
			{
				if(CheckMessage())
				{
					int ID;
					GetMessage(God_File);
					ifstream fin(God_File);
					fin>>From_ID>>s;
					if(s=="CheckBlockSucceed")
					{
						fin>>ID;
						if(ID+1==i/BlockSizeLimit) {cerr<<"God:   Start to post NewBlock: "<<i/BlockSizeLimit<<"\n";break;}
					}
					fin.close();
				}
				#ifndef No_Delay
				Sleep(Delay_Time);
				#endif
			}
		}

		if(i==DataScale) break;

		Trans *p=Make_Trans();
		PostMessage(Server_ID);//change to server
		ofstream fout(God_File);
		fout<<Local_ID<<" NewTrans\n";
		p->write(fout);
		fout.close();EndPostMessage(God_File);
		#ifndef No_Delay
		Sleep(NewTrans_Delay_Time);
		#endif
	}

	PostMessage(Announce_ID);
	fout.open(God_File);
	fout<<Local_ID<<" EndProgress\n";
	fout.close();EndPostMessage(God_File);

	cerr<<"************* BlockChain Project Report *************\n\n";

	double RunTime=0,WrkTime=0;int Mry_P=0,Mry_V=0;
	int Cnt_EndPoc=0;
	while(Cnt_EndPoc<Mashine_Num)
	{
		if(CheckMessage())
		{
			GetMessage(God_File);
			ifstream fin(God_File);
			fin>>From_ID>>s;
			if(s=="ReportFromServer")
			{
				double _RunTime,_WrkTime;int _Mry_P,_Mry_V;
				fin>>s>>_RunTime>>s>>_WrkTime>>s>>_Mry_P>>_Mry_V;
				fin.close();
				RunTime+=_RunTime;WrkTime+=_WrkTime;Mry_P+=_Mry_P;Mry_V+=_Mry_V;
				Cnt_EndPoc++;
				cerr<<"ServerReport:\n";
				cerr<<"Running Time:  "<<_RunTime<<"s\n";
				cerr<<"Working Time:  "<<_WrkTime<<"s\n";
				cerr<<"Using of Physic RAM:    "<<(double)_Mry_P/1048576<<"MB\n";
				cerr<<"Using of Virtual RAM:   "<<(double)_Mry_V/1048576<<"MB\n\n";
				Cnt_EndPoc++;
			}
			else if(s=="ReportFromUser")
			{
				double _RunTime,_WrkTime;int _Mry_P,_Mry_V;
				fin>>s>>_RunTime>>s>>_WrkTime>>s>>_Mry_P>>_Mry_V;
				fin.close();
				RunTime+=_RunTime;WrkTime+=_WrkTime;Mry_P+=_Mry_P;Mry_V+=_Mry_V;
				Cnt_EndPoc++;
			}
			fin.close();
		}
		#ifndef No_Delay
		Sleep(Delay_Time);
		#endif
	}

	cerr<<"FinalReport:\n";
	cerr<<"Averange Running Time:  "<<RunTime/Mashine_Num<<"s\n";
	cerr<<"Averange Working Time:  "<<WrkTime/Mashine_Num<<"s\n";
	cerr<<"Using of Physic RAM:    "<<(double)Mry_P/Mashine_Num/1048576<<"MB\n";
	cerr<<"Using of Virtual RAM:   "<<(double)Mry_V/Mashine_Num/1048576<<"MB\n\n";

	cerr<<"\n****************** Project Complete *****************"<<endl;

	system("pause");

	return 0;
}
