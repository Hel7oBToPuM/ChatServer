#include "simpleFunctions.h"

bool simpleFunctions::exist(const std::string& path) {
	std::ifstream f(path);
	return f.good();
}

in_addr simpleFunctions::getIpv4FromChar(const char* ip)
{
	in_addr ip_to_num;
	inet_pton(AF_INET, ip, &ip_to_num);
	return ip_to_num;
}

bool simpleFunctions::recvall(SOCKET& socket, char* msg, int length, int flag)
{
	int total;
	total = recv(socket, msg, length, flag);
	if (total == 0 || total == SOCKET_ERROR) return false;
	while (total < length)
	{
		int i;
		i = recv(socket, msg + total, length - total, flag);
		if (i == SOCKET_ERROR || i == 0) return false;
		total += i;
	}
	return true;
}

void simpleFunctions::getLoginPassword(std::string& log_pass, std::string& login, std::string& password)
{
	size_t n = log_pass.find(' ');
	if (n == std::string::npos) {
		login = log_pass.substr(0, 16);
		password = log_pass.substr(17, 16);
	}
	else if (n <= 15) {
		login = log_pass.substr(0, n);
		log_pass = log_pass.substr(17);
		password = log_pass.substr(0, log_pass.find(' '));
	}
	else {
		login = log_pass.substr(0, 16);
		log_pass = log_pass.substr(17);
		password = log_pass.substr(0, log_pass.find(' '));
	}
}

void simpleFunctions::makeMessageForSend(std::string& message_form, std::string& reciever, const std::string& sender)
{
	reciever = message_form.substr(0, message_form.find(char(30)));
	message_form = message_form.substr(message_form.find(char(30)));
	message_form.insert(0, sender);
}

bool simpleFunctions::load_userschannels(const std::string& path, std::map<std::string, std::set<std::string>>& container)
{
	if (exist(path)) {
		std::cout << "Файл " << path << " с каналами клиентов найден!" << std::endl << "Загрузка даннных из файла..." << std::endl;
		std::ifstream f(path);
		if (!f.is_open()) {
			std::cout << "Ошибка открытия файла!" << std::endl;
			f.close();
			return false;
		}
		while (true)
		{
			std::string record;
			getline(f, record);
			if (f.eof()) break;
			std::string username = record.substr(0, record.find(char(30)));
			record = record.substr(record.find(char(30)) + 1);
			while (record.find(char(31)) != std::string::npos)
			{
				container[username].insert(record.substr(record.find(char(31))));
				record = record.substr(record.find(char(31)) + 1);
			}
		}
		std::cout << "Данные загружены!" << std::endl;
		f.close();
	}
	else {
		std::cout << "Файл " << path << " с каналами клиентов отсутствует!" << std::endl << "Создание нового файла..." << std::endl;
		std::ofstream f(path);
		if (!f.is_open()) {
			std::cout << "Ошибка создания файла!" << std::endl;
			f.close();
			return false;
		}
		std::cout << "Файл создан!" << std::endl;
		f.close();
	}
	return true;
}

bool simpleFunctions::save_userschannels(const std::string& path, std::map<std::string, std::set<std::string>>& container)
{
	if (exist(path)) {
		std::cout << "Файл " << path << " с каналами клиентов найден!" << std::endl << "Сохранение даннных в файл..." << std::endl;
		std::ofstream f(path);
		if (!f.is_open()) {
			std::cout << "Ошибка открытия файла!" << std::endl;
			f.close();
			return false;
		}
		for (const auto& record : container)
		{
			f << record.first+char(30);
			for (const std::string& channel : record.second)
			{
				f << channel + char(31);
			}
			f << '\n';
		}
		std::cout << "Данные сохранены!" << std::endl;
	}
	else
	{
		std::cout << "Файл " << path << " с каналами клиентов отсутствует!" << std::endl << "Создание нового файла..." << std::endl;
		std::ofstream f(path);
		if (!f.is_open()) {
			std::cout << "Ошибка создания файла!" << std::endl;
			f.close();
			return false;
		}
		std::cout << "Файл создан!" << std::endl << "Сохранение даннных в файл..." << std::endl;
		for (const auto& record : container)
		{
			f << record.first + char(30);
			for (const std::string& channel : record.second)
			{
				f << channel + char(31);
			}
			f << '\n';
		}
		std::cout << "Данные сохранены!" << std::endl;
	}
	return true;
}

bool simpleFunctions::load_userdata(const std::string& path, std::map<std::string, std::string>& container)
{
	if (exist(path)) {
		std::cout << "Файл " << path << " с данными клиентов найден!" << std::endl << "Загрузка даннных из файла..." << std::endl;
		std::ifstream f(path);
		if (!f.is_open()) {
			std::cout << "Ошибка открытия файла!" << std::endl;
			f.close();
			return false;
		}
		while (true)
		{
			std::string record;
			getline(f, record);
			if (f.eof()) break;
			container[record.substr(0, record.find((char(31))))] = record.substr(record.find((char(31))) + 1);
		}
		std::cout << "Данные загружены!" << std::endl;
		f.close();
	}
	else {
		std::cout << "Файл " << path << " с данными клиентов отсутствует!" << std::endl << "Создание нового файла..." << std::endl;
		std::ofstream f;
		f.open(path);
		if (!f.is_open()) {
			std::cout << "Ошибка создания файла!" << std::endl;
			f.close();
			return false;
		}
		std::cout << "Файл создан!" << std::endl;
		f.close();
	}
	return true;
}

