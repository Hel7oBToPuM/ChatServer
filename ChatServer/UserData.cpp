#include "UserData.h"

bool UserData::setOnline()
{
	mute_usersocket_useronline.lock();
	if (usersocket.contains(user) == false) {
		usersocket[user] = Client;
		useronline.insert(user);
		mute_usersocket_useronline.unlock();
		return true;
	}
	mute_usersocket_useronline.unlock();
	return false;
}

void UserData::setOffline()
{
	mute_usersocket_useronline.lock();
	usersocket.erase(user);
	useronline.erase(user);
	mute_usersocket_useronline.unlock();
}

bool UserData::isPasswordCorrect(std::string& login, std::string& password)
{
	const auto it = userdata.find(login);
	if (it->second != password) {
		return false;
	}
	return true;
}

user_events UserData::findUser()
{
	int request_size;
	if (simpFunc.recvall(Client, (char*)&request_size, sizeof(int), NULL) == false) return EVENT_ERROR; //получаю размер запроса
	char* request = new char[request_size];
	if (simpFunc.recvall(Client, request, request_size, NULL) == false) {
		delete[] request;
		return EVENT_ERROR;
	}

	std::string username = std::string(request, request_size); //Преобразование без потерь и ошибок (проверено эмпирически)
	delete[] request;
	std::string response;
	mute_userdata.lock();
	for (const auto& data : userdata)
	{
		if (data.first.find(username) != std::string::npos && data.first != user) {
			response += data.first + char(31);
		}
	}
	mute_userdata.unlock();

	if (!response.empty())
	{
		response.erase(response.length() - 1);
		int response_length = response.length();
		int errorStat;
		errorStat = send(Client, (char*)&response_length, sizeof(int), NULL);
		if (errorStat == SOCKET_ERROR) return EVENT_ERROR;
		char* msg = new char[response_length];
		response.copy(msg, response_length);
		errorStat = send(Client, msg, response_length, NULL);
		if (errorStat == SOCKET_ERROR) {
			delete[] msg;
			return EVENT_ERROR;
		}
		delete[] msg;
	}
	else {
		int errorStat;
		int response_length = 0;
		errorStat = send(Client, (char*)&response_length, sizeof(int), NULL);
		if (errorStat == SOCKET_ERROR) return EVENT_ERROR;
	}
	return EVENT_COMPLETE;
}

user_events UserData::getUserStatus()
{
	int request_size;
	if (simpFunc.recvall(Client, (char*)&request_size, sizeof(int), NULL) == false) return EVENT_ERROR; //получаю размер запроса
	char* request = new char[request_size];
	if (simpFunc.recvall(Client, request, request_size, NULL) == false) {
		delete[] request;
		return EVENT_ERROR;
	}

	std::string username = std::string(request, request_size);
	delete[] request;
	mute_usersocket_useronline.lock();
	if (useronline.contains(username)) {
		mute_usersocket_useronline.unlock();
		int errorStat;
		bool response = true;
		errorStat = send(Client, (char*)&response, sizeof(bool), NULL);
		if (errorStat == SOCKET_ERROR) return EVENT_ERROR;
	}
	else {
		mute_usersocket_useronline.unlock();
		int errorStat;
		bool response = false;
		errorStat = send(Client, (char*)&response, sizeof(bool), NULL);
		if (errorStat == SOCKET_ERROR) return EVENT_ERROR;
	}
	return EVENT_COMPLETE;
}

user_events UserData::sendMessageToUser()
{
	int msg_size;
	if (simpFunc.recvall(Client, (char*)&msg_size, sizeof(int), NULL) == false) return EVENT_ERROR;
	char* msg_send = new char[msg_size];
	if (simpFunc.recvall(Client, msg_send, msg_size, NULL) == false) {
		delete[] msg_send;
		return EVENT_ERROR;
	}
	std::string message_form = std::string(msg_send, msg_size);
	delete[] msg_send;

	std::string reciever;
	simpFunc.makeMessageForSend(message_form, reciever, user);
	mute_userslistofmsg.lock();
	users_list_of_msg[reciever].push_back(message_form);
	mute_userslistofmsg.unlock();
	return EVENT_COMPLETE;
}

