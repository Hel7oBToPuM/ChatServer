#pragma once
#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <mutex>
#include <map>
#include <iostream>
#include <string>
#include <fstream>
#include <set>
#include <vector>

//�����
struct channel
{
	std::string creater; //��������� ������
	std::set<std::string> members; //��������� ������
};

class simpleFunctions
{
private:
	//�������� ������� �����
	bool exist(const std::string& path);
public:
	//������ ������ �� ����� � ������
	void getLoginPassword(std::string& log_pass, std::string& login, std::string& password);
	//������� ��������� ���� ������ �� TCP/IP
	bool recvall(SOCKET& socket, char* msg, int length, int flag);
	//������� �������������� ip �� char � �������� ������
	in_addr getIpv4FromChar(const char* ip);
	//�������������� ����������� message-������ � message-����� ��� ��������
	void makeMessageForSend(std::string& message_form, std::string& reciever, const std::string& sender);
	//�������� ������� ��� ������� ������� �� �����
	bool load_userschannels(const std::string& path, std::map<std::string, std::set<std::string>> &container);
	//���������� ������� ��� ������� ������� � ����
	bool save_userschannels(const std::string& path, std::map<std::string, std::set<std::string>>& container);
	//�������� ������ ��� ������� ������ �� �����
	bool load_channeldata(const std::string& path, std::map<std::string, channel>& container);
	//���������� ������ ��� ������� ������ � ����
	bool save_channeldata(const std::string& path, std::map<std::string, channel>& container);
	//�������� ������ �����-������ �� �����
	bool load_userdata(const std::string& path, std::map<std::string, std::string>& container);
	//���������� ������ �����-������ � ����
	bool save_userdata(const std::string& path, std::map<std::string, std::string>& container);
	//�������� ���� ��������� ��������� ����������� ��� ������� �������
	bool load_users_list_of_msg(const std::string& path, std::map<std::string, std::vector<std::string>>& container);
	//���������� ���� ��������� ��������� ����������� ��� ������� �������
	bool save_users_list_of_msg(const std::string& path, std::map<std::string, std::vector<std::string>>& container);
	//�������� ���� ��������� �������
	bool load_msgpool_of_channels(const std::string& path, std::map<std::string, std::map<std::string, std::vector<std::string>>>& container);
	//���������� ���� ��������� �������
	bool save_msgpool_of_channels(const std::string& path, std::map<std::string, std::map<std::string, std::vector<std::string>>>& container);
};