bool simpleFunctions::save_userdata(const std::string& path, std::map<std::string, std::string>& container)
{
	if (exist(path)) {
		std::cout << "Файл " << path << " с данными клиентов найден!" << std::endl << "Сохранение даннных в файл..." << std::endl;
		std::ofstream f(path);
		if (!f.is_open()) {
			std::cout << "Ошибка открытия файла!" << std::endl;
			f.close();
			return false;
		}
		for (const auto& record : container)
		{
			f << record.first + char(31) + record.second + '\n';
		}
		std::cout << "Данные сохранены!" << std::endl;
	}
	else
	{
		std::cout << "Файл " << path << " с данными клиентов отсутствует!" << std::endl << "Создание нового файла..." << std::endl;
		std::ofstream f(path);
		if (!f.is_open()) {
			std::cout << "Ошибка создания файла!" << std::endl;
			f.close();
			return false;
		}
		std::cout << "Файл создан!" << std::endl << "Сохранение даннных в файл..." << std::endl;
		for (const auto& record : container)
		{
			f << record.first + char(31) + record.second + '\n';
		}
		std::cout << "Данные сохранены!" << std::endl;
	}
	return true;
}

bool simpleFunctions::load_channeldata(const std::string& path, std::map<std::string, channel>& container)
{
	if (exist(path)) {
		std::cout << "Файл " << path << " с каналами  найден!" << std::endl << "Загрузка даннных из файла..." << std::endl;
		std::ifstream f(path);
		if (!f.is_open()) {
			std::cout << "Ошибка открытия файла!" << std::endl;
			f.close();
			return false;
		}
		while (true)
		{
			std::string record;
			getline(f, record);
			if (f.eof()) break;
			std::string channel_name = record.substr(0, record.find(char(30)));
			record = record.substr(record.find(char(30))+1);
			container[channel_name].creater = record.substr(0, record.find(char(30)));
			record = record.substr(record.find(char(30)) + 1);
			while (record.find(char(31)) != std::string::npos)
			{
				container[channel_name].members.insert(record.substr(record.find(char(31))));
				record = record.substr(record.find(char(31)) + 1);
			}
		}
		std::cout << "Данные загружены!" << std::endl;
		f.close();
	}
	else {
		std::cout << "Файл " << path << " с каналами  отсутствует!" << std::endl << "Создание нового файла..." << std::endl;
		std::ofstream f(path);
		if (!f.is_open()) {
			std::cout << "Ошибка создания файла!" << std::endl;
			f.close();
			return false;
		}
		std::cout << "Файл создан!" << std::endl;
		f.close();
	}
	return true;
}
bool simpleFunctions::save_channeldata(const std::string& path, std::map<std::string, channel>& container)
{
	if (exist(path)) {
		std::cout << "Файл " << path << " с каналами найден!" << std::endl << "Сохранение даннных в файл..." << std::endl;
		std::ofstream f(path);
		if (!f.is_open()) {
			std::cout << "Ошибка открытия файла!" << std::endl;
			f.close();
			return false;
		}
		for (const auto& record : container)
		{
			f << record.first + char(30) + record.second.creater + char(30);
			for (const auto& member : record.second.members)
			{
				f << member + char(31);
			}
			f << '\n';
		}
		std::cout << "Данные сохранены!" << std::endl;
	}
	else
	{
		std::cout << "Файл " << path << " с каналами отсутствует!" << std::endl << "Создание нового файла..." << std::endl;
		std::ofstream f(path);
		if (!f.is_open()) {
			std::cout << "Ошибка создания файла!" << std::endl;
			f.close();
			return false;
		}
		std::cout << "Файл создан!" << std::endl << "Сохранение даннных в файл..." << std::endl;
		for (const auto& record : container)
		{
			f << record.first + char(30) + record.second.creater + char(30);
			for (const auto& member : record.second.members)
			{
				f << member + char(31);
			}
			f << '\n';
		}
		std::cout << "Данные сохранены!" << std::endl;
	}
	return true;
}

