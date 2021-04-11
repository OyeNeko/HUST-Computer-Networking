#include "stdafx.h"
#include "Global.h"
#include "StopWaitRdtReceiver.h"


StopWaitRdtReceiver::StopWaitRdtReceiver():NextSeqNum(0),base(0)
{
	NextSeqNum = base + N;
	lastAckPkt.acknum = -1; //初始状态下，上次发送的确认包的确认序号为-1，使得当第一个接受的数据包出错时该确认报文的确认号为-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//忽略该字段
	for(int i = 0; i < Configuration::PAYLOAD_SIZE;i++){
		lastAckPkt.payload[i] = '.';
	}
	for (int i = 0; i < Seqlenth; i++) {
		packetWaitingFlags[i] = false;
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}


StopWaitRdtReceiver::~StopWaitRdtReceiver()
{
}

void StopWaitRdtReceiver::receive(const Packet &packet) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);


	//如果校验和正确，同时收到报文的序号等于接收方期待收到的报文序号一致
	if (checkSum == packet.checksum) {
		
		cout << "接收方滑动窗口内容为 " << '[' << ' ';
		for (int i = 0; i < 4; i++) {
				cout << base+i << ' ';
		}
		cout << ']' << endl;

		if (base == packet.seqnum) {//为基序列
			cout << "接收方收到的报文序号为" << packet.seqnum << endl;

			pUtils->printPacket("接收方正确收到发送方的报文", packet);
			lastAckPkt.acknum = packet.seqnum; //确认序号等于收到的报文序号
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("接收方发送确认报文", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
			packetWaitingFlags[packet.seqnum % Seqlenth] = true;
			ReceivedPacket[packet.seqnum % Seqlenth] = packet;
			ReceivedPacket[packet.seqnum % Seqlenth].acknum = 0;
			while (packetWaitingFlags[base % Seqlenth])//取出Message，向上递交给应用层
			{
				Message msg;
				memcpy(msg.data, ReceivedPacket[base % Seqlenth].payload, sizeof(ReceivedPacket[base % Seqlenth].payload));
				pns->delivertoAppLayer(RECEIVER, msg);
				packetWaitingFlags[base++ % Seqlenth] = false;//释放缓存区
				packetWaitingFlags[NextSeqNum++ % Seqlenth] = false;//放入缓冲区
				ReceivedPacket[packet.seqnum % Seqlenth].acknum = -1;
			}
		}
		else if (base < packet.seqnum && packet.seqnum < NextSeqNum) {//当是第一次收到时，且不是基序列时
			cout << "接收方收到的报文序号为" << packet.seqnum << endl;
			pUtils->printPacket("接收方已缓存发送方的报文", packet);

			//放到缓存区
			ReceivedPacket[packet.seqnum % Seqlenth] = packet;
			packetWaitingFlags[packet.seqnum % Seqlenth] = true;

			lastAckPkt.acknum = packet.seqnum; //确认序号等于收到的报文序号
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("接收方发送确认报文", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方

			//this->expectSequenceNumberRcvd = 1 - this->expectSequenceNumberRcvd; //接收序号在0-1之间切换
			
		}
		else if(packet.seqnum >= base-N && packet.seqnum < base)
		{
			pUtils->printPacket("接收方正确收到已确认的过时报文", packet);
			lastAckPkt.acknum = packet.seqnum; //确认序号等于收到的报文序号
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("接收方发送确认报文", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
		}
		else {
			pUtils->printPacket("接收方没有正确收到发送方的报文,报文序号不对", packet);
			cout << "此时接收方期待的序号是" << this->base << "~" << this->NextSeqNum << "之间" << endl;
		}
	}
	else pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
}