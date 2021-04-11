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
	if (this->waitingState) { //���ͷ����ڵȴ�ȷ��״̬
		return false;
	}

	if (expectSequenceNumberSend < base + N) {

		this->packetWaitingAck[expectSequenceNumberSend % Seqlenth].acknum = -1; //���Ը��ֶ�
		this->packetWaitingAck[expectSequenceNumberSend % Seqlenth].seqnum = this->expectSequenceNumberSend;
		this->packetWaitingAck[expectSequenceNumberSend % Seqlenth].checksum = 0;
		memcpy(this->packetWaitingAck[expectSequenceNumberSend % Seqlenth].payload, message.data, sizeof(message.data));
		this->packetWaitingAck[expectSequenceNumberSend % Seqlenth].checksum = pUtils->calculateCheckSum(this->packetWaitingAck[expectSequenceNumberSend % Seqlenth]);
		ACKFlags[expectSequenceNumberSend % Seqlenth] = false;

		pUtils->printPacket("���ͷ����ͱ���", this->packetWaitingAck[expectSequenceNumberSend % Seqlenth]);
		
		cout << "����" << expectSequenceNumberSend << "���ͷ�������ʱ��" << endl;
		pns->startTimer(SENDER, Configuration::TIME_OUT, expectSequenceNumberSend);			//�������ͻ����з���ʱ��
		
		pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[expectSequenceNumberSend % Seqlenth]);								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
		expectSequenceNumberSend++;
		
		cout << "�˱��ķ��ͺ�expectSequenceNumberSendΪ" << expectSequenceNumberSend << endl;
		if (expectSequenceNumberSend == base + N) {
			this->waitingState = true;//����ȴ�״̬
		}
	}
	return true;
}

void StopWaitRdtSender::receive(const Packet& ackPkt) {

	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����
	if (checkSum == ackPkt.checksum) {
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
		for (int i = base + N; i < base + 8; i++) {

			packetWaitingAck[i % Seqlenth].seqnum = -1;
		}
		cout << "���ͷ�������������Ϊ " << '[' << ' ';
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
			cout << "��ȷ��ACK���Ϊ" << ackPkt.acknum << "��ACK" << endl;
			pns->stopTimer(SENDER, ackPkt.acknum);
			ACKFlags[base % Seqlenth] = true;
			while (ACKFlags[base % Seqlenth]) {
				ACKFlags[base++ % Seqlenth] = false;
			}
			waitingState = false;
		}
		else if (ackPkt.acknum > base && !ACKFlags[ackPkt.acknum % Seqlenth]) {
			cout << "��ȷ��ACK���Ϊ" << ackPkt.acknum << "��ACK" << endl;
			pns->stopTimer(SENDER, ackPkt.acknum);
			ACKFlags[ackPkt.acknum % Seqlenth] = true;
		}
		else
		{
			cout << "�յ�����Ҫ�����е�ACK,�����ȴ�" << endl;
		}

	}
	else cout << "���ͷ��յ���ACK��" << endl;
	
}

void StopWaitRdtSender::timeoutHandler(int seqNum) {
	//Ψһһ����ʱ��,���迼��seqNum
	cout << "���ͳ�ʱ" << endl;
	pns->stopTimer(SENDER, seqNum);
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);
	cout << "�ط�" << seqNum << "�ű���" << endl;
	pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط�����", this->packetWaitingAck[seqNum % Seqlenth]);
	pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck[seqNum % Seqlenth]);			//���·������ݰ�

}
