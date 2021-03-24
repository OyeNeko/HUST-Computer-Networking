#include "Config.h"

Config::Config(void)
{
}

Config::~Config(void)
{
}

string Config::SERVERADDRESS = "\0";	//服务器IP地址			//最大连接数5
const int Config::BUFFERLENGTH = 1024;				//缓冲区大小256字节
int Config::PORT = 5050;						//服务器端口5050
const u_long Config::BLOCKMODE = 1;					//SOCKET为非阻塞模式
string  Config::path = "\0";
int Config::DealConfig() {

	ifstream File("Content/Config.ini", ios::binary);
	if (!File) {
		cerr << "open Config file error!" << endl;
		return 0;
	}
	File.seekg(0, ios::end);
	size_t size = File.tellg();
	char* buf = new char[(size + 1)];
	memset(buf, '\0', (size + 1));
	File.seekg(0, ios::beg);
	File.read(buf, size);
	File.close();
	string s(buf, buf + size);
	int p,q;
	p = s.find("path");
	q = s.find("ip");
	Config::path = s.substr(p + 5,q-p-7);
	int i;
	do
	{
		i = Config::path.find("/");
		if (i < 0)break;
		Config::path.erase(i, 1);
		Config::path.insert(i, "\\");
	} while (i >= 0);

	p = s.find("port");
	Config::SERVERADDRESS = s.substr(q + 3,p-q-5);
	q = s.length();
	s = s.substr(p + 5, q - p-5);
	
	Config::PORT = std::stoi(s);
 	return 1;
};
