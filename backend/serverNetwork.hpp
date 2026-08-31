#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <string>
#include <unordered_map>

#pragma once

struct ReceiveResult {
    std::string recieveStr;
    int status;
};

struct Room {
    std::string roomLabel;
    int peopleOnline;
};

//0 = waiting
//1 = clientdisconnect/error

//recieveStr is "" when its clientdisconnect (10061)
//recieveStr is std::to_string(ErrorCode) otherwise

class TalkServerNetwork{
    public:
        ReceiveResult RecieveClientNetworkData(const SOCKET* ClientSocket);
        std::string SendClientNetworkData(const SOCKET* ClientSocket, std::string StringToSend);
        bool ServerClientHandShake(const SOCKET* AcceptedClientSocket, int supported, int maximumRoomNumber);
};