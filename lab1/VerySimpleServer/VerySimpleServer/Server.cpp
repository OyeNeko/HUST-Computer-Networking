#pragma once
#include "Config.h"

int ReadMyFile(string filename, Request& q)
{
	ifstream File(filename, ios::binary);
	if (!File) {
		cerr << "open error!" << endl;
		return 0;
	}
	File.seekg(0, ios::end);
	size_t size = File.tellg();
	char* buf = new char[(size+1)];
	memset(buf, '\0', (size+1));
	File.seekg(0, ios::beg);
	File.read(buf, size);  
	int j = strlen(buf);
	File.close();
	string s(buf,buf+size);
	q.body = s;
	return 1;
}

void main(){
	
	if (!Config::DealConfig()) {
		cout << "Please input yuor file path:";
		getline(cin, Config::path);
		cout << "Please input your ip adress:";
		getline(cin, Config::SERVERADDRESS);
		cout << "Please input your port:";
		cin >> Config::PORT;
	};

	WSADATA wsaData;
	fd_set wfds;				//Long类型的数组，每一个数组元素可与一打开的文件句柄建立联系
	fd_set rfds;				//用于检查socket是否有数据到来的的文件描述符，用于socket非阻塞模式下等待网络事件通知（有数据到来）
	bool first_connetion = true;

	int nRc = WSAStartup(0x0202,&wsaData);

	if(nRc){
		printf("Winsock  startup failed with error!\n");
	}

	if(wsaData.wVersion != 0x0202){
		printf("Winsock version is not correct!\n");
		WSACleanup();
		return;
	}

	printf("Winsock  startup Ok!\n");


	SOCKET srvSocket;
	sockaddr_in addr,clientAddr;
	SOCKET sessionSocket;
	string clientAddress;

	int addrLen;
	//create socket
	srvSocket = socket(AF_INET,SOCK_STREAM,0);//创建套接字
	if(srvSocket != INVALID_SOCKET)
		printf("Socket create Ok!\n");
	//set port and ip
	addr.sin_family = AF_INET;
	addr.sin_port = htons(Config::PORT);//端口号为5050
	//addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addr.sin_addr.S_un.S_addr = inet_addr(Config::SERVERADDRESS.c_str());//配置ip
	//binding
	int rtn = bind(srvSocket,(LPSOCKADDR)&addr,sizeof(addr));
	if (rtn != SOCKET_ERROR)
		printf("Socket bind Ok!\n");
	else {
		cout << "Socket bind Error!" << WSAGetLastError() << endl;
		closesocket(srvSocket);
		WSACleanup();
	}//listen
	rtn = listen(srvSocket,5);//创建一个套接口并监听申请的连接.
	if (rtn != SOCKET_ERROR)
		printf("Socket listen Ok!\n");
	else cout << "Socket listen Ok!" << WSAGetLastError() << endl;
	addrLen = sizeof(clientAddr);

	u_long blockMode = 1;//将srvSock设为非阻塞模式以监听客户连接请求

	if ((rtn = ioctlsocket(srvSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO：允许或禁止套接口s的非阻塞模式。
		cout << "ioctlsocket() failed with error!\n";
		return;
	}
	cout << "ioctlsocket() for server socket ok!	Waiting for client connection and data\n";

	char* recvBuf = new char[Config::BUFFERLENGTH]();
	memset(recvBuf, '\0', Config::BUFFERLENGTH);
	while (true) {
		//清空read,write描述符
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		//设置等待客户连接请求
		FD_SET(srvSocket, &rfds);

		if (!first_connetion) {
			//设置等待会话SOKCET可接受数据或可发送数据
			FD_SET(sessionSocket, &rfds);
			FD_SET(sessionSocket, &wfds);
		}
		//开始等待
		int nTotal = select(0, &rfds, &wfds, NULL, NULL);

			//如果srvSock收到连接请求，接受客户连接请求
		if (FD_ISSET(srvSocket, &rfds)) {//测试srvSocket是否可读，即是否网络上有数据
			nTotal--;

			//产生会话SOCKET
			sessionSocket = accept(srvSocket, (LPSOCKADDR)&clientAddr, &addrLen);
			if (sessionSocket != INVALID_SOCKET)
				printf("Socket listen one client request!\n");

			//把会话SOCKET设为非阻塞模式
			if ((rtn = ioctlsocket(sessionSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO：允许或禁止套接口s的非阻塞模式。
				cout << "ioctlsocket() failed with error!\n";
				return ;
			}
			cout << "ioctlsocket() for session socket ok!	Waiting for client connection and data\n";
			

			//设置等待会话SOKCET可接受数据或可发送数据
			FD_SET(sessionSocket, &rfds);
			FD_SET(sessionSocket, &wfds);
			first_connetion = false;

		}

			//检查会话SOCKET是否有数据到来
		if (nTotal > 0)
		{

			//如果会话SOCKET有数据到来，则接受客户的数据

			if (FD_ISSET(sessionSocket, &rfds)) {
				//receiving data from client
				memset(recvBuf, '\0', Config::BUFFERLENGTH);
				rtn = recv(sessionSocket, recvBuf, Config::BUFFERLENGTH, 0);
				cout << recvBuf << endl;
				if (rtn > 0)
				{
					stringstream ss(recvBuf);
					Request rqst = rqst.Dealrqst(ss);
					if (ReadMyFile(Config::path + rqst.url, rqst))
					{

						string rsbd = rqst.ResponseString(rqst, 200);
						int wtn = send(sessionSocket, rsbd.c_str(), rsbd.length(), 0);
						if (wtn == SOCKET_ERROR) {
							cout << "Send Failed!!" << endl;
							continue;
						}
						wtn = send(sessionSocket, rqst.body.c_str(), rqst.body.length(), 0);
						if (wtn == SOCKET_ERROR) {
							cout << "Send Failed!!" << endl;
						}
						cout << "Send success!" << endl;
					}
					else
					{
						ReadMyFile(Config::path + "NotFound.html", rqst);

						string rsbd = rqst.ResponseString(rqst, 404);
						int wtn = send(sessionSocket, rsbd.c_str(), rsbd.length(), 0);
						if (wtn == SOCKET_ERROR) {
							cout << "Send Failed!!" << endl;
						}
						wtn = send(sessionSocket, rqst.body.c_str(), rqst.body.length(), 0);
						if (wtn == SOCKET_ERROR) {
							cout << "Send Failed!!" << endl;
						}
						cout << "Send success!" << endl;
					}
				}
				else {
					//printf("Client leaving ...\n");
					closesocket(sessionSocket);  //既然client离开了，就关闭sessionSocket

				}
			}
		}
		
	}
}

