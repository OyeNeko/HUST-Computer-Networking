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
	fd_set wfds;				//Long���͵����飬ÿһ������Ԫ�ؿ���һ�򿪵��ļ����������ϵ
	fd_set rfds;				//���ڼ��socket�Ƿ������ݵ����ĵ��ļ�������������socket������ģʽ�µȴ������¼�֪ͨ�������ݵ�����
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
	srvSocket = socket(AF_INET,SOCK_STREAM,0);//�����׽���
	if(srvSocket != INVALID_SOCKET)
		printf("Socket create Ok!\n");
	//set port and ip
	addr.sin_family = AF_INET;
	addr.sin_port = htons(Config::PORT);//�˿ں�Ϊ5050
	//addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addr.sin_addr.S_un.S_addr = inet_addr(Config::SERVERADDRESS.c_str());//����ip
	//binding
	int rtn = bind(srvSocket,(LPSOCKADDR)&addr,sizeof(addr));
	if (rtn != SOCKET_ERROR)
		printf("Socket bind Ok!\n");
	else {
		cout << "Socket bind Error!" << WSAGetLastError() << endl;
		closesocket(srvSocket);
		WSACleanup();
	}//listen
	rtn = listen(srvSocket,5);//����һ���׽ӿڲ��������������.
	if (rtn != SOCKET_ERROR)
		printf("Socket listen Ok!\n");
	else cout << "Socket listen Ok!" << WSAGetLastError() << endl;
	addrLen = sizeof(clientAddr);

	u_long blockMode = 1;//��srvSock��Ϊ������ģʽ�Լ����ͻ���������

	if ((rtn = ioctlsocket(srvSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO��������ֹ�׽ӿ�s�ķ�����ģʽ��
		cout << "ioctlsocket() failed with error!\n";
		return;
	}
	cout << "ioctlsocket() for server socket ok!	Waiting for client connection and data\n";

	char* recvBuf = new char[Config::BUFFERLENGTH]();
	memset(recvBuf, '\0', Config::BUFFERLENGTH);
	while (true) {
		//���read,write������
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		//���õȴ��ͻ���������
		FD_SET(srvSocket, &rfds);

		if (!first_connetion) {
			//���õȴ��ỰSOKCET�ɽ������ݻ�ɷ�������
			FD_SET(sessionSocket, &rfds);
			FD_SET(sessionSocket, &wfds);
		}
		//��ʼ�ȴ�
		int nTotal = select(0, &rfds, &wfds, NULL, NULL);

			//���srvSock�յ��������󣬽��ܿͻ���������
		if (FD_ISSET(srvSocket, &rfds)) {//����srvSocket�Ƿ�ɶ������Ƿ�������������
			nTotal--;

			//�����ỰSOCKET
			sessionSocket = accept(srvSocket, (LPSOCKADDR)&clientAddr, &addrLen);
			if (sessionSocket != INVALID_SOCKET)
				printf("Socket listen one client request!\n");

			//�ѻỰSOCKET��Ϊ������ģʽ
			if ((rtn = ioctlsocket(sessionSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO��������ֹ�׽ӿ�s�ķ�����ģʽ��
				cout << "ioctlsocket() failed with error!\n";
				return ;
			}
			cout << "ioctlsocket() for session socket ok!	Waiting for client connection and data\n";
			

			//���õȴ��ỰSOKCET�ɽ������ݻ�ɷ�������
			FD_SET(sessionSocket, &rfds);
			FD_SET(sessionSocket, &wfds);
			first_connetion = false;

		}

			//���ỰSOCKET�Ƿ������ݵ���
		if (nTotal > 0)
		{

			//����ỰSOCKET�����ݵ���������ܿͻ�������

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
					closesocket(sessionSocket);  //��Ȼclient�뿪�ˣ��͹ر�sessionSocket

				}
			}
		}
		
	}
}

