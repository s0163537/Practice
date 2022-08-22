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
int Counter = 0;
int S = 0;
bool Packet;
DWORD WINAPI ServerThread(LPVOID number) {
    bool packettype;
    int* pointer_int = (int*)number;
    int index = *pointer_int;
    index--;
    cout << "socket number=" << index << endl;
    SOCKET Con = Connections[index];
    while (true) {
        recv(Con, (char*)&packettype, sizeof(Packet), NULL);
        if (packettype == 1) {
            cout << "recieve packet PACK" << endl;
            int msg_size, toWhom;
            recv(Con, (char*)&toWhom, sizeof(int), NULL);
            recv(Con, (char*)&msg_size, sizeof(int), NULL);
            char* msg = new char[msg_size + 1];
            msg[msg_size] = 0;
            recv(Con, msg, msg_size, NULL);  
            cout << "Client" << index << ": " << msg << endl;
            if (toWhom == -1) {
                for (int i = 0; i < S+1; i++)
                {
                    if (i == index)continue;
                    bool msgtype = 1;
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
    WSAData wsaData;        //WSAStartup
    WORD DllVersion = MAKEWORD(2, 1);
    if (WSAStartup(DllVersion, &wsaData) != 0)
    {
        cout << "Error" << endl; exit(1);
    }
    SOCKADDR_IN addr;
    int sizeofaddr = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("192.168.1.176");
    addr.sin_port = htons(123);
    addr.sin_family = AF_INET;
    SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
    bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
    listen(sListen, 10);
    SOCKET newConnection;
    while(Counter<100){
        int i = Counter;
        newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);
        if (newConnection == 0) { cout << "Error #2\n"; }
        else {
            cout << "Client Connected!\n";
            string msg = "Welcome to chat!";
            int msg_size = msg.size();
            bool msgtype = 1;
            send(newConnection, (char*)&msgtype, sizeof(Packet), NULL);
            send(newConnection, (char*)&Counter, sizeof(int), NULL);
            send(newConnection, (char*)&msg_size, sizeof(int), NULL);
            send(newConnection, (char*)&msg[0], msg_size, NULL);
            Connections[Counter] = newConnection;
            cout << "count=" << Counter << endl;
            CreateThread(NULL, NULL, ServerThread, &Counter, NULL, NULL);
            Counter++;
            S++;
            bool testpacket = 0;
            cout << "type packet=TEST" << endl;
            send(newConnection, (char*)&testpacket, sizeof(Packet), NULL);
        }
    }
    return 0;
}