user_events UserData::checkMessages()
{
	int errorStat;
	std::string messages_packet;
	mute_userslistofmsg.lock();
	if (users_list_of_msg.contains(user)) {
		for (const std::string& message_form : users_list_of_msg[user])
		{
			messages_packet += message_form + char(31);
		}
		int messages_size = messages_packet.length();
		errorStat = send(Client, (char*)&messages_size, sizeof(int), NULL);
		if (errorStat == SOCKET_ERROR) {
			mute_userslistofmsg.unlock();
			return EVENT_ERROR;
		}
		char* messages = new char[messages_size];
		messages_packet.copy(messages, messages_size);
		errorStat = send(Client, messages, messages_size, NULL);
		if (errorStat == SOCKET_ERROR) {
			mute_userslistofmsg.unlock();
			delete[] messages;
			return EVENT_ERROR;
		}
		users_list_of_msg.erase(user);
		mute_userslistofmsg.unlock();
		delete[] messages;
		return EVENT_COMPLETE;
	}
	else {
		mute_userslistofmsg.unlock();
		int messages_size = 0;
		errorStat = send(Client, (char*)&messages_size, sizeof(int), NULL);
		if (errorStat == SOCKET_ERROR) return EVENT_ERROR;
		return EVENT_COMPLETE;
	}
}

user_events UserData::createChannel()
{
	int name_size;
	if (simpFunc.recvall(Client, (char*)&name_size, sizeof(int), NULL) == false) return EVENT_ERROR;
	char* name = new char[name_size];
	if (simpFunc.recvall(Client, name, name_size, NULL) == false) return EVENT_ERROR;
	std::string channel_name = std::string(name, name_size);
	delete[] name;
	int errorStat;
	mute_channels.lock();
	if (channels.contains(channel_name)) {
		mute_channels.unlock();
		errorStat=send(Client, (char*)&error, sizeof(bool), NULL);
		if (errorStat == SOCKET_ERROR) return EVENT_ERROR;
		return EVENT_COMPLETE;
	}
	channels[channel_name].creater = user;
	users_cr_channels[user].insert(channel_name);
	msgpool_of_channels[channel_name];
	mute_channels.unlock();
	errorStat = send(Client, (char*)&ok, sizeof(bool), NULL);
	if (errorStat == SOCKET_ERROR) return EVENT_ERROR;
	return EVENT_COMPLETE;
}

user_events UserData::deleteChannel() 
{
	int name_size;
	if (simpFunc.recvall(Client, (char*)&name_size, sizeof(int), NULL) == false) return EVENT_ERROR;
	char* name = new char[name_size];
	if (simpFunc.recvall(Client, name, name_size, NULL) == false) return EVENT_ERROR;
	std::string channel_name = std::string(name, name_size);
	delete[] name;
	mute_channels.lock();
	for (const std::string& member : channels[channel_name].members) {
		users_channels[member].erase(channel_name);
		if (users_channels[member].size() == 0)
		{
			users_channels.erase(member);
		}
	};
	users_cr_channels[user].erase(channel_name);
	if (users_cr_channels[user].size() == 0)
	{
		users_channels.erase(user);
	}
	channels.erase(channel_name);
	msgpool_of_channels.erase(channel_name);
	mute_channels.unlock();
	int errorStat;
	errorStat = send(Client, (char*)&ok, sizeof(bool), NULL);
	if (errorStat == SOCKET_ERROR) return EVENT_ERROR;
	return EVENT_COMPLETE;
};

