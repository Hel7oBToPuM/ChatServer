#include <thread>
#include "UserData.h"
#include <future>
#include <chrono>

const char Ipv4[] = "127.0.0.1"; //Текущий Ip-адрес сервера
const int port = 1000; //Текущий порт сервера
const std::string userdata_path = "userdata.txt"; //Название файла, в котором будут храниться (или хранятся) логины и пароли клиентов 
const std::string users_channels_path = "users_channels.txt"; //Название файла, в котором будут храниться (или хранятся) каналы, к которым подключен пользователь +
const std::string users_cr_channels_path = "users_cr_channels.txt"; //Название файла, в котором будут храниться (или хранятся) каналы, которые создал пользователь +
const std::string users_list_of_msg_path = "users_list_of_msg.txt"; //Название файла, в котором будут храниться (или хранятся) все сообщения ожидающие отправления для каждого клиента
const std::string channels_path = "channels.txt"; //Название файла, в котором будут храниться (или хранятся) все название-канал
const std::string msgpool_of_channels_path = "msgpool_of_channels.txt"; //Название файла, в котором будут храниться (или хранятся) каналы, которым соответстуют пользователи, которым соотвестувуют неполученные ими сообщения из канала

bool ok = true; //ок
bool error = false; //ошибка

std::map<std::string, std::set<std::string>> users_cr_channels; //Каналы, которые создал пользователь |Нужно передавать клиенту при авторизации| |Сохранять в файл|
std::map<std::string, std::set<std::string>> users_channels; //Каналы, к которым подключен пользователь |Нужно передавать клиенту при авторизации| |Сохранять в файл|
std::map<std::string, std::vector<std::string>> users_list_of_msg; //Хранит все сообщения ожидающие отправления для каждого клиента |Нужно передавать клиенту при авторизации| |Сохранять в файл|
std::map<std::string, std::string> userdata; //Хранит логин-пароль для каждого клиент |Сохранять в файл|
std::map<std::string, channel> channels;//Хранит все название-канал |Сохранять в файл|
std::map<std::string, SOCKET> usersocket; //Содержит пару имя пользователя - сокет, если клиент подключен
std::set<std::string> useronline; //Содержит пользователей в статусе online |Нужно передавать клиенту при авторизации|
std::map<std::string, std::map<std::string, std::vector<std::string>>> msgpool_of_channels; //Хранит каналы, которым соответстуют пользователи, которым соотвестувуют неполученные ими сообщения из канала |Сохранять в файл| |Нужно передавать клиенту при авторизации|

std::mutex mute_channels; //блокировка для потоков channels (защита данных)
std::mutex mute_userdata; //блокировка для потоков userdata (защита данных)
std::mutex mute_usersocket_useronline; //блокировка для потоков usersocket (защита данных)
std::mutex servermsg; //Блокировка для серверных сообщений (защита данных)
std::mutex mute_userslistofmsg; //блокировка для потоков users_list_of_msg (защита данных)

