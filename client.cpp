#include "transmit.h"
#include <wininet.h>
#include <tchar.h>
#include <Urlmon.h>
#include <locale>
#include <streambuf>

using namespace std;

#define ERROR_OPEN_FILE       10
#define ERROR_MEMORY          11
#define ERROR_SIZE            12
#define ERROR_INTERNET_OPEN   13
#define ERROR_INTERNET_CONN   14
#define ERROR_INTERNET_REQ    15
#define ERROR_INTERNET_SEND   16

//change IP
const string url("/api.php"),
			 boundary("--blockchaintest19260817");

string basic_header, header, contentend, field[3];
struct hostent *host;

inline string CreateField(string name)
{
	return string("") + "--" + boundary + "\r\n" + "Content-Disposition: form-data; name=\"" + name + "\"\r\n\r\n";
}

void HttpRequestInit()
{
	//init
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "WSAStartup failed.\n";
		exit(-1);
	}
	//print header
	basic_header = "";
	basic_header = basic_header + "Host: " + IP + "\r\n" +\
								  "Connection: close\r\n" +\
								  "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n" +\
								  "Accept: */*\r\n" +\
								  "Accept-encoding: identity\r\n" +\
								  "Cache-Control: no-cache\r\n";
	field[0] = CreateField("target");
	field[1] = CreateField("client");
	field[2] = CreateField("data");
	contentend = "--" + boundary + "--";
	struct in_addr addr = { 0, 0, 0, 0 };
	addr.s_addr = inet_addr(IP.c_str());
	host =  gethostbyaddr((char *) &addr, 4, AF_INET);
	return ;
}

void HttpRequestClean()
{
	WSACleanup();
	return ;
}

//it has some problem when sending very long request  (change it to curl -F "target=" -F "client=" -F "data=<file")
string MakeRequest(string request)
{
	static char buffer[10009];
	SOCKET Socket;
	SOCKADDR_IN SockAddr;
	locale local;
	string data;

	//HTTP POST

	Socket=socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);

	SockAddr.sin_port=htons(80);
	SockAddr.sin_family=AF_INET;
	SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

	if(connect(Socket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0)
	{
		cerr << "Could not connect";
		return "";
	}

	send(Socket, request.c_str(), strlen(request.c_str()),0 );
	
	memset(buffer,0,sizeof(buffer));
	while ((recv(Socket, buffer, 10000,0)) > 0)
		for (int i=0;buffer[i]>=32 || buffer[i]=='\n' || buffer[i]=='\r';i++)
			data+=buffer[i];
//	cerr << data << '\n';
	data.erase(0, min((int)data.size(),(int)data.find("\r\n\r\n")+7));
	data.erase(max(0,(int)data.size()-7), min(7,(int)data.size()));
	closesocket(Socket);
	return data;
}

int MakePost(int client,int target,string filename)
{
	//read file
	ifstream fin(filename.c_str());
	string data((std::istreambuf_iterator<char>(fin)),
					 std::istreambuf_iterator<char>());
	string content("");
	content = content + field[0] + to_string(target) + "\r\n";
	content = content + field[1] + to_string(client) + "\r\n";
	content = content + field[2] + data + "\r\n";
	content = content + contentend;

	//build header

	header = "POST "+ url + " HTTP/1.1\r\n" + basic_header + "Content-Length: " + to_string(strlen(content.c_str())) + "\r\n\r\n";

	string request = header + content;
	//cerr << request << '\n';
	MakeRequest(request);

	return 0;

}

int GetCount(int client)
{
	//build header
	header = "GET " + url + "?client=" + to_string(client) + "&type=0 HTTP/1.1\r\n" + basic_header + "\r\n";
	return atoi(MakeRequest(header).c_str());
}

void PrintRecord(int client,int pos,string filename)
{
	/*
	string request="http://"+IP+url+"?client="+to_string(client)+"&type=1&pos="+to_string(pos);
	HRESULT res=URLDownloadToFile(NULL,request.c_str(),filename.c_str(),0,NULL);
	if (res == S_OK)
		cerr << "Download Done\n";
	if (res == E_OUTOFMEMORY)
		cerr << "Buffer length invalid, or insufficient memory\n";
	if (res == INET_E_DOWNLOAD_FAILURE)
		cerr << "URL is invalid\n";
	return ;*/
	#ifdef UsingSystemCurl 
	string cmd="curl -s \""+IP+url+"?client="+to_string(client)+"&type=1&pos="+to_string(pos)+"\" >"+filename;
	#else
	string cmd=".\\curl -s \""+IP+url+"?client="+to_string(client)+"&type=1&pos="+to_string(pos)+"\" >"+filename;
	#endif
	system(cmd.c_str());
}

/*
int main()
{
	HttpRequestInit();
	MakePost(1, 2, "yuw.txt");
//	cerr << GetCount(2);
//	cerr << GetRecord(2, 3) << GetRecord(2, 4) << '\n';
	// PrintRecord(2, 3, "1.txt");
	HttpRequestClean();
	return 0;
}
*/
