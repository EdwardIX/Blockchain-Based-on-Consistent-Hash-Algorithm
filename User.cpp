#include"transmit.h"
#include"database.h"
#include"report.h"

int Local_ID;
char User_File[20];

map<int,int> M_Trans;//the key-value type to store the Trans is in which block
vector<Block*> V_Block;//the vector to store Blocks
set<int> S_CkBlk;

class RequireMessage{
public:
	map<Trans*,Block*> M;
	Trans* p;
	~RequireMessage(){M.clear();delete p;}
};
map<int,RequireMessage*> M_Req;

//find the Trans as well as the Block
//Only check Local data
Trans *Find_Trans(int ID,Block*& blo)
{
	int pos;map<int,int>::iterator it=M_Trans.find(ID);
	if(it==M_Trans.end()) return NULL;
	if(it->second>(int)V_Block.size() || !(blo=V_Block[it->second]) || !~(pos=blo->find(ID))) return NULL;
	return &(blo->V[pos]);
}

void CheckBlockSucceed(Block *blo)
{
	if((int)V_Block.size()<=blo->ID) V_Block.resize(blo->ID+1);V_Block[blo->ID]=blo;
	for(size_t i=0;i<blo->V.size();i++) M_Trans[blo->V[i].ID]=blo->ID;
	blo->build();

	ofstream fout(User_File);PostMessage(Server_ID);
	fout<<Local_ID<<" CheckBlockResult\nSucceed "<<blo->ID;
	fout.close();EndPostMessage(User_File);
	cerr<<"User:  ****** CheckBlockSucceed ****** \n";
}
void CheckBlockFailed(Block* blo)
{
	ofstream fout(User_File);PostMessage(Server_ID);
	fout<<Local_ID<<" CheckBlockResult\nFailed "<<blo->ID;
	fout.close();EndPostMessage(User_File);
	delete blo;
}

int Recheck_Trans(Trans *p,Block *blo,int &DelMsg)
{
	if(++p->CheckStatus<(int)p->PID.size()) return 0;
	//cerr<<"\nStarting Recheck Trans: "<<p->ID<<endl;
	vector<int> tData(p->Data.size());
	for(size_t i=0;i<p->PID.size();i++) 
	{
		Block* nblo;Trans* np=Find_Trans(p->PID[i],nblo);
		map<int,RequireMessage*>::iterator it=M_Req.end();
		if(!np) 
		{
			it=M_Req.find(p->PID[i]);
			if(it==M_Req.end() || !(np=it->second->p) || np->CheckStatus<=Check_Num) 
				{cerr<<"Recheck_Trans:   ****** ERROR: no received or checked trans when calling *******\n";system("pause");}
			it->second->M.erase(p);//checked
		}
		for(size_t j=0;j<p->Data.size();j++) 
		{
			if(!i)          tData[j]=np->Data[j];
			else if(i%3==1) tData[j]|=np->Data[j];
			else if(i%3==2) tData[j]&=np->Data[j];
			else if(i%3==0) tData[j]^=np->Data[j];
		}
		if(it!=M_Req.end() && it->second->M.empty()) {if(it->first==DelMsg) DelMsg=-1;delete it->second;M_Req.erase(it);}
	}
	if(!p->PID.size() || tData==p->Data) 
	{
		blo->CheckStatus++;
		if(blo->CheckStatus<0) CheckBlockFailed(blo);
		else if(blo->CheckStatus>=(int)blo->V.size()) CheckBlockSucceed(blo);
		//cerr<<"(Suc) ";
		return 1;
	}
	cerr<<"Recheck_Trans:    ****** ERROR ******\n";
	blo->CheckStatus=-INF;return p->CheckStatus=-1;
}

void Receive_Trans(ifstream& fin)
{
	Trans *p=new Trans;
	p->read(fin);
	if(!p->check_File(fin)) {cerr<<"Receive_Trans:   ****** ERROR: failed checking trans from file ******\n";system("pause");return;}

	map<int,RequireMessage*>::iterator it=M_Req.find(p->ID);
	if(it==M_Req.end()){cerr<<"Receive_Trans:   ****** ERROR: not Required message ******\n";system("pause");return;}

	if(!it->second->p) it->second->p=p,p->CheckStatus=1;
	else if(*it->second->p==*p) {it->second->p->CheckStatus++;delete p;}
	else {cerr<<"Receive_Trans:   ******ERROR: Receive two different Trans ******\n";system("pause");return;}

	//call for check
	if(it->second->p->CheckStatus>=Check_Num+1)
	{
		int DelMsg=it->first;
		//cerr<<it->first<<' '<<it->second->p->ID<<endl;
		map<Trans*,Block*>::iterator tit=it->second->M.begin(),nxit;
		/*for(;tit!=it->second->M.end();tit=nxit) 
		{
			nxit=tit;nxit++;
			cerr<<tit->first->ID<<' ';
		}
		cerr<<'\n';
		tit=it->second->M.begin();*/
		for(;tit!=it->second->M.end();tit=nxit) 
		{
			nxit=tit;nxit++;
			//cerr<<tit->first->ID<<' ';
			Recheck_Trans(tit->first,tit->second,DelMsg);
			if(!~DelMsg) break;
		}
	 	//cerr<<'\n';
	}
}