user_events UserData::findChannel()
{
	int name_size;
	if (simpFunc.recvall(Client, (char*)&name_size, sizeof(int), NULL) == false) return EVENT_ERROR;
	char* name = new char[name_size];
	if (simpFunc.recvall(Client, name, name_size, NULL) == false) return EVENT_ERROR;
	std::string channel_name = std::string(name, name_size);
	delete[] name;
	
	std::string response;
	mute_channels.lock();
	for (const auto& channel : channels)
	{
		if (channel.first.find(channel_name) != std::string::npos) {
			response += channel.first + char(31);
		}
	}
	mute_channels.unlock();

	if (!response.empty())
	{
		response.erase(response.length() - 1);
		int response_length = response.length();
		int errorStat;
		errorStat = send(Client, (char*)&response_length, sizeof(int), NULL);
		if (errorStat == SOCKET_ERROR) return EVENT_ERROR;
		char* msg = new char[response_length];
		response.copy(msg, response_length);
		errorStat = send(Client, msg, response_length, NULL);
		if (errorStat == SOCKET_ERROR) {
			delete[] msg;
			return EVENT_ERROR;
		}
		delete[] msg;
	}
	else {
		int errorStat;
		int response_length = 0;
		errorStat = send(Client, (char*)&response_length, sizeof(int), NULL);
		if (errorStat == SOCKET_ERROR) return EVENT_ERROR;
	}
	return EVENT_COMPLETE;
}

user_events UserData::connectToChannel()
{
	int name_size;
	if (simpFunc.recvall(Client, (char*)&name_size, sizeof(int), NULL) == false) return EVENT_ERROR;
	char* name = new char[name_size];
	if (simpFunc.recvall(Client, name, name_size, NULL) == false) return EVENT_ERROR;
	std::string channel_name = std::string(name, name_size);
	delete[] name;

	mute_channels.lock();
	channels[channel_name].members.insert(user);
	users_channels[user].insert(channel_name);
	mute_channels.unlock();

	int errorStat;
	errorStat = send(Client, (char*)&ok, sizeof(bool), NULL);
	if (errorStat == SOCKET_ERROR) return EVENT_ERROR;
	return EVENT_COMPLETE;
}

user_events UserData::disconnectFromChannel()
{
	int name_size;
	if (simpFunc.recvall(Client, (char*)&name_size, sizeof(int), NULL) == false) return EVENT_ERROR;
	char* name = new char[name_size];
	if (simpFunc.recvall(Client, name, name_size, NULL) == false) return EVENT_ERROR;
	std::string channel_name = std::string(name, name_size);
	delete[] name;

	mute_channels.lock();
	channels[channel_name].members.erase(user);
	users_channels[user].erase(channel_name);
	if (users_channels[user].size() == 0)
	{
		users_channels.erase(user);
	}
	msgpool_of_channels[channel_name].erase(user);
	mute_channels.unlock();

	int errorStat;
	errorStat = send(Client, (char*)&ok, sizeof(bool), NULL);
	if (errorStat == SOCKET_ERROR) return EVENT_ERROR;
	return EVENT_COMPLETE;
}

user_events UserData::addToChannel()
{
	int name_size;
	if (simpFunc.recvall(Client, (char*)&name_size, sizeof(int), NULL) == false) return EVENT_ERROR;
	char* name = new char[name_size];
	if (simpFunc.recvall(Client, name, name_size, NULL) == false) return EVENT_ERROR;
	std::string channel_name = std::string(name, name_size);
	delete[] name;

	int username_size;
	if (simpFunc.recvall(Client, (char*)&username_size, sizeof(int), NULL) == false) return EVENT_ERROR;
	char* username = new char[username_size];
	if (simpFunc.recvall(Client, username, username_size, NULL) == false) return EVENT_ERROR;
	std::string user_name = std::string(username, username_size);
	delete[] username;

	int errorStat;
	mute_userdata.lock();
	if (userdata.contains(user_name))
	{
		mute_userdata.unlock();
		mute_channels.lock();
		if (!channels[channel_name].members.contains(user_name))
		{
			channels[channel_name].members.insert(user_name);
			users_channels[user_name].insert(channel_name);
			mute_channels.unlock();
			errorStat = send(Client, (char*)&ok, sizeof(bool), NULL);
			if (errorStat == SOCKET_ERROR) return EVENT_ERROR;
			return EVENT_COMPLETE;
		}
		else
		{
			mute_channels.unlock();
			errorStat = send(Client, (char*)&error, sizeof(bool), NULL);
			if (errorStat == SOCKET_ERROR) return EVENT_ERROR;
			return EVENT_COMPLETE;
		}
	}

	mute_userdata.unlock();
	errorStat = send(Client, (char*)&error, sizeof(bool), NULL);
	if (errorStat == SOCKET_ERROR) return EVENT_ERROR;
	return EVENT_COMPLETE;
}

