#pragma once
#include <string>
#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <ctime>
#include "Http.h"
#include <sstream>
#include <fstream>
#include <filesystem>
#include <fstream>
#include <map>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <ctime>
#include <iterator>
#include <string.h>
using namespace std;
#pragma comment(lib,"ws2_32.lib")



//保存配置信息
class Config
{
public:
	static const int BUFFERLENGTH;		//缓冲区大小
	static  string SERVERADDRESS;  //服务器地址
	static  int PORT;				//服务器端口
	static const u_long BLOCKMODE;			//SOCKET阻塞模式
	static string  path;
	static int DealConfig();

private:
	Config(void);
	~Config(void);
};