//Цикл работы с клиентом, запускаемый в потоке
void workWithClient(SOCKET Client)
{
	UserData user_data(Client); //объект класса взаимодействия с клиентом
	servermsg.lock();
	std::cout << "Попытка авторизации!" << std::endl;
	servermsg.unlock();
	if (user_data.authClient() == false)
	{
		if (user_data.user.empty()) {
			servermsg.lock();
			std::cout << "Попытка авторизации не удалась!" << std::endl;
			servermsg.unlock();
		}
		else {
			servermsg.lock();
			std::cout << "Клиент " << user_data.user << " отключился!" << std::endl;
			servermsg.unlock();
		}
		user_data.setOffline();
		closesocket(Client);
		return;
	}
	while (true)
	{
		user_events action = user_data.waitForAction();
		switch (action)
		{
			case FIRST_DATA_REQUEST:
			{
				if (user_data.firstDataRequest() == EVENT_ERROR)
				{
					servermsg.lock();
					std::cout << "Клиент " << user_data.user << " отключился!" << std::endl;
					servermsg.unlock();
					user_data.setOffline();
					closesocket(Client);
					return;
				}
				break;
			}
			case FIND_USER:
			{
				if (user_data.findUser() == EVENT_ERROR)
				{
					servermsg.lock();
					std::cout << "Клиент " << user_data.user << " отключился!" << std::endl;
					servermsg.unlock();
					user_data.setOffline();
					closesocket(Client);
					return;
				}
				break;
			}
			case CHECK_ONLINE:
			{
				if (user_data.getUserStatus() == EVENT_ERROR)
				{
					servermsg.lock();
					std::cout << "Клиент " << user_data.user << " отключился!" << std::endl;
					servermsg.unlock();
					user_data.setOffline();
					closesocket(Client);
					return;
				}
				break;
			}
			case SEND_MESSAGE_TO_USER:
			{
				if (user_data.sendMessageToUser() == EVENT_ERROR)
				{
					servermsg.lock();
					std::cout << "Клиент " << user_data.user << " отключился!" << std::endl;
					servermsg.unlock();
					user_data.setOffline();
					closesocket(Client);
					return;
				}
				break;
			}
			case CHECK_MESSAGES_FROM_USERS:
			{
				if (user_data.checkMessages() == EVENT_ERROR)
				{
					servermsg.lock();
					std::cout << "Клиент " << user_data.user << " отключился!" << std::endl;
					servermsg.unlock();
					user_data.setOffline();
					closesocket(Client);
					return;
				}
				break;
			}
			case CREATE_CHANNEL:
			{
				if (user_data.createChannel() == EVENT_ERROR)
				{
					servermsg.lock();
					std::cout << "Клиент " << user_data.user << " отключился!" << std::endl;
					servermsg.unlock();
					user_data.setOffline();
					closesocket(Client);
					return;
				}
				break;
			}
			case DELETE_CHANNEL:
			{
				if (user_data.deleteChannel() == EVENT_ERROR)
				{
					servermsg.lock();
					std::cout << "Клиент " << user_data.user << " отключился!" << std::endl;
					servermsg.unlock();
					user_data.setOffline();
					closesocket(Client);
					return;
				}
				break;
			}
			case FIND_CHANNEL:
			{
				if (user_data.findChannel() == EVENT_ERROR)
				{
					servermsg.lock();
					std::cout << "Клиент " << user_data.user << " отключился!" << std::endl;
					servermsg.unlock();
					user_data.setOffline();
					closesocket(Client);
					return;
				}
				break;
			}
			case CONNECT_TO_CHANNEL:
			{
				if (user_data.connectToChannel() == EVENT_ERROR)
				{
					servermsg.lock();
					std::cout << "Клиент " << user_data.user << " отключился!" << std::endl;
					servermsg.unlock();
					user_data.setOffline();
					closesocket(Client);
					return;
				}
				break;
			}
			case DISCONNECT_FROM_CHANNEL:
			{
				if (user_data.disconnectFromChannel() == EVENT_ERROR)
				{
					servermsg.lock();
					std::cout << "Клиент " << user_data.user << " отключился!" << std::endl;
					servermsg.unlock();
					user_data.setOffline();
					closesocket(Client);
					return;
				}
				break;
			}
			case ADD_TO_CHANNEL_USER:
			{
				if (user_data.addToChannel() == EVENT_ERROR)
				{
					servermsg.lock();
					std::cout << "Клиент " << user_data.user << " отключился!" << std::endl;
					servermsg.unlock();
					user_data.setOffline();
					closesocket(Client);
					return;
				}
				break;
			}
			case SEND_MESSAGE_TO_CHANNEL:
			{
				if (user_data.sendMessageToChannel() == EVENT_ERROR)
				{
					servermsg.lock();
					std::cout << "Клиент " << user_data.user << " отключился!" << std::endl;
					servermsg.unlock();
					user_data.setOffline();
					closesocket(Client);
					return;
				}
				break;
			}
			case CHECK_MESSAGES_FROM_CHANNELS:
			{
				if (user_data.checkMessagesFromChannels() == EVENT_ERROR)
				{
					servermsg.lock();
					std::cout << "Клиент " << user_data.user << " отключился!" << std::endl;
					servermsg.unlock();
					user_data.setOffline();
					closesocket(Client);
					return;
				}
				break;
			}
			case CHECK_LIST_OF_CHANNELS:
			{
				if (user_data.checkListOfChannels() == EVENT_ERROR)
				{
					servermsg.lock();
					std::cout << "Клиент " << user_data.user << " отключился!" << std::endl;
					servermsg.unlock();
					user_data.setOffline();
					closesocket(Client);
					return;
				}
				break;
			}
			case EVENT_ERROR:
			{
				servermsg.lock();
				std::cout << "Клиент " << user_data.user << " отключился!" << std::endl;
				servermsg.unlock();
				user_data.setOffline();
				closesocket(Client);
				return;
			}
		}
	}
}