user_events UserData::sendMessageToChannel()
{
	int msg_size;
	if (simpFunc.recvall(Client, (char*)&msg_size, sizeof(int), NULL) == false) return EVENT_ERROR;
	char* msg_send = new char[msg_size];
	if (simpFunc.recvall(Client, msg_send, msg_size, NULL) == false) {
		delete[] msg_send;
		return EVENT_ERROR;
	}
	std::string message_form = std::string(msg_send, msg_size);
	delete[] msg_send;

	std::string channel_name = message_form.substr(0, message_form.find(':'));
	message_form = message_form.substr(message_form.find(':')+1);
	mute_channels.lock();
	for (const std::string& member : channels[channel_name].members) 
	{
		if (member != user)
			msgpool_of_channels[channel_name][member].push_back(message_form);
	}
	mute_channels.unlock();
	int errorStat;
	errorStat = send(Client, (char*)&ok, sizeof(bool), NULL);
	if (errorStat == SOCKET_ERROR) return EVENT_ERROR;
	return EVENT_COMPLETE;
}

user_events UserData::checkMessagesFromChannels()
{
	int errorStat;
	std::string messages_packet;
	mute_channels.lock();

	if (users_channels.contains(user)) //если пользователь состоит в каком-то канале
	{
		for (const std::string& channel : users_channels[user])  //для каждого канала в котором состоит пользователь
		{
			if (!msgpool_of_channels[channel][user].empty()) //если для него в пуле есть сообщения
			{
				for (const auto& message : msgpool_of_channels[channel][user]) //каждое сообщение
				{
					messages_packet += channel + char(30) + message + char(31);
				}
				msgpool_of_channels[channel][user].clear(); //удалить сообщения в пуле из этого канала для пользователя
			}
		}
	}
	else
	{
		mute_channels.unlock();
		int messages_size = 0;
		errorStat = send(Client, (char*)&messages_size, sizeof(int), NULL);
		if (errorStat == SOCKET_ERROR) return EVENT_ERROR;
		return EVENT_COMPLETE;
	}

	if (messages_packet.length() == 0)
	{
		mute_channels.unlock();
		int messages_size = 0;
		errorStat = send(Client, (char*)&messages_size, sizeof(int), NULL);
		if (errorStat == SOCKET_ERROR) return EVENT_ERROR;
		return EVENT_COMPLETE;
	}
	else
	{
		int messages_size = messages_packet.length();
		errorStat = send(Client, (char*)&messages_size, sizeof(int), NULL);
		if (errorStat == SOCKET_ERROR) {
			mute_channels.unlock();
			return EVENT_ERROR;
		}
		char* messages = new char[messages_size];
		messages_packet.copy(messages, messages_size);
		errorStat = send(Client, messages, messages_size, NULL);
		if (errorStat == SOCKET_ERROR) {
			mute_channels.unlock();
			delete[] messages;
			return EVENT_ERROR;
		}
		mute_channels.unlock();
		delete[] messages;
		return EVENT_COMPLETE;
	}
}

