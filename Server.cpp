#include"transmit.h"
#include"database.h"
#include"report.h"

const int Local_ID = Server_ID;

map<int,int> M_Trans;//store the Trans is in which block
vector<Block*> V_Block;//the vector to store Blocks
map<string,int> M_User;
queue<Trans*> Q_Trans;

void Program_Init()
{
	int i;
	for(i=2;i<Mashine_Num;i++) M_User[SHA256(to_string(i))]=i;
}

//find the Trans as well as the Block
Trans *Find_Trans(int ID,Block*& Blo)
{
	map<int,int>::iterator it=M_Trans.find(ID);int t;
	if(it==M_Trans.end()) return NULL;
	if(it->second>(int)V_Block.size() || !(Blo=V_Block[it->second])) return NULL;
	if(!~(t=Blo->find(ID))) return NULL;
	return &(Blo->V[t]);
}

int Check_Trans(Trans* p)
{
	vector<int> tData(p->Data.size());
	for(size_t i=0;i<p->PID.size();i++) 
	{
		Block* nblo;Trans* np=Find_Trans(p->PID[i],nblo);
		if(!np || !np->check_Local(nblo)) 
		{
			cerr<<"Server:  ********* Check_Trans Unchecked *********\n";
			system("pause");return p->CheckStatus=0;
		}
		for(size_t j=0;j<p->Data.size();j++) 
		{
			if(!i)          tData[j]=np->Data[j];
			else if(i%3==1) tData[j]|=np->Data[j];
			else if(i%3==2) tData[j]&=np->Data[j];
			else if(i%3==0) tData[j]^=np->Data[j];
		}
	}
	if(!p->PID.size() || tData==p->Data) return p->CheckStatus=1;
	cerr<<"Server:  ********* Check_Trans ERROR *********\n";
	system("pause");return p->CheckStatus=-1;
}

void Receive_Trans(ifstream& fin)
{
	Trans* p=new Trans;
	p->read(fin);fin.close();
	if(Check_Trans(p)==-1) return;
	Q_Trans.push(p);
}

bool Build_Block()
{
	//cerr<<"Start to Build_Block\n";
	Block* lstblo=(V_Block.size()?*V_Block.rbegin():NULL);Trans* p;
	if(!lstblo || lstblo->V.size()>=BlockSizeLimit) 
	{
		V_Block.push_back(lstblo=new Block);
		lstblo->ID=V_Block.size()-1;
	}

	while(lstblo->V.size()<BlockSizeLimit && !Q_Trans.empty())
	{
		p=Q_Trans.front();
		if(!p->CheckStatus) 
		{
			Check_Trans(p);
			if(p->CheckStatus==-1) {Q_Trans.pop();continue;}
			else if(!p->CheckStatus) return false;
		}
		lstblo->V.push_back(*p);delete p;
		Q_Trans.pop();
	}

	if(lstblo->V.size()>=BlockSizeLimit) 
	{
		ofstream fout(Server_File);PostMessage(Announce_ID);
		fout<<Local_ID<<" NewBlock\n";
		lstblo->write(fout);
		fout.close();EndPostMessage(Server_File);
		return true;
	}
	return false;
}

bool Check_Block(Block* blo)
{
	if(!blo->PoW_Check()) {blo->CheckStatus=-1;return false;}
	for(size_t i=0;i<blo->V.size();i++)
	{
		Check_Trans(&(blo->V[i]));
		if(!blo->V[i].CheckStatus) 
		{
			cerr<<"Server:    ******* ERROR: Building A Block While the last one is not finished *******\n";
			blo->CheckStatus=0;
			return false;
		}
		if(!~blo->V[i].CheckStatus) 
		{
			blo->CheckStatus=-1;
			return false;
		}
	}
	blo->CheckStatus=1;
	return true;
}

void Receive_Block(ifstream& fin)
{
	Block* blo=new Block;
	blo->read(fin);fin.close();
	if((int)V_Block.size()>blo->ID && V_Block[blo->ID]->CheckStatus>0){cerr<<"Turn down a block\n";return;}
	blo->build();

	map<string,int>::iterator it=M_User.lower_bound(blo->hash());
	for(int i=0;i<Store_Num;i++)
	{
		if(it==M_User.end()) it=M_User.begin();
		PostMessage(it->second);
		EndPostMessage(Server_File);
	}

	if(!Check_Block(blo))
	{
		cerr<<"Server:   ******* ERROR: Receive an Wrong Block *******\n";
		system("pause");
	}
	else {if((int)V_Block.size()<=blo->ID) V_Block.resize(blo->ID+1);V_Block[blo->ID]=blo;}//temply store
}

