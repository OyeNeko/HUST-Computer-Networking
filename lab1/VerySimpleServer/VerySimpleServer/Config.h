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



//����������Ϣ
class Config
{
public:
	static const int BUFFERLENGTH;		//��������С
	static  string SERVERADDRESS;  //��������ַ
	static  int PORT;				//�������˿�
	static const u_long BLOCKMODE;			//SOCKET����ģʽ
	static string  path;
	static int DealConfig();

private:
	Config(void);
	~Config(void);
};