user_events UserData::checkListOfChannels()
{
	int errorStat;
	int channels_size;
	if (simpFunc.recvall(Client, (char*)&channels_size, sizeof(int), NULL) == false) return EVENT_ERROR;
	if (channels_size == 0)
	{
		errorStat = send(Client, (char*)&ok, sizeof(bool), NULL);
		if (errorStat == SOCKET_ERROR) return EVENT_ERROR;
		return EVENT_COMPLETE;
	}
	char* channels_msg = new char[channels_size];
	if (simpFunc.recvall(Client, channels_msg, channels_size, NULL) == false) {
		delete[] channels_msg;
		return EVENT_ERROR;
	}
	std::string channels_name = std::string(channels_msg, channels_size);
	delete[] channels_msg;

	int response_size;
	std::string response;
	mute_channels.lock();
	if (!users_channels[user].empty()) //если список каналов пользователя не пуст (где он, как участник)
	{
		std::set<std::string> user_channels;
		while (channels_name.find(char(31)) != std::string::npos) //парсинг списка каналов пользователя
		{
			user_channels.insert(channels_name.substr(0, channels_name.find(char(31))));
			channels_name = channels_name.substr(channels_name.find(char(31)) + 1);
		}
		for (const std::string& serv_channel : users_channels[user]) //если у пользователя нет какого-то канала из списка, хранящегося на сервере
		{
			if (!user_channels.contains(serv_channel))
			{
				response += serv_channel + char(31);
			}
		}
		for (const std::string& user_channel : user_channels)
		{
			if (!users_channels[user].contains(user_channel))
			{
				response += user_channel + char(31);
			}
		}
		response_size = response.length();
		if (response_size == 0)
		{
			errorStat = send(Client, (char*)&ok, sizeof(bool), NULL);
			if (errorStat == SOCKET_ERROR) {
				mute_channels.unlock();
				return EVENT_ERROR;
			}
			mute_channels.unlock();
			return EVENT_COMPLETE;
		}
		else
		{
			errorStat = send(Client, (char*)&error, sizeof(bool), NULL);
			if (errorStat == SOCKET_ERROR) {
				mute_channels.unlock();
				return EVENT_ERROR;
			}
			errorStat = send(Client, (char*)&response_size, sizeof(int), NULL);
			if (errorStat == SOCKET_ERROR)
			{
				mute_channels.unlock();
				return EVENT_ERROR;
			}
			char* response_msg = new char(response_size);
			response.copy(response_msg, response_size);
			errorStat = send(Client, response_msg, response_size, NULL);
			if (errorStat == SOCKET_ERROR)
			{
				mute_channels.unlock();
				return EVENT_ERROR;
			}
			mute_channels.unlock();
		}
	}
	else //если список каналов пуст
	{
		errorStat = send(Client, (char*)&error, sizeof(bool), NULL);
		if (errorStat == SOCKET_ERROR)
		{
			mute_channels.unlock();
			return EVENT_ERROR;
		}
		response_size = 0;
		errorStat = send(Client, (char*)&response_size, sizeof(int), NULL);
		if (errorStat == SOCKET_ERROR)
		{
			mute_channels.unlock();
			return EVENT_ERROR;
		}
		mute_channels.unlock();
	}
	return EVENT_COMPLETE;
}

user_events UserData::waitForAction()
{
	user_events user_event;
	if (simpFunc.recvall(Client, (char*)&user_event, sizeof(user_event), NULL) == false) return EVENT_ERROR;
	return user_event;
}

bool UserData::registerClient(std::string& log_pass)
{
	std::string login, password;
	simpFunc.getLoginPassword(log_pass, login, password);
	mute_userdata.lock();
	if (userdata.contains(login) == true) {
		mute_userdata.unlock();
		return false;
	}
	userdata[login] = password;
	mute_userdata.unlock();
	setOnline();
	user = login;
	return true;
}

