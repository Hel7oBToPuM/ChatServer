#pragma once
#include <mutex>
#include <set>
#include "simpleFunctions.h"



//������������� ��� ��� ���� �������� �������
enum user_events
{
	SEND_MESSAGE_TO_USER, //��������� ��������� ������� ������������ +
	SEND_MESSAGE_TO_CHANNEL, //��������� ��������� � ����� +
	CREATE_CHANNEL, //������� ����� +
	CONNECT_TO_CHANNEL, //������������ � ������ +
	DISCONNECT_FROM_CHANNEL, //����������� �� ������ +
	FIND_USER, //����� ������������ +
	DELETE_CHANNEL, //������� ����� +
	EVENT_ERROR, // ������ +
	FIND_CHANNEL, // ����� ����� +
	ADD_TO_CHANNEL_USER, //�������� � ����� ������������ +
	EVENT_COMPLETE, // �������� ������� ������� ���������� +
	CHECK_ONLINE, //��������� ����������� ������� ������� +
	EVENT_ERROR_WITHOUT_DISCONNECT, // ������ ��� ���������� ������������ +
	CHECK_MESSAGES_FROM_USERS, //�������� �������� ��������� �� ������������� +
	CHECK_MESSAGES_FROM_CHANNELS, //�������� �������� ��������� �� ������������� +
	CHECK_LIST_OF_CHANNELS,//�������� ������������ ������� ��� ������������ +
	FIRST_DATA_REQUEST //������ ������ ��� �������������� +
};

//�������� ���� ���������, �������������� �������

extern bool ok; //��
extern bool error; //������

extern std::map<std::string, std::set<std::string>> users_channels; //������, � ������� ��������� ������������
extern std::map<std::string, std::vector<std::string>> users_list_of_msg; //������ ��� ��������� ��������� ����������� ��� ������� �������
extern std::map<std::string, std::string> userdata; //������ �����-������ ��� ������� ������
extern std::map<std::string, channel> channels; //������ ��� ��������-�����;
extern std::map<std::string, SOCKET> usersocket; //�������� ���� ��� ������������ - �����, ���� ������ ���������
extern std::set<std::string> useronline; //�������� ������������� � ������� online
extern std::map<std::string, std::set<std::string>> users_cr_channels; //������, ������� ������ ������������
extern std::map<std::string, std::map<std::string, std::vector<std::string>>> msgpool_of_channels; //������ ������, ������� ������������ ������������, ������� ������������� ������������ ��� ��������� �� ������

extern std::mutex mute_userdata; //���������� ��� ������� userdata (������ ������)
extern std::mutex mute_usersocket_useronline; //���������� ��� ������� usersocket (������ ������)
extern std::mutex servermsg; //���������� ��� ��������� ���������
extern std::mutex mute_userslistofmsg; //���������� ��� ������� users_list_of_msg (������ ������)
extern std::mutex mute_channels; //���������� ��� ������� channels (������ ������)

//����� ��� �������������� � ��������
class UserData
{
public:
	simpleFunctions simpFunc;
	SOCKET Client; //����� ������������
	std::string user; //������������, � ������� �������� ������ ������
	//�����������
	UserData(SOCKET socketClient);
	//��������� ������� � ������
	bool setOnline();
	//��������� ������� � �������
	void setOffline();
	//�������� ������� ������
	bool isPasswordCorrect(std::string& login, std::string& password);
	// ����� ������������
	user_events findUser();
	//������� �������� �������� �������
	user_events waitForAction();
	//��������� ������� (������/�������) ������������
	user_events getUserStatus();
	//��������� ��������� ������������
	user_events sendMessageToUser();
	//��������� � �������� �������� ���������
	user_events checkMessages();
	//������� �����
	user_events createChannel();
	//������� �����
	user_events deleteChannel();
	//����� �����
	user_events findChannel();
	//������������ � ������
	user_events connectToChannel();
	//����������� �� ������
	user_events disconnectFromChannel();
	//�������� � ����� ������������
	user_events addToChannel();
	//��������� ��������� � �����
	user_events sendMessageToChannel();
	//��������� ��������� � �����
	user_events checkMessagesFromChannels();
	//�������� ������������ ������ ����� ��� ������������
	user_events checkListOfChannels();
	//������ ������ ����� �����������
	user_events firstDataRequest();
	//����������� ������� � ����
	bool registerClient(std::string& log_pass);
	//�������� ������ ������� � ���� ��� �����
	bool loginClient(std::string& log_pass);
	//������� ����� ����������� �������
	bool authClient();
};