//-1 Unlegal 1 legal 0 Unchecked
//for each Trans, it should only be checked once
int Check_Trans(Trans* p,Block* blo)
{
	p->CheckStatus=0;
	for(size_t i=0;i<p->PID.size();i++) 
	{
		Block* nblo;Trans* np=Find_Trans(p->PID[i],nblo);
		if(np) p->CheckStatus++;
		else //sending Require message
		{
			map<int,RequireMessage*>::iterator it=M_Req.find(p->PID[i]);
			if(it==M_Req.end())
			{
				M_Req[p->PID[i]]=new RequireMessage;
				it=M_Req.find(p->PID[i]);
				//cerr<<"insert "<<p->ID<<"("<<p<<") in "<<it->first<<"\n";
				it->second->M[p]=blo;
				it->second->p=NULL;

				ofstream fout(User_File);PostMessage(Server_ID);
				fout<<Local_ID<<" GetTrans\n"<<p->PID[i];
				fout.close();EndPostMessage(User_File);
			}
			else
			{
				//cerr<<"insert "<<p->ID<<" in "<<it->first<<"\n";
				it->second->M[p]=blo;
			}
		}
	}
	if(p->CheckStatus<(int)p->PID.size()) return 0;
	int tmp=-1;return Recheck_Trans(p,blo,tmp);
}

void Respond_Trans(ifstream& fin)
{
	int Trans_ID,Target_ID;
	fin>>Trans_ID>>Target_ID;fin.close();

	Block* blo;Trans* p=Find_Trans(Trans_ID,blo);
	if(!p) {cerr<<"User:   ****** Respond_Trans: cannot find trans"<<Trans_ID<<" ******\n";system("pause");return;}

	PostMessage(Target_ID);ofstream fout(User_File);
	fout<<Local_ID<<" GetTransResult\n";
	p->write(fout);p->print_File(fout,blo);
	fout.close();EndPostMessage(User_File);
}

void check_Checker_Status()
{
	set<int>::iterator it=S_CkBlk.begin();
	while(it!=S_CkBlk.end()) 
	{
		//cerr<<"RUN check_Checker_Status\n";
		if(PoW_If_Finish(*it))
		{
			PostMessage(Server_ID);
			EndPostMessage(Block_File);
			it=S_CkBlk.erase(it);
		}
		else it++;
	}
}

void PoW_Block(ifstream& fin)
{
	Block* blo=new Block;
	blo->read(fin);fin.close();
	blo->CheckStatus=0;

	for(size_t i=0;i<blo->V.size();i++) Check_Trans(&(blo->V[i]),blo);
}

void Receive_Block(int Block_ID)
{
	PoW_Stop(Block_ID);S_CkBlk.erase(Block_ID);
	if((int)V_Block.size()<=Block_ID) V_Block.resize(Block_ID+1);
}

void Report()
{
	PostMessage(God_ID);
	ofstream fout(User_File);pair<size_t,size_t> p=Memory_Info();
	fout<<Local_ID<<" ReportFromUser\n";
	fout<<"Running_Time: "<<Timer_Stop(0);
	fout<<"\nWorking_Time: "<<Timer_Tot(1);
	fout<<"\nMemory_Info: "<<p.first<<' '<<p.second<<endl;
	fout<<'\n';
	fout.close();
	EndPostMessage(User_File);
}

int main()
{
	string s;int From_ID;

	cerr<<"User:   Input Local_ID:\n";cin>>Local_ID;
	sprintf(User_File,"User%d_tmp.txt",Local_ID);
	Transmit_Init(Local_ID);Timer_Init();
	cerr<<"User:   Finish Initing (Local_ID = "<<Local_ID<<")\n";

	//check if all fin is closed
	while(1)
	{
		if(CheckMessage())
		{
			Timer_Start(1);
			GetMessage(User_File);
			ifstream fin(User_File);
			fin>>From_ID>>s;
			//cerr<<s<<'\n';
			if(s=="StartRunning")
			{
				Timer_Clear(0);Timer_Clear(1);
				Timer_Start(0);
				fin.close();
			}else
			if(s=="RespondTrans")
			{
				Respond_Trans(fin);
				//cerr<<"End RespondTrans\n";
			}else
			if(s=="GetTransResult")
			{
				Receive_Trans(fin);
			}else	
			if(s=="NewBlock")
			{
				int Block_ID;
				fin>>Block_ID;S_CkBlk.insert(Block_ID);
				fin.close();
				system((string("del ")+Checker_File).c_str());
				system((string("rename ")+User_File+" "+Checker_File).c_str());
				PoW_Start(Block_ID,Local_ID);
				//cerr<<"End NewBlock\n";
			}else
			if(s=="PoWBlock")
			{
				PoW_Block(fin);
			}else
			if(s=="CheckBlockSucceed")
			{
				int Block_ID;fin>>Block_ID;
				Receive_Block(Block_ID);
			}else
			if(s=="EndProgress")
			{
				Report();
				Transmit_End();
				exit(0);
			}
			else cerr<<"User:  ERROR:  Unreconized Commend: "<<s<<'\n';
			Timer_Stop(1);
		}
		check_Checker_Status();
		#ifndef No_Delay
		Sleep(Delay_Time);
		#endif
	}

	return 0;
}