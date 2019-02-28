#ifndef database_h
#define database_h

#include"hash.h"

class Trans;
class Merkle;
class Block;

class Trans{
public:
	int ID;vector<int> PID;
	vector<int> Data;
	int CheckStatus;

	void read(ifstream& fin);
	void write(ofstream& fout);
	
	Trans(){CheckStatus=0;}
	inline void del(){PID.clear();Data.clear();}

	string hash();

	bool check_File(ifstream& fin);
	bool check_Local(Block* blo);
	void print_File(ofstream& fout,Block* blo);

	inline bool operator ==(const Trans&sec) const {return ID==sec.ID&&PID==sec.PID&&Data==sec.Data;}
};

typedef vector<Trans>::iterator vit;

class node{
public:
	node *lc,*rc;
	string result;
};

class Merkle{
private:
	node *rt;int size;

	void des(node *p);
	node *build(vit l,vit r);

	void find_File(ofstream& fout,node *p,int l,int r,int d);
	string find_Local(node *p,int l,int r,int d);
public:
	inline void del(){if(rt) des(rt);size=0;}
	~Merkle(){del();}

	inline string Get_Hash(){return rt->result;}

	inline void init(vector<Trans>& V){size=V.size();rt=build(V.begin(),V.end());}

	inline void print_File(ofstream& fout,int d){find_File(fout,rt,0,size,d);}
	inline string check_Local(int d){return find_Local(rt,0,size,d);}
};


class Block{
public:
	Merkle *T;
	map<int,int> Mp;
	int ID;long long rnd;
	vector<Trans> V;
	int CheckStatus;

	Block(){T=NULL;CheckStatus=0;Mp.clear();V.clear();}
	void del();
	~Block(){del();}

	void build();
	string hash();

	void read(ifstream& fin);
	void write(ofstream& fout);

	int find(int ID);

	void write_Merkle(ofstream& fout,int d);

	//attention! you must call this after build
	bool PoW_Check();
};

void PoW_Start(int ID,int Local_ID);
bool PoW_If_Finish(int ID);
void PoW_Stop(int ID);

#endif