bool UserData::loginClient(std::string& log_pass)
{
	std::string login, password;
	simpFunc.getLoginPassword(log_pass, login, password);
	mute_userdata.lock();
	if (userdata.contains(login) == true) {
		if (isPasswordCorrect(login, password) == true) {
			user = login;
			if (setOnline() == true) {
				mute_userdata.unlock();
				return true;
			}
		}
	}
	mute_userdata.unlock();
	return false;
}

bool UserData::authClient()
{
	while (true)
	{
		int errorStat;
		bool reg_or_log; // true - регистрация; false - вход
		char msg[33]; //буфер для принятия данных
		std::string log_pass; //буфер для работы с данными
		char msgH[] = "Hello!";

		if (simpFunc.recvall(Client, (char*)&reg_or_log, sizeof(bool), NULL) == false) return false;
		if (reg_or_log == true) // регистрация
		{
			if (simpFunc.recvall(Client, msg, sizeof(msg), NULL) == false) return false;
			log_pass = std::string(msg, sizeof(msg));
			if (registerClient(log_pass) == false)
			{
				errorStat = send(Client, (char*)&error, sizeof(bool), NULL);
				if (errorStat == SOCKET_ERROR) return false;
				continue;
			}
			else
			{
				servermsg.lock();
				std::cout << "Клиент " << user << " зарегистрирован!" << std::endl;
				servermsg.unlock();
				errorStat = send(Client, (char*)&ok, sizeof(bool), NULL);
				if (errorStat == SOCKET_ERROR) return false;
				break;
			}
		}
		else // вход
		{
			if (simpFunc.recvall(Client, msg, sizeof(msg), NULL) == false) return false;
			log_pass = std::string(msg, sizeof(msg));
			if (loginClient(log_pass) == false)
			{
				errorStat = send(Client, (char*)&error, sizeof(bool), NULL);
				if (errorStat == SOCKET_ERROR) return false;
				continue;
			}
			else
			{
				servermsg.lock();
				std::cout << "Клиент " << user << " вошел!" << std::endl;
				servermsg.unlock();
				errorStat = send(Client, (char*)&ok, sizeof(bool), NULL);
				if (errorStat == SOCKET_ERROR) return false;
				break;
			}
		}
	}
	return true;
}

