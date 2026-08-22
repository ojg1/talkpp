#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <string>

#pragma once

class TalkServerNetwork{
    public:
        std::string RecieveClientNetworkData(const SOCKET* ClientSocket,  std::vector<SOCKET>* ClientsVector, SOCKET* disconnectClient);
        std::string SendClientNetworkData(const SOCKET* ClientSocket, std::string StringToSend);

};