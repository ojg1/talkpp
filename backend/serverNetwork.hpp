#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <string>

#pragma once

class TalkServerNetwork{
    public:
        std::string RecieveClientNetworkData(SOCKET* ClientSocket,  std::vector<SOCKET>* ClientsVector);
        std::string SendClientNetworkData(SOCKET* ClientSocket, std::string StringToSend);

};