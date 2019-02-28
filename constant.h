/********* Project New **********/

#ifndef constant_h
#define constant_h

#include<bits/stdc++.h>
#include<windows.h>
using namespace std;

const int INF = 0x3f3f3f3f;

const string IP("192.168.43.21"); 

const int Max_Clock_Num = 5;

const int Mashine_Num = 2 + 1;//attention!! included Server and God
const int Store_Num = 1;//store block
const int Check_Num = 1;//check trans in User  notice: 1.Check_Num must be smaller than Store_Num  2.User will receive Check_Num+1 messagess  
const int Trans_DataSize = 60;
const int Trans_PIDSize = 4;
const int BlockSizeLimit = 10;
const int DataScale= 100;

const string Hash_Limit("0000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

const int Announce_ID = 10000;
const int God_ID      = 0;
const int Server_ID   = 1;
const int Delay_Time  = 5;
const int NewTrans_Delay_Time = 5;

const char God_File[20]="God_tmp.txt";
const char Server_File[20]="Server_tmp.txt";
const char Checker_File[20]="Checker_tmp.txt";
const char Block_File[20]="Block_tmp.txt";


#endif //constant_h
