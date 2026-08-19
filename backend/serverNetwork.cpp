#include <iostream>
#include <string>
#include <vector>
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include "serverNetwork.hpp"

std::string TalkServerNetwork::RecieveClientNetworkData(SOCKET* ClientSocket, std::vector<SOCKET>* ClientsVector) {

    /*
    Returns a full elligible string safely from a TCP client over a network.
    */

    bool ClientDisconnect = false;
    char TalkBuffer[1024];
    int occBytes = 0;
    
    while (occBytes < 2) {

        int RecievedBytes = recv(*ClientSocket, TalkBuffer, 1024, 0);
        occBytes += RecievedBytes;

        if (RecievedBytes == 0) {
            ClientDisconnect = true;
            break;
        };
    };

    if (ClientDisconnect) {
        return "Client Disconnected, automatically deleting client";
    };

    uint16_t TalkFrame;
    memcpy(&TalkFrame, TalkBuffer, 2);

    while (occBytes < TalkFrame) {

        int RecievedBytes = recv(*ClientSocket, TalkBuffer+occBytes, 1024, 0);
        occBytes += RecievedBytes;

        if (RecievedBytes == 0) {
            ClientDisconnect = true;
            break;
        };
    };

    if (ClientDisconnect) {
        return "Client Disconnected, automatically deleting client";
    };

    std::string Message(TalkBuffer+2, occBytes-2);
    std::string Final(TalkBuffer, occBytes);

    return Final;
};


std::string TalkServerNetwork::SendClientNetworkData(SOCKET* ClientSocket, std::string StringToSend) {

    std::string Final;

    const char* TextMessageBuffer = StringToSend.c_str();
    int occBytes = 0;
    
    while (occBytes < StringToSend.length()) {
        int SendBytes = send(*ClientSocket, TextMessageBuffer+occBytes, StringToSend.length(), 0);
        occBytes += SendBytes;
   };

    return Final;
};