//ожидание ввода со стороны сервера для его закрытия
int waitingForClose()
{
	getchar();
	return 0;
}

//ожидание новых подключений
void waitingForConnections(SOCKET serverListen)
{
	while (true)
	{
		SOCKET newConnection = accept(serverListen, NULL, NULL);
		if (newConnection == 0 || newConnection == INVALID_SOCKET) {
			if (newConnection == INVALID_SOCKET) break;
			std::cout << "Ошибка подключения клиента!" << std::endl;
		}
		else {
			std::thread(workWithClient, newConnection).detach(); //Создание отделенного потока для обработки клиента
		}
	}
}

int main()
{
	setlocale(LC_ALL, "Russian");
	simpleFunctions sF;
	if ((sF.load_userdata(userdata_path, userdata) && sF.load_users_list_of_msg(users_list_of_msg_path, users_list_of_msg) &&
		sF.load_channeldata(channels_path, channels) && sF.load_userschannels(users_channels_path, users_channels) &&
		sF.load_msgpool_of_channels(msgpool_of_channels_path, msgpool_of_channels)) == false) return 0;
	//Инициализация библиотеки сокетов
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cout << "Ошибка запуска библиотеки сокетов!" << std::endl << "Закрытие сервера...";
		return 1;
	}

	//Инициализация сокета для прослушивания
	SOCKET serverListen = socket(AF_INET, SOCK_STREAM, NULL);
	if (serverListen == INVALID_SOCKET) {
		std::cout << "Ошибка инициализации прослушивающего сокета!" << std::endl << "Закрытие сервера...";
		closesocket(serverListen);
		WSACleanup();
		return 1;
	}

	//Заполнение структуры, используемой для привязки сокета к адресу
	SOCKADDR_IN serverInfo;
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr = sF.getIpv4FromChar(Ipv4);
	serverInfo.sin_port = htons(port);

	//Привязка прослушивающего сокета к адресу
	if (bind(serverListen, (SOCKADDR*)&serverInfo, sizeof(serverInfo)) != 0) {
		std::cout << "Ошибка привязки сокета к адресу" << std::endl << "Закрытие сервера...";
		closesocket(serverListen);
		WSACleanup();
		return 1;
	}
	else
		std::cout << "Сокет привязан к адресу: " << Ipv4 << ":" << port << "!" << std::endl;

	//Прослушивание по адресу
	if (listen(serverListen, SOMAXCONN) != 0) {
		std::cout << "Сервер не может начать прослушивание по адресу!" << std::endl << "Закрытие сервера...";
		closesocket(serverListen);
		WSACleanup();
		return 1;
	}
	else
		std::cout << "Сервер прослушивает..." << std::endl;

	std::thread(waitingForConnections, serverListen).detach(); // Запуск цикла создания новых подключений в отдельный поток
	std::future<int> stop = std::async(std::launch::async, waitingForClose); //асинхронный запуск ожидания выключения сервера;
	std::future_status status = stop.wait_for(std::chrono::milliseconds(10));
	while (status != std::future_status::ready) {
		status = stop.wait_for(std::chrono::milliseconds(10));
		if (status == std::future_status::ready) {
			stop.get();
			break;
		}
	}
	shutdown(serverListen, 2);
	closesocket(serverListen);
	std::this_thread::sleep_for(std::chrono::seconds(5));
	WSACleanup();
	if ((sF.save_userdata(userdata_path, userdata) && sF.save_users_list_of_msg(users_list_of_msg_path, users_list_of_msg) &&
		sF.save_channeldata(channels_path, channels) && sF.save_userschannels(users_channels_path, users_channels) &&
		sF.save_msgpool_of_channels(msgpool_of_channels_path, msgpool_of_channels)) == false)
	{
		std::cout << "ВО ВРЕМЯ СОХРАНЕНИЯ ВЫЯВЛЕНЫ ОШИБКИ! ЧАСТЬ ДАННЫХ УТЕРЯНА!";
	}
	return 0;
}