void Confirm_Block(Block* blo,int From_ID)//attention: if one User send Two messages, it will go wrong
{
	blo->CheckStatus++;
	if(blo->CheckStatus<=Store_Num) return;//when it >=Store_Num+1,then it is legal

	PostMessage(Announce_ID);ofstream fout(Server_File);
	fout<<Local_ID<<" CheckBlockSucceed\n"<<blo->ID<<endl;
	fout.close();EndPostMessage(Server_File);

	for(size_t i=0;i<blo->V.size();i++) M_Trans[blo->V[i].ID]=blo->ID;blo->build();
}

void Get_Trans(int Trans_ID,int From_ID)
{
	Block* blo;Trans* p;
	if(!(p=Find_Trans(Trans_ID,blo))) {cerr<<"Server:   ****** Get_Trans: cannot find trans"<<Trans_ID<<" ******\n";return;}
	
	map<string,int>::iterator it=M_User.lower_bound(blo->hash());ofstream fout;
	for(int i=0;i<Check_Num;i++)
	{
		if(it==M_User.end()) it=M_User.begin();
		PostMessage(it->second);fout.open(Server_File);
		fout<<Local_ID<<" RespondTrans\n"<<Trans_ID<<' '<<From_ID<<'\n';
		fout.close();EndPostMessage(Server_File);
	}

	PostMessage(From_ID);fout.open(Server_File);
	fout<<Local_ID<<" GetTransResult\n";
	p->write(fout);p->print_File(fout,blo);
	fout.close();EndPostMessage(Server_File);
}

void Report()
{
	PostMessage(God_ID);
	ofstream fout(Server_File);pair<size_t,size_t> p=Memory_Info();
	fout<<Local_ID<<" ReportFromServer\n";
	fout<<"Running_Time: "<<Timer_Stop(0);
	fout<<"\nWorking_Time: "<<Timer_Tot(1);
	fout<<"\nMemory_Info: "<<p.first<<' '<<p.second<<endl;
	fout<<'\n';
	fout.close();
	EndPostMessage(Server_File);
}

int main()
{
	string s;int From_ID;

	Transmit_Init(Local_ID);
	Program_Init();
	Timer_Init();
	cerr<<"Server:   Finish Initing (Local_ID = "<<Local_ID<<") \n";

	char c;cerr<<"Server:   Input any char to Start Server\n";cin>>c;

	while(1)
	{	
		if(CheckMessage())
		{
			Timer_Start(1);
			GetMessage(Server_File);
			ifstream fin(Server_File);
			fin>>From_ID>>s;
			//cerr<<s<<'\n';
			if(s=="StartRunning")
			{
				Timer_Clear(0);Timer_Clear(1);
				Timer_Start(0);
				fin.close();
			}else  
			if(s=="NewTrans")
			{
				Receive_Trans(fin);
			}else
			if(s=="GetTrans")
			{
				int Trans_ID;
				fin>>Trans_ID;fin.close();
				Get_Trans(Trans_ID,From_ID);
			}else
			if(s=="PoWBlock")
			{
				cerr<<"Server:    Start Checking PoWBlock\n";
				Receive_Block(fin);
			}else
			if(s=="CheckBlockResult")
			{
				int Block_ID;
				fin>>s>>Block_ID;fin.close();
				if(Block_ID>=(int)V_Block.size() || V_Block[Block_ID]==NULL) cerr<<"Server:   ******* ERROR: User Sending Strange Block ******\n";
				if(s=="Succeed") Confirm_Block(V_Block[Block_ID],From_ID);
				else cerr<<"Server:   ******* ERROR: User Checking Block Failed *******\n";
			}else
			if(s=="EndProgress")
			{
				Report();
				Transmit_End();
				fin.close();
				exit(0);
			}else 
			if(s=="CheckBlockSucceed" || s=="NewBlock") continue;
			else cerr<<"Server:  ERROR:  Unreconized Commend: "<<s<<'\n';

			while(Build_Block());
			Timer_Stop(1);
		}
		#ifndef No_Delay
		Sleep(Delay_Time);
		#endif
	}

	return 0;
}