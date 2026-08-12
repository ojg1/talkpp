#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>

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

    if (bindStatus == SOCKET_ERROR) {std::cout << "An error occured while binding. Error code: " << bindStatus << "\n";};      

    auto listenStatus = listen(talkSocket, 10);

    if (listenStatus == SOCKET_ERROR) {std::cout << "An error occured while listening. Error code: " << listenStatus << "\n";};

    std::vector<SOCKET> Clients = {};

    u_long blockingIdentifier = 1;
    ioctlsocket(talkSocket, FIONBIO, &blockingIdentifier);

    bool runServer = true;
    while (runServer) {

        SOCKET clientSocket = accept(talkSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET && WSAGetLastError() == WSAEWOULDBLOCK) {
            //no client, continue
        } else {
            Clients.push_back(clientSocket);
        };

        for (const auto& sock : Clients) {
            
        };


    };


    WSACleanup();

    return 0;
};