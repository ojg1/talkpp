#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>
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

    if (bindStatus == SOCKET_ERROR) {std::cout << "An error occured while binding. Error: " << WSAGetLastError() << "\n";};      

    auto listenStatus = listen(talkSocket, 10);

    if (listenStatus == SOCKET_ERROR) {std::cout << "An error occured while listening. Error: " << WSAGetLastError() << "\n";};

    std::vector<SOCKET> Clients = {};

    u_long blockingIdentifier = 1;
    ioctlsocket(talkSocket, FIONBIO, &blockingIdentifier);

    bool runServer = true;
    while (runServer) {

        SOCKET clientSocket = accept(talkSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            if (WSAGetLastError() != WSAEWOULDBLOCK) {
                std::cout << "Client socket is invalid.\n";
            };
        } else {
            Clients.push_back(clientSocket);
        };

        SOCKET disconnectCli;

        // const auto& a : b in which a can not be modified during the loop
        for (const auto& sock : Clients) {

            char clirecvbuffer[1024];
            bool cliDisconnected = false;
            int occupiedBytes = 0;

            while (occupiedBytes < 2) {            
                int clirecv = recv(talkSocket, clirecvbuffer + occupiedBytes, 1024, 0);
                if (clirecv == 0 || clirecv == SOCKET_ERROR) {
                    std::cout << "Client Disconnected or encountered an error.\n";
                    cliDisconnected = true;
                    break;
                };
                occupiedBytes = occupiedBytes + clirecv;
            };

            if (cliDisconnected) {
                disconnectCli = sock;
                continue;
            };

            //set byte buffer
            uint16_t length;
            memcpy(&length, clirecvbuffer, sizeof(length));

            while (occupiedBytes < length) {            
                int clirecv = recv(talkSocket, clirecvbuffer + occupiedBytes, 1024, 0);
                if (clirecv == 0 || clirecv == SOCKET_ERROR) {
                    std::cout << "Client Disconnected or encountered an error.\n";
                    cliDisconnected = true;
                    break;
                };
                occupiedBytes = occupiedBytes + clirecv;
            };

            if (cliDisconnected) {
                disconnectCli = sock;
                continue;
            };

            //if data sent from client applicable then send all clients

            std::string clientMessage(clirecvbuffer+2, occupiedBytes-2);

            for (const auto& sock : Clients) {

                

            };

        };

        if (disconnectCli) {
            Clients.erase(
                std::remove(Clients.begin(), Clients.end(), disconnectCli),
                Clients.end()
            );
        };
        
    };

    WSACleanup();

    return 0;
};