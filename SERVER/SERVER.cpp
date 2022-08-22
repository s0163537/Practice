#include <winsock2.h>
#include <windows.h>
#include <string>
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)
using namespace std;

SOCKET Connections[100];
//массив сокетов активных клиентов 
int Counter = 0;//число активных клиентов
int S = 0;
bool Packet;//допустимые типы пакетов (Pack,Test)
//Функционал потока отдельного клиента 
DWORD WINAPI ServerThread(LPVOID number) {
    bool packettype;
    int* pointer_int = (int*)number;// определение номера сокета клиента
    int index = *pointer_int;
    index--;
    cout << "socket number=" << index << endl;
    SOCKET Con = Connections[index];
    while (true) {  // общение с клиентом
        recv(Con, (char*)&packettype, sizeof(Packet), NULL);// получение информации от клиента
        //определение типа полученного пакета
        if (packettype == 1) {
            cout << "recieve packet PACK" << endl;
            int msg_size, toWhom;
            recv(Con, (char*)&toWhom, sizeof(int), NULL);
            //определение объема пакета
            recv(Con, (char*)&msg_size, sizeof(int), NULL);
            char* msg = new char[msg_size + 1]; //резервирование буфера нужного размера для принятия пакета
            msg[msg_size] = 0;
            recv(Con, msg, msg_size, NULL);  // получение пакета
            cout << "Client" << index << ": " << msg << endl;
            //передача полученного сообщения другим участникам чата
            if (toWhom == -1) {// если toWhom == -1, сообщение отправляется всем участникам чата, иначе определенному участнику
                for (int i = 0; i < S+1; i++)
                {
                    if (i == index)continue;
                    bool msgtype = 1; //(Pack)
                    //передача типа, объема и содержания информационного пакета 
                    send(Connections[i], (char*)&msgtype, sizeof(Packet), NULL);
                    send(Connections[i], (char*)&index, sizeof(int), NULL);
                    send(Connections[i], (char*)&msg_size, sizeof(int), NULL);
                    send(Connections[i], msg, msg_size, NULL);
                }
                delete[] msg;
            }
            else {
                bool msgtype = 1;
                send(Connections[toWhom], (char*)&msgtype, sizeof(Packet), NULL);
                send(Connections[toWhom], (char*)&index, sizeof(int), NULL);
                send(Connections[toWhom], (char*)&msg_size, sizeof(int), NULL);
                send(Connections[toWhom], msg, msg_size, NULL);
                delete[] msg;
            }
        }
        else {
            cout << "Unrecognized packet: " << packettype << endl; break;
        }
    }
    closesocket(Con);
    return 0;
}
int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    //инициализация winsock
    WSAData wsaData;        //WSAStartup
    WORD DllVersion = MAKEWORD(2, 1);
    if (WSAStartup(DllVersion, &wsaData) != 0)
    {
        cout << "Error" << endl; exit(1);
    }
    //сохранение в слушающем сокете информации о сервере 
    SOCKADDR_IN addr;
    int sizeofaddr = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("192.168.1.176");
    addr.sin_port = htons(123);
    addr.sin_family = AF_INET;
    SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
    bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
    //режим прослушивания, организация очереди 
    listen(sListen, 10);
    SOCKET newConnection;
    while(Counter<100){
        //извлечение запросов из очереди 
        int i = Counter;
        newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);
        if (newConnection == 0) { cout << "Error #2\n"; }
        else {
            cout << "Client Connected!\n"; // приветствие нового клиента 
            string msg = "Welcome to chat!";
            int msg_size = msg.size();
            //передача клиенту пакета типа pack
            bool msgtype = 1;
            send(newConnection, (char*)&msgtype, sizeof(Packet), NULL);
            send(newConnection, (char*)&Counter, sizeof(int), NULL);
            send(newConnection, (char*)&msg_size, sizeof(int), NULL);
            send(newConnection, (char*)&msg[0], msg_size, NULL);
            //сохранение сокета клиента в массиве участников чата
            Connections[Counter] = newConnection;
            cout << "count=" << Counter << endl;
            CreateThread(NULL, NULL, ServerThread, &Counter, NULL, NULL);//создание нового потока для обслуживания клиента
            Counter++;
            S++;
            //передача клиенту сигнального сообщения 
            bool testpacket = 0;
            cout << "type packet=TEST" << endl;
            send(newConnection, (char*)&testpacket, sizeof(Packet), NULL);
        }
    }
    return 0;
}