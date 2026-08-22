#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <conio.h>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include "serverNetwork.hpp"

int main(){

    AllocConsole();
    
    FILE* NewConsole;
    freopen_s(&NewConsole, "CONOUT$", "w", stdout);
    freopen_s(&NewConsole, "CONOUT$", "w", stderr);
    freopen_s(&NewConsole, "CONIN$", "r", stdin);

    std::cout << "Server Starting\n";
    
    WSADATA wsaData;
    int status = WSAStartup(MAKEWORD(2,2),&wsaData);

    if (status != 0) {std::cout << "An error occured while starting WSA. Error code: " << status;};

    int AddressFamily = AF_INET;
    int Type = SOCK_STREAM; //TCP
    int Protocol = IPPROTO_TCP;
    SOCKET talkSocket = socket(AddressFamily, Type, Protocol);

    sockaddr_in SocketAddress{};
    SocketAddress.sin_family = AF_INET;
    SocketAddress.sin_port = htons(930);
    SocketAddress.sin_addr.s_addr  = INADDR_ANY;

    auto bindStatus = bind(talkSocket, (SOCKADDR*)&SocketAddress, sizeof(SocketAddress));

    if (bindStatus == SOCKET_ERROR) {std::cout << "An error occured while binding. Error: " << WSAGetLastError() << "\n";};      

    auto listenStatus = listen(talkSocket, 10);

    if (listenStatus == SOCKET_ERROR) {std::cout << "An error occured while listening. Error: " << WSAGetLastError() << "\n";};

    std::vector<SOCKET> Clients = {};

    u_long blockingIdentifier = 1;
    ioctlsocket(talkSocket, FIONBIO, &blockingIdentifier);

    bool runServer = true;
    std::cout << "Server is running!\n";
    while (runServer) {

        // if (_kbhit()) {
        //     char key = _getch();

        //     if (key == 'q') {
        //         runServer = false;
        //     }
        // }

        SOCKET clientSocket = accept(talkSocket, nullptr, nullptr);
       
        if (clientSocket == INVALID_SOCKET) {
            if (WSAGetLastError() != WSAEWOULDBLOCK) {
                std::cout << "Client socket is invalid. Error code:" << WSAGetLastError() << "\n";
            };
        } else {
            std::cout << "------NEWSOCKET------\n";
            int socketError = 0;
            int optLen = sizeof(socketError);

            int result = getsockopt(clientSocket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&socketError), &optLen);

            if (result == SOCKET_ERROR) {
                std::cout << "getsockopt failed: "
                        << WSAGetLastError() << "\n";
            } else {
                std::cout << "Socket SO_ERROR: " << socketError << "\n";
            }

            Clients.push_back(clientSocket);
            std::cout << "accepted client: " << clientSocket << "\n";
        };

        SOCKET disconnectCli = INVALID_SOCKET;

        // const auto& a : b in which a can not be modified during the loop
        for (const auto& sock : Clients) {

            TalkServerNetwork TSNet;

            //Recieve string
            std::string ReceiveResult = TSNet.RecieveClientNetworkData(&sock, &Clients, &disconnectCli);
            std::cout << ReceiveResult << std::endl;
            //Send string to rest of clients
            for (const auto& subSock : Clients) {
                if (subSock == sock) {
                    continue;
                } else {
                    std::string SendResult = TSNet.SendClientNetworkData(&subSock,  ReceiveResult);
                }            
            }
        };

        if (disconnectCli != INVALID_SOCKET) {
            Clients.erase(
                std::remove(Clients.begin(), Clients.end(), disconnectCli),
                Clients.end()
            );
        };
        
    };

    WSACleanup();

    return 0;
};