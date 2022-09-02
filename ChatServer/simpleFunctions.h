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

//канал
struct channel
{
	std::string creater; //создатель канала
	std::set<std::string> members; //участники канала
};

class simpleFunctions
{
private:
	//Проверка наличия файла
	bool exist(const std::string& path);
public:
	//Парсер пакета на логин и пароль
	void getLoginPassword(std::string& log_pass, std::string& login, std::string& password);
	//Функция получения всех данных по TCP/IP
	bool recvall(SOCKET& socket, char* msg, int length, int flag);
	//Функция преобразования ip из char в байтовый формат
	in_addr getIpv4FromChar(const char* ip);
	//Преобразование полученного message-пакета в message-пакет для отправки
	void makeMessageForSend(std::string& message_form, std::string& reciever, const std::string& sender);
	//Загрузка каналов для каждого клиента из файла
	bool load_userschannels(const std::string& path, std::map<std::string, std::set<std::string>> &container);
	//Сохранение каналов для каждого клиента в файл
	bool save_userschannels(const std::string& path, std::map<std::string, std::set<std::string>>& container);
	//Загрузка данных для каждого канала из файла
	bool load_channeldata(const std::string& path, std::map<std::string, channel>& container);
	//Сохранение данных для каждого канала в файл
	bool save_channeldata(const std::string& path, std::map<std::string, channel>& container);
	//Загрузка данных логин-пароль из файла
	bool load_userdata(const std::string& path, std::map<std::string, std::string>& container);
	//Сохранение данных логин-пароль в файл
	bool save_userdata(const std::string& path, std::map<std::string, std::string>& container);
	//Загрузка всех сообщений ожидающих отправления для каждого клиента
	bool load_users_list_of_msg(const std::string& path, std::map<std::string, std::vector<std::string>>& container);
	//Сохранение всех сообщений ожидающих отправления для каждого клиента
	bool save_users_list_of_msg(const std::string& path, std::map<std::string, std::vector<std::string>>& container);
	//Загрузка пула сообщений каналов
	bool load_msgpool_of_channels(const std::string& path, std::map<std::string, std::map<std::string, std::vector<std::string>>>& container);
	//Сохранение пула сообщений каналов
	bool save_msgpool_of_channels(const std::string& path, std::map<std::string, std::map<std::string, std::vector<std::string>>>& container);
};

