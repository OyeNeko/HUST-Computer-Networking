#include "stdafx.h"
#include "Global.h"
#include "StopWaitRdtSender.h"


StopWaitRdtSender::StopWaitRdtSender():base(0),expectSequenceNumberSend(0),waitingState(false)
{
	for (int i = 0; i < Seqlenth; i++) {
		ACKFlags[i] = false;
	}
	
}


StopWaitRdtSender::~StopWaitRdtSender()
{
}



bool StopWaitRdtSender::getWaitingState() {
	return waitingState;
}




bool StopWaitRdtSender::send(const Message &message) {
	if (this->waitingState) { //发送方处于等待确认状态
		return false;
	}

	if (expectSequenceNumberSend < base + N) {

		this->packetWaitingAck[expectSequenceNumberSend % Seqlenth].acknum = -1; //忽略该字段
		this->packetWaitingAck[expectSequenceNumberSend % Seqlenth].seqnum = this->expectSequenceNumberSend;
		this->packetWaitingAck[expectSequenceNumberSend % Seqlenth].checksum = 0;
		memcpy(this->packetWaitingAck[expectSequenceNumberSend % Seqlenth].payload, message.data, sizeof(message.data));
		this->packetWaitingAck[expectSequenceNumberSend % Seqlenth].checksum = pUtils->calculateCheckSum(this->packetWaitingAck[expectSequenceNumberSend % Seqlenth]);
		ACKFlags[expectSequenceNumberSend % Seqlenth] = false;

		pUtils->printPacket("发送方发送报文", this->packetWaitingAck[expectSequenceNumberSend % Seqlenth]);
		
		cout << "序列" << expectSequenceNumberSend << "发送方启动计时器" << endl;
		pns->startTimer(SENDER, Configuration::TIME_OUT, expectSequenceNumberSend);			//启动发送基序列方定时器
		
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[expectSequenceNumberSend % Seqlenth]);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
		expectSequenceNumberSend++;
		
		cout << "此报文发送后，expectSequenceNumberSend为" << expectSequenceNumberSend << endl;
		if (expectSequenceNumberSend == base + N) {
			this->waitingState = true;//进入等待状态
		}
	}
	return true;
}

void StopWaitRdtSender::receive(const Packet& ackPkt) {

	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
	if (checkSum == ackPkt.checksum) {
		pUtils->printPacket("发送方正确收到确认", ackPkt);
		for (int i = base + N; i < base + 8; i++) {

			packetWaitingAck[i % Seqlenth].seqnum = -1;
		}
		cout << "发送方滑动窗口内容为 " << '[' << ' ';
		for (int i = base; i < base + N; i++) {
			if (packetWaitingAck[i % Seqlenth].seqnum == -1) {
				cout << '*' << ' ';
			}
			else {
				cout << packetWaitingAck[i % Seqlenth].seqnum << ' ';
			}
		}
		cout << ']' << endl;
		if (ackPkt.acknum == base) {
			cout << "已确认ACK序号为" << ackPkt.acknum << "的ACK" << endl;
			pns->stopTimer(SENDER, ackPkt.acknum);
			ACKFlags[base % Seqlenth] = true;
			while (ACKFlags[base % Seqlenth]) {
				ACKFlags[base++ % Seqlenth] = false;
			}
			waitingState = false;
		}
		else if (ackPkt.acknum > base && !ACKFlags[ackPkt.acknum % Seqlenth]) {
			cout << "已确认ACK序号为" << ackPkt.acknum << "的ACK" << endl;
			pns->stopTimer(SENDER, ackPkt.acknum);
			ACKFlags[ackPkt.acknum % Seqlenth] = true;
		}
		else
		{
			cout << "收到不需要的序列的ACK,继续等待" << endl;
		}

	}
	else cout << "发送方收到的ACK损坏" << endl;
	
}

void StopWaitRdtSender::timeoutHandler(int seqNum) {
	//唯一一个定时器,无需考虑seqNum
	cout << "发送超时" << endl;
	pns->stopTimer(SENDER, seqNum);
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);
	cout << "重发" << seqNum << "号报文" << endl;
	pUtils->printPacket("发送方定时器时间到，重发报文", this->packetWaitingAck[seqNum % Seqlenth]);
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[seqNum % Seqlenth]);			//重新发送数据包

}
