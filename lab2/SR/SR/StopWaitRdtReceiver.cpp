#include "stdafx.h"
#include "Global.h"
#include "StopWaitRdtReceiver.h"


StopWaitRdtReceiver::StopWaitRdtReceiver():NextSeqNum(0),base(0)
{
	NextSeqNum = base + N;
	lastAckPkt.acknum = -1; //��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//���Ը��ֶ�
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
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);


	//���У�����ȷ��ͬʱ�յ����ĵ���ŵ��ڽ��շ��ڴ��յ��ı������һ��
	if (checkSum == packet.checksum) {
		
		cout << "���շ�������������Ϊ " << '[' << ' ';
		for (int i = 0; i < 4; i++) {
				cout << base+i << ' ';
		}
		cout << ']' << endl;

		if (base == packet.seqnum) {//Ϊ������
			cout << "���շ��յ��ı������Ϊ" << packet.seqnum << endl;

			pUtils->printPacket("���շ���ȷ�յ����ͷ��ı���", packet);
			lastAckPkt.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
			packetWaitingFlags[packet.seqnum % Seqlenth] = true;
			ReceivedPacket[packet.seqnum % Seqlenth] = packet;
			ReceivedPacket[packet.seqnum % Seqlenth].acknum = 0;
			while (packetWaitingFlags[base % Seqlenth])//ȡ��Message�����ϵݽ���Ӧ�ò�
			{
				Message msg;
				memcpy(msg.data, ReceivedPacket[base % Seqlenth].payload, sizeof(ReceivedPacket[base % Seqlenth].payload));
				pns->delivertoAppLayer(RECEIVER, msg);
				packetWaitingFlags[base++ % Seqlenth] = false;//�ͷŻ�����
				packetWaitingFlags[NextSeqNum++ % Seqlenth] = false;//���뻺����
				ReceivedPacket[packet.seqnum % Seqlenth].acknum = -1;
			}
		}
		else if (base < packet.seqnum && packet.seqnum < NextSeqNum) {//���ǵ�һ���յ�ʱ���Ҳ��ǻ�����ʱ
			cout << "���շ��յ��ı������Ϊ" << packet.seqnum << endl;
			pUtils->printPacket("���շ��ѻ��淢�ͷ��ı���", packet);

			//�ŵ�������
			ReceivedPacket[packet.seqnum % Seqlenth] = packet;
			packetWaitingFlags[packet.seqnum % Seqlenth] = true;

			lastAckPkt.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�

			//this->expectSequenceNumberRcvd = 1 - this->expectSequenceNumberRcvd; //���������0-1֮���л�
			
		}
		else if(packet.seqnum >= base-N && packet.seqnum < base)
		{
			pUtils->printPacket("���շ���ȷ�յ���ȷ�ϵĹ�ʱ����", packet);
			lastAckPkt.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
		}
		else {
			pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,������Ų���", packet);
			cout << "��ʱ���շ��ڴ��������" << this->base << "~" << this->NextSeqNum << "֮��" << endl;
		}
	}
	else pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
}