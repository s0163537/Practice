#include <winsock2.h>
#include <windows.h>
#include <string>
#include <mysql.h>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)
using namespace std;

SOCKET Connection;
bool Packet;
DWORD WINAPI ClientThread(LPVOID sock) {
    SOCKET Con = ((SOCKET*)sock)[0];
    bool packettype;
    while (1) {
        recv(Con, (char*)&packettype, sizeof(Packet), NULL);
        if (packettype == 1)
        {
            cout << "type packet=PACK" << endl;
            int msg_size, index;
            recv(Con, (char*)&index, sizeof(int), NULL);
            recv(Con, (char*)&msg_size, sizeof(int), NULL); 
            char* msg = new char[msg_size + 1];
            msg[msg_size] = '\0';
            recv(Con, msg, msg_size, NULL);
            cout << "Client" << index << ": " << msg << endl;
            delete[] msg;
        }
        else {
            if (packettype == 0)
                cout << "Test packet.\n";  
            else
            {
                cout << "Unrecognized packet: " << packettype << endl;       
                break;
            }
        }
    }
    closesocket(Connection); 	
    return 0;
}

int main() {    // with threads
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    WSAData wsaData;    
    WORD Ver = MAKEWORD(2, 1);
    if (WSAStartup(Ver, &wsaData) != 0) {
        cout << "Error" << endl;   
        return 1;
    }
    int sign_up_in;
    string login, password;
    bool database_flag = false;
    cout << "������� 1 ��� �����, ���� 2 ��� �����������" << endl;
    cin >> sign_up_in;
    //����� ����� ������� ����������� � ��
    MYSQL* conn;    
    // �������� ���������� ����������
    conn = mysql_init(NULL);
    if (conn == NULL) {
        // ���� ���������� �� ������� � ������� ��������� �� ������
        fprintf(stderr, "Error: can't create MySQL-descriptor\n");
    }
    // ������������ � �������
    if (!mysql_real_connect(conn, "localhost", "root", "pass", "chat", 3306, NULL, 0)) {
        // ���� ��� ����������� ���������� ���������� � ��������
        // ���� ������ ������� ��������� �� ������
        fprintf(stderr, "Error: can'tconnect to database %s\n", mysql_error(conn));
    }
    else {
        //��������� �������� � ���� �����
        if (sign_up_in == 1) {
            //����� ��������� ����� � ������ � ������������, ��������� ���� �� ����� � �� � ����� ��������� ����, ���� ��� ���� �������� - ���������� ������ ���������
            cout << "������� ����� � ������ ��� �����\n";
            int tries = 0;
            while (tries < 3 && !database_flag) {
                cout << "�����: ";
                cin >> login;
                cout << "������: ";
                cin >> password;
                MYSQL_RES* res_set;
                MYSQL_ROW row;
                string stmt1 = "select user_login, user_password from users where user_login='" + login + "' and user_password='" + password + "'";
                mysql_query(conn, stmt1.c_str());
                res_set = mysql_store_result(conn);
                int numrows = mysql_num_rows(res_set);
                if (numrows == NULL) {
                    cout << "�������� ����� ��� ������\n";
                    tries++;
                }
                else {
                    database_flag = true;
                }
            }
        }
        else {
            //����� ������ ����� ����� � ������ ��� ������
            if (sign_up_in == 2) {
                cout << "������� ����� ����� � ������\n������ �� ������ ��������� ����������� ������\n(. , ? : ! ~ * % ^ & � �.�.)\n";
                bool login_ok = false;
                while (!login_ok) {
                    cout << "�����: ";
                    cin >> login;
                    cout << "������: ";
                    cin >> password;
                    //��������� ���� �� ����� ������ � ��, ���� ����, ��������� ������ ������
                    MYSQL_RES* res_set;
                    MYSQL_ROW row;
                    string stmt1 = "select user_login from users where user_login='" + login + "'";
                    mysql_query(conn, stmt1.c_str());
                    res_set = mysql_store_result(conn);
                    int numrows = mysql_num_rows(res_set);
                    if (numrows == NULL) {
                        //� ����� ��������
                        string stmt_ins = "insert into users values ( '" + login + "' , '" + password + "')";
                        mysql_query(conn, stmt_ins.c_str());
                        login_ok = true;
                    }
                    else {
                        cout << "��� ���������� ������������ � ����� �������, ������� ������\n";
                    }
                }
                database_flag = true;
            }
            else {
                cout << "\n������������ ����!\n";
            }
        }
    }
    // ��������� ���������� � �������� ���� ������
    mysql_close(conn);
    //��������� ���������� �� ���� ����� ��
    if (database_flag) {
        SOCKADDR_IN addr;
        int sizeofaddr = sizeof(addr);
        addr.sin_addr.s_addr = inet_addr("192.168.1.176");
        addr.sin_port = htons(123);
        addr.sin_family = AF_INET;
        Connection = socket(AF_INET, SOCK_STREAM, NULL);
        if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0)
        {
            cout << "Error: failed connect to server.\n";            
            return 1;
        }
        //system("cls");
        cout << "Connected!\n����� ����������, " + login + "\n";
        cout << "��� �������� ��������� ������� ����� ���� ���������,\n� ����� ����� �������, ��������\n������ ��������� ���������(-1 ���� ������ ��������� ����)\n";
        CreateThread(NULL, NULL, ClientThread, &Connection, NULL, NULL);
        string msg1;
        int toWhom;
        while (true) {
            cin >> msg1;
            cin >> toWhom;
            int msg_size = msg1.size();
            bool packettype = 1;
            send(Connection, (char*)&packettype, sizeof(Packet), NULL);
            send(Connection, (char*)&toWhom, sizeof(int), NULL);
            send(Connection, (char*)&msg_size, sizeof(int), NULL);
            send(Connection, (char*)&msg1[0], msg_size, NULL);
            Sleep(10);
        }
        system("pause");
        return 0;
    }
    else {
        fprintf(stdout, "\n�� ������� ����� ��� ������������������!\n");
        return 0;
    }
}