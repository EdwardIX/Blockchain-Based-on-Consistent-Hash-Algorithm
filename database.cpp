#include"database.h"

/*************************** in class Trans ***************************/
void Trans::read(ifstream& fin)
{
	int x,n;

	del();

	fin>>ID>>n;
	while(n--) fin>>x,PID.push_back(x);
	fin>>n;
	while(n--) fin>>x,Data.push_back(x);
}

void Trans::write(ofstream& fout)
{
	fout<<ID<<'\n'<<PID.size()<<'\n';
	for(size_t i=0;i<PID.size();i++) fout<<PID[i]<<' ';
	fout<<'\n'<<Data.size()<<'\n';
	for(size_t i=0;i<Data.size();i++) fout<<Data[i]<<' ';
	fout<<'\n';
}

string Trans::hash()
{
	string s=to_string(ID);
	for(size_t i=0;i<PID.size();i++) s=s+'&'+to_string(PID[i]);
	for(size_t i=0;i<Data.size();i++) s=s+'&'+to_string(Data[i]);
	return SHA256(s);
}

bool Trans::check_File(ifstream& fin)
{
	string now=hash(),nxt;int t;
	while(1)
	{
		fin>>nxt>>t;
		if(t==2) return now==nxt;
		if(t) now=SHA256(now+nxt);
		else  now=SHA256(nxt+now);
	}
}

bool Trans::check_Local(Block* blo)
{
	int pos=blo->find(ID);
	if(!~pos) return false;
	if(!blo->T) {cerr<<"check_Local:  ****** ERROR: Unbuild when using check_Local ******\n";return false;}
	return blo->T->check_Local(pos)==hash();
}

void Trans::print_File(ofstream& fout,Block* blo)
{
	int pos=blo->find(ID);
	if(!~pos) {cerr<<"print_File:  ****** ERROR: Unfinded Trans when using print_File ******\n";return;}
	if(!blo->T) {cerr<<"print_File:  ****** ERROR: Unbuild when using print_File ******\n";return;}
	blo->T->print_File(fout,pos);
}

/*************************** in class Merkle ***************************/

void Merkle::des(node *p)
{
	if(!p) return;
	des(p->lc);des(p->rc);
	delete p;
}

node *Merkle::build(vit l,vit r)
{
	node *p=new node;
	if(l+1==r)
	{
		p->result=l->hash();
		p->lc=p->rc=NULL;
		return p;
	}
	p->lc=build(l,((r-l)>>1)+l);
	p->rc=build(((r-l)>>1)+l,r);
	p->result=SHA256(p->lc->result+p->rc->result);
	return p;
}

void Merkle::find_File(ofstream& fout,node *p,int l,int r,int d)
{
	if(l+1==r) return;

	int mid=(l+r)>>1;
	if(d<mid) find_File(fout,p->lc,l,mid,d),fout<<p->rc->result<<" 1 ";
	else      find_File(fout,p->rc,mid,r,d),fout<<p->lc->result<<" 0 ";
	if(p==rt) fout<<p->result<<" 2 "<<endl;
}

string Merkle::find_Local(node *p,int l,int r,int d)
{
	if(l+1==r) return p->result;

	if(SHA256(p->lc->result+p->rc->result)!=p->result) return "";

	int mid=(l+r)>>1;
	if(d<mid) return find_Local(p->lc,l,mid,d);
	else      return find_Local(p->rc,mid,r,d);
}

/*************************** in class Block ***************************/

void Block::del()
{
	if(T) delete T;
	V.clear();Mp.clear();
}

void Block::build()
{
	if(T) delete T;T=new Merkle;T->init(V);
	Mp.clear();for(size_t i=0;i<V.size();i++) Mp[V[i].ID]=i;
}

string Block::hash()
{
	if(!T) {cerr<<"Block::Hash:  ***** ERROR:  No Build when using hash *****\n";system("pause");return "";}
	return T->Get_Hash();
}

void Block::read(ifstream& fin)
{
	int N;
	fin>>ID>>rnd>>N;V.resize(N);
	for(int i=0;i<N;i++) V[i].read(fin);
}

void Block::write(ofstream& fout)
{
	fout<<ID<<' '<<rnd<<' '<<V.size()<<' '<<endl;
	for(size_t i=0;i<V.size();i++) V[i].write(fout);
	fout<<endl;
}


int Block::find(int ID)
{
	map<int,int>::iterator it=Mp.find(ID);
	if(it==Mp.end()) return -1;
	return it->second;
}

void Block::write_Merkle(ofstream& fout,int d)
{
	T->print_File(fout,d);
}

//to store the HANDLE of Checker Process
map<int,HANDLE> M_Pro;

void PoW_Start(int ID,int Local_ID)
{
	if(M_Pro.count(ID)) {cerr<<"Checker is already Running\n";return;}

	STARTUPINFO si;PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));ZeroMemory(&pi, sizeof(pi));
	char cmd[40];sprintf(cmd,"%s",("Checker.exe "+to_string(Local_ID)).data());
	CreateProcess(NULL,cmd,NULL,NULL,false,0,NULL,NULL,&si,&pi);
	M_Pro[ID]=pi.hProcess;
}

bool Block::PoW_Check()
{
	if(!T) {cerr<<"Unbuild When Checking";return false;}
	string ans=SHA256(hash()+to_string(rnd));
	//cerr<<ans<<endl;
	return ans<=Hash_Limit;
}

bool PoW_If_Finish(int ID)
{
	char file[50];
	sprintf(file,"block_%d.txt",ID);

	DWORD ipExitCode;
	if(!GetExitCodeProcess(M_Pro[ID],&ipExitCode)) system("echo \"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\" && pause");
	if(ipExitCode==STILL_ACTIVE) return false;
	system((string("del ")+Block_File).c_str());
	system((string("rename ")+file+' '+Block_File).c_str());
	return true;
}

void PoW_Stop(int ID)
{
	if(!M_Pro.count(ID)) {cerr<<"PoW_Stop:   Did not start Checker\n";return;}
	TerminateProcess(M_Pro[ID],0);
}