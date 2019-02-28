#include"database.h"

Block* blo;

bool Try(long long rnd)
{
	blo->rnd=rnd;
	return blo->PoW_Check();
}

long long rnd(){return ((((((long long)rand()<<15)+rand())<<15)+rand())<<15)+rand();}

int main(int argc,char *argv[])//0 name 1 Local_ID
{
	ifstream fin(Checker_File);//cerr<<argv[1]<<'\n';

	int From_ID;string s;
	fin>>From_ID>>s;

	blo=new Block;
	blo->read(fin);
	fin.close();
	blo->build();

	cerr<<"Checker:   ***** Start checking block: "<<blo->ID<<" *****\n";
	
	srand(time(0)+atoi(argv[1])*233);
	
	int TryTime=0;
	while(!Try(rnd()))
	{
		TryTime++;
		//if(!(TryTime%100)) cerr<<TryTime<<'\n';
	}

	char file[50];
	sprintf(file,"block_%d.txt",blo->ID);
	ofstream fout(file);
	fout<<argv[1]<<" PoWBlock\n";
	blo->write(fout);
	fout.close();

cerr<<"Checker:   ***** Check Success  TryTime = "<<TryTime<<" *****\n";

	return 0;
}