user_events UserData::firstDataRequest()
{
	int errorStat;
	mute_userslistofmsg.lock();
	if (users_list_of_msg.contains(user))
	{
		std::string response;
		for (const auto& msg_form : users_list_of_msg[user])
		{
			response += msg_form + char(31);
		}
		int response_size = response.length();
		errorStat = send(Client, (char*)&response_size, sizeof(int), NULL);
		if (errorStat == SOCKET_ERROR) {
			mute_userslistofmsg.unlock();
			return EVENT_ERROR;
		}
		if (response_size != 0)
		{
			char* response_msg = new char[response_size];
			response.copy(response_msg, response_size);
			errorStat = send(Client, response_msg, response_size, NULL);
			if (errorStat == SOCKET_ERROR) {
				mute_userslistofmsg.unlock();
				delete[] response_msg;
				return EVENT_ERROR;
			}
			delete[] response_msg;
		}
	}
	else
	{
		int response_size = 0;
		errorStat = send(Client, (char*)&response_size, sizeof(int), NULL);
		if (errorStat == SOCKET_ERROR) {
			mute_userslistofmsg.unlock();
			return EVENT_ERROR;
		}
	}
	users_list_of_msg.erase(user);
	mute_userslistofmsg.unlock();

	mute_usersocket_useronline.lock();
	{
		std::string response;
		for (const auto& username : useronline)
			if (username != user) response += username + char(31);
		int response_size = response.length();
		errorStat = send(Client, (char*)&response_size, sizeof(int), NULL);
		if (errorStat == SOCKET_ERROR) {
			mute_usersocket_useronline.unlock();
			return EVENT_ERROR;
		}
		if (response_size != 0)
		{
			char* response_msg = new char[response_size];
			response.copy(response_msg, response_size);
			errorStat = send(Client, response_msg, response_size, NULL);
			if (errorStat == SOCKET_ERROR) {
				mute_usersocket_useronline.unlock();
				return EVENT_ERROR;
			}
		}
	}
	mute_usersocket_useronline.unlock();

	mute_channels.lock();
	if (users_channels.contains(user))
	{
		std::string response;
		for (const auto& channel_name : users_channels[user]) 
				response += channel_name + char(31);
		int response_size = response.length();
		errorStat = send(Client, (char*)&response_size, sizeof(int), NULL);
		if (errorStat == SOCKET_ERROR) {
			mute_channels.unlock();
				return EVENT_ERROR;
		}
		char* response_msg = new char[response_size];
		response.copy(response_msg, response_size);
		errorStat = send(Client, response_msg, response_size, NULL);
		if (errorStat == SOCKET_ERROR) {
			mute_channels.unlock();
			return EVENT_ERROR;
		}
	}
	else
	{
		int response_size = 0;
		errorStat = send(Client, (char*)&response_size, sizeof(int), NULL);
		if (errorStat == SOCKET_ERROR) {
			mute_channels.unlock();
			return EVENT_ERROR;
		}
	}

	if (users_cr_channels.contains(user))
	{
		std::string response;
		for (const auto& channel_name : users_cr_channels[user])
			response += channel_name + char(31);
		int response_size = response.length();
		errorStat = send(Client, (char*)&response_size, sizeof(int), NULL);
		if (errorStat == SOCKET_ERROR) {
			mute_channels.unlock();
			return EVENT_ERROR;
		}
		char* response_msg = new char[response_size];
		response.copy(response_msg, response_size);
		errorStat = send(Client, response_msg, response_size, NULL);
		if (errorStat == SOCKET_ERROR) {
			mute_channels.unlock();
			return EVENT_ERROR;
		}
	}
	else
	{
		int response_size = 0;
		errorStat = send(Client, (char*)&response_size, sizeof(int), NULL);
		if (errorStat == SOCKET_ERROR) {
			mute_channels.unlock();
			return EVENT_ERROR;
		}
	}

	{
		std::string response;
		int response_size = 0;
		if (users_channels.contains(user))
		{
			for (const auto& channel_name : users_channels[user])
			{
				if (msgpool_of_channels[channel_name].contains(user))
				{
					for (const auto& msg_form : msgpool_of_channels[channel_name][user])
					{
						response += msg_form + char(31);
					}
				}
			}
			response_size = response.length();
			if (response_size != 0)
			{
				errorStat = send(Client, (char*)&response_size, sizeof(int), NULL);
				if (errorStat == SOCKET_ERROR) {
					mute_channels.unlock();
					return EVENT_ERROR;
				}
				char* response_msg = new char[response_size];
				response.copy(response_msg, response_size);
				errorStat = send(Client, response_msg, response_size, NULL);
				if (errorStat == SOCKET_ERROR) {
					mute_channels.unlock();
					return EVENT_ERROR;
				}
				for (const auto& channel_name : users_channels[user])
				{
					msgpool_of_channels[channel_name].erase(user);
				}
			}
			else
			{
				errorStat = send(Client, (char*)&response_size, sizeof(int), NULL);
				if (errorStat == SOCKET_ERROR) {
					mute_channels.unlock();
					return EVENT_ERROR;
				}
			}
		}
		else
		{
			errorStat = send(Client, (char*)&response_size, sizeof(int), NULL);
			if (errorStat == SOCKET_ERROR) {
				mute_channels.unlock();
				return EVENT_ERROR;
			}
		}
	}
	mute_channels.unlock();
	return EVENT_COMPLETE;
}

UserData::UserData(SOCKET socketClient)
{
	Client = socketClient;
}