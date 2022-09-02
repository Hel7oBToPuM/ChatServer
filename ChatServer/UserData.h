#pragma once
#include <mutex>
#include <set>
#include "simpleFunctions.h"



//Перечисляемый тип для всех действий клиента
enum user_events
{
	SEND_MESSAGE_TO_USER, //Отправить сообщение другому пользователю +
	SEND_MESSAGE_TO_CHANNEL, //Отправить сообщение в канал +
	CREATE_CHANNEL, //Создать канал +
	CONNECT_TO_CHANNEL, //Подключиться к каналу +
	DISCONNECT_FROM_CHANNEL, //Отключиться от канала +
	FIND_USER, //Найти пользователя +
	DELETE_CHANNEL, //Удалить канал +
	EVENT_ERROR, // Ошибка +
	FIND_CHANNEL, // Найти канал +
	ADD_TO_CHANNEL_USER, //Добавить в канал пользователя +
	EVENT_COMPLETE, // Действие клиента успешно обработано +
	CHECK_ONLINE, //Проверить доступность другого клиента +
	EVENT_ERROR_WITHOUT_DISCONNECT, // Ошибка без отключения пользователя +
	CHECK_MESSAGES_FROM_USERS, //Проверка входящих сообщений от пользователей +
	CHECK_MESSAGES_FROM_CHANNELS, //Проверка входящих сообщений от пользователей +
	CHECK_LIST_OF_CHANNELS,//Проверка актуальности каналов для пользователя +
	FIRST_DATA_REQUEST //Запрос данных при аутентификации +
};

//Хранение всех сообщений, отправляющихся клиенту

extern bool ok; //ок
extern bool error; //ошибка

extern std::map<std::string, std::set<std::string>> users_channels; //Каналы, к которым подключен пользователь
extern std::map<std::string, std::vector<std::string>> users_list_of_msg; //Хранит все сообщения ожидающие отправления для каждого клиента
extern std::map<std::string, std::string> userdata; //Хранит логин-пароль для каждого клиент
extern std::map<std::string, channel> channels; //Хранит все название-канал;
extern std::map<std::string, SOCKET> usersocket; //Содержит пару имя пользователя - сокет, если клиент подключен
extern std::set<std::string> useronline; //Содержит пользователей в статусе online
extern std::map<std::string, std::set<std::string>> users_cr_channels; //Каналы, которые создал пользователь
extern std::map<std::string, std::map<std::string, std::vector<std::string>>> msgpool_of_channels; //Хранит каналы, которым соответстуют пользователи, которым соотвестувуют неполученные ими сообщения из канала

extern std::mutex mute_userdata; //блокировка для потоков userdata (защита данных)
extern std::mutex mute_usersocket_useronline; //блокировка для потоков usersocket (защита данных)
extern std::mutex servermsg; //Блокировка для серверных сообщений
extern std::mutex mute_userslistofmsg; //блокировка для потоков users_list_of_msg (защита данных)
extern std::mutex mute_channels; //блокировка для потоков channels (защита данных)

//Класс для взаимодействия с клиентом
class UserData
{
public:
	simpleFunctions simpFunc;
	SOCKET Client; //Сокет пользователя
	std::string user; //пользователь, с которым работает объект класса
	//Конструктор
	UserData(SOCKET socketClient);
	//Перевести клиента в онлайн
	bool setOnline();
	//Перевести клиента в оффлайн
	void setOffline();
	//Проверка наличия пароля
	bool isPasswordCorrect(std::string& login, std::string& password);
	// Найти пользователя
	user_events findUser();
	//Функция ожидания действия клиента
	user_events waitForAction();
	//Получение статуса (онлайн/оффлайн) пользователя
	user_events getUserStatus();
	//Отправить сообщение пользователю
	user_events sendMessageToUser();
	//Проверить и получить входящее сообщения
	user_events checkMessages();
	//Создать канал
	user_events createChannel();
	//Удалить канал
	user_events deleteChannel();
	//Найти канал
	user_events findChannel();
	//Подключиться к каналу
	user_events connectToChannel();
	//Отключиться от канала
	user_events disconnectFromChannel();
	//Добавить в канал пользователя
	user_events addToChannel();
	//Отправить сообщение в канал
	user_events sendMessageToChannel();
	//Отправить сообщение в канал
	user_events checkMessagesFromChannels();
	//Проверка актуальности списка чатов для пользователя
	user_events checkListOfChannels();
	//Запрос данных после авторизации
	user_events firstDataRequest();
	//Регистрация клиента в базе
	bool registerClient(std::string& log_pass);
	//Проверка данных клиента в базе для входа
	bool loginClient(std::string& log_pass);
	//Функция цикла авторизации клиента
	bool authClient();
};