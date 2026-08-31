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
#include "quickutils.hpp" 

void plog(std::string ansi, std::string header, std::string message) {
    std::cout << ansi << "[" << header << "]\x1b[0m" << message << "\x1b[0m\n";
    return;
};

int main(){

    //Allocate Console
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
    std::cout << "\x1b[0;38;5;10;49m[servertpp]\x1b[0m Server is running!\n";
    while (runServer) {
 
        SOCKET clientSocket = accept(talkSocket, nullptr, nullptr);
       
        if (clientSocket == INVALID_SOCKET) {
            int erracc = WSAGetLastError();

            if (erracc != WSAEWOULDBLOCK) {
                plog("\x1b[1;38;5;11;49m", "talksocketinfo", "An error occured while accepting a client. Error Code: " + erracc);
            } else if (erracc == WSAECONNRESET) {
                closesocket(clientSocket);
            };
        } else {
            plog("\x1b[1;38;5;11;49m", "talksocketinfo", "Validating client...");
            int socketError = 0;
            int optLen = sizeof(socketError);

            int result = getsockopt(clientSocket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&socketError), &optLen);

            if (result == SOCKET_ERROR) {
                plog("\x1b[1;38;5;11;49m", "talksocketinfo", "getsockopt() failed. Error Code: " + std::to_string(WSAGetLastError()));
            } else {
                plog("\x1b[1;38;5;11;49m", "talksocketinfo", "Socket last error: " + std::to_string(socketError));
            }

            Clients.push_back(clientSocket);

            plog("\x1b[1;38;5;11;49m", "talksocketinfo", "\x1b[1;3;38;5;46;49mconnection success");
            sockaddr_in clientAddress;
            int size = sizeof(clientAddress);
            getpeername(clientSocket, (sockaddr*)&clientAddress, &size);
            plog("\x1b[1;38;5;11;49m", "talksocketinfo", "Client IP: " + (std::string)inet_ntoa(clientAddress.sin_addr));
        };

        SOCKET disconnectCli = INVALID_SOCKET;

        // const auto& a : b in which a can not be modified during the loop
        for (const auto& sock : Clients) {

            TalkServerNetwork TSNet;

            //Recieve string
            ReceiveResult RecvResult = TSNet.RecieveClientNetworkData(&sock);
            if (RecvResult.recieveStr != "" && RecvResult.status == 0 && RecvResult.recieveStr != "ServerWaiting") {

                std::string clientip;
                sockaddr_in clsad;
                int sizes = sizeof(clsad);

                getpeername(sock, (sockaddr*)&clsad, &sizes);
                clientip = (std::string)inet_ntoa(clsad.sin_addr);
                plog("\x1b[1;38;5;11;49m", "talksocketinfo", "From Client:" + clientip + ", recieve result: " + RecvResult.recieveStr);
            
            } else if (RecvResult.status == 1) {
                closesocket(sock);
                disconnectCli = sock;
            };
            //Send string to rest of clients
            for (const auto& subSock : Clients) {
                if (subSock == sock) {
                    continue;
                } else {
                    std::string SendResult = TSNet.SendClientNetworkData(&subSock, RecvResult.recieveStr);
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