bool simpleFunctions::load_users_list_of_msg(const std::string& path, std::map<std::string, std::vector<std::string>>& container)
{
	if (exist(path)) {
		std::cout << "Файл " << path << " с сообщениями для пользователей найден!" << std::endl << "Загрузка даннных из файла..." << std::endl;
		std::ifstream f(path);
		if (!f.is_open()) {
			std::cout << "Ошибка открытия файла!" << std::endl;
			f.close();
			return false;
		}
		while (true)
		{
			std::string record;
			getline(f, record);
			if (f.eof()) break;
			std::string username = record.substr(0, record.find(char(30)));
			record = record.substr(record.find(char(30)) + 1);
			while (record.find(char(31)) != std::string::npos)
			{
				container[username].push_back(record.substr(0, record.find(char(31))));
				record = record.substr(record.find(char(31)) + 1);
			}
		}
		std::cout << "Данные загружены!" << std::endl;
		f.close();
	}
	else {
		std::cout << "Файл " << path << " с сообщениями для пользователей  отсутствует!" << std::endl << "Создание нового файла..." << std::endl;
		std::ofstream f(path);
		if (!f.is_open()) {
			std::cout << "Ошибка создания файла!" << std::endl;
			f.close();
			return false;
		}
		std::cout << "Файл создан!" << std::endl;
		f.close();
	}
	return true;
}
bool simpleFunctions::save_users_list_of_msg(const std::string& path, std::map<std::string, std::vector<std::string>>& container)
{
	if (exist(path)) {
		std::cout << "Файл " << path << " с сообщениями для пользователей найден!" << std::endl << "Сохранение даннных в файл..." << std::endl;
		std::ofstream f(path);
		if (!f.is_open()) {
			std::cout << "Ошибка открытия файла!" << std::endl;
			f.close();
			return false;
		}
		for (const auto& record : container)
		{
			f << record.first + char(30);
			for (const auto& msg : record.second)
			{
				f << msg + char(31);
			}
			f << '\n';
		}
		std::cout << "Данные сохранены!" << std::endl;
	}
	else
	{
		std::cout << "Файл " << path << " с сообщениями для пользователей отсутствует!" << std::endl << "Создание нового файла..." << std::endl;
		std::ofstream f(path);
		if (!f.is_open()) {
			std::cout << "Ошибка создания файла!" << std::endl;
			f.close();
			return false;
		}
		std::cout << "Файл создан!" << std::endl << "Сохранение даннных в файл..." << std::endl;
		for (const auto& record : container)
		{
			f << record.first + char(30);
			for (const auto& msg : record.second)
			{
				f << msg + char(31);
			}
			f << '\n';
		}
		std::cout << "Данные сохранены!" << std::endl;
	}
	return true;
}

bool simpleFunctions::load_msgpool_of_channels(const std::string& path, std::map<std::string, std::map<std::string, std::vector<std::string>>>& container)
{
	if (exist(path)) {
		std::cout << "Файл " << path << " с сообщениями для пользователей найден!" << std::endl << "Загрузка даннных из файла..." << std::endl;
		std::ifstream f(path);
		if (!f.is_open()) {
			std::cout << "Ошибка открытия файла!" << std::endl;
			f.close();
			return false;
		}
		std::string channel_name;
		while (true)
		{
			std::string record;
			getline(f, record);
			if (f.eof()) break;
			if (record.find(char(30)) == std::string::npos)
			{
				channel_name = record;
				container[channel_name];
				continue;
			}
			std::string username = record.substr(0, record.find(char(30)));
			record = record.substr(record.find(char(30)) + 1);
			while (record.find(char(31)) != std::string::npos)
			{
				container[channel_name][username].push_back(record.substr(0, record.find(char(31))));
				record = record.substr(record.find(char(31)) + 1);
			}
		}
		std::cout << "Данные загружены!" << std::endl;
		f.close();
	}
	else {
		std::cout << "Файл " << path << " с сообщениями для пользователей  отсутствует!" << std::endl << "Создание нового файла..." << std::endl;
		std::ofstream f(path);
		if (!f.is_open()) {
			std::cout << "Ошибка создания файла!" << std::endl;
			f.close();
			return false;
		}
		std::cout << "Файл создан!" << std::endl;
		f.close();
	}
	return true;
}
bool simpleFunctions::save_msgpool_of_channels(const std::string& path, std::map<std::string, std::map<std::string, std::vector<std::string>>>& container)
{
	if (exist(path)) {
		std::cout << "Файл " << path << " с сообщениями для пользователей найден!" << std::endl << "Сохранение даннных в файл..." << std::endl;
		std::ofstream f(path);
		if (!f.is_open()) {
			std::cout << "Ошибка открытия файла!" << std::endl;
			f.close();
			return false;
		}
		for (const auto& record : container)
		{
			f << record.first+'\n';
			for (const auto& under_record : record.second)
			{
				f << under_record.first + char(30);
				for (const auto& msg : under_record.second)
				{
					f << msg + char(31);
				}
			}
			f << '\n';
		}
		std::cout << "Данные сохранены!" << std::endl;
	}
	else
	{
		std::cout << "Файл " << path << " с сообщениями для пользователей отсутствует!" << std::endl << "Создание нового файла..." << std::endl;
		std::ofstream f(path);
		if (!f.is_open()) {
			std::cout << "Ошибка создания файла!" << std::endl;
			f.close();
			return false;
		}
		std::cout << "Файл создан!" << std::endl << "Сохранение даннных в файл..." << std::endl;
		for (const auto& record : container)
		{
			f << record.first + '\n';
			for (const auto& under_record : record.second)
			{
				f << under_record.first + char(30);
				for (const auto& msg : under_record.second)
				{
					f << msg + char(31);
				}
			}
			f << '\n';
		}
		std::cout << "Данные сохранены!" << std::endl;
	}
	return true;
}