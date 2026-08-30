#include <iostream>
#include <string>
#include <vector>
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include "serverNetwork.hpp"
#include "quickutils.hpp"

/*
1st recv loop recv(*ClientSocket, TalkBuffer + occBytes, 2 - occBytes, 0);
2nd recv loop recv(*ClientSocket, TalkBuffer+occBytes, TalkFrame-occBytes, 0);
*/

std::string RecieveStep(const SOCKET *ClientSocket, char *Buffer, int Length, bool *ClientDisconnect, size_t *occBytes) {

    int RecievedBytes = recv(*ClientSocket, Buffer, Length, 0);
    int Error = (RecievedBytes == SOCKET_ERROR) ? WSAGetLastError() : 0;
    // std::cout << "RecievedBytes (recv()): " << RecievedBytes << "\n";
    std::cout << "";

    if (RecievedBytes == SOCKET_ERROR) {
        if (Error == WSAEWOULDBLOCK) {
            return "waiting";
        } else {
            std::cout << "Error code: " << Error << "\n";
            *ClientDisconnect = true;
            return "failure";   
        }
    } else {   
        if (RecievedBytes > 0) {
            *occBytes += RecievedBytes;
        } else if (RecievedBytes == 0) {
            std::cout << "Client-side activated disconnect\n";
            *ClientDisconnect = true;
            return "failure";
        };
    };

    return "success";

};

void printRawBytes(size_t *occBytes, char *TalkBuffer) {
    std::cout << "Raw frame bytes: ";
    for (int i = 0; i < *occBytes; i++) {
        std::cout << std::hex << static_cast<int>(static_cast<unsigned char>(TalkBuffer[i]))<< " ";
    }

    std::cout << std::dec << "\n";
};

std::string TalkServerNetwork::RecieveClientNetworkData(const SOCKET* ClientSocket, std::vector<SOCKET>* ClientsVector, SOCKET* disconnectClient) {

    /*
    Returns a full elligible string safely from a TCP client over a network. 
    */

    bool ClientDisconnect = false;
    char TalkBuffer[1024];
    std::string::size_type occBytes = 0;

    while (occBytes < 2) {
        
        std::string Result = RecieveStep(ClientSocket, TalkBuffer + occBytes, 2 - occBytes, &ClientDisconnect, &occBytes);
        
        if (Result == "success") {
            continue;
        };

        if (Result == "waiting") {
            return "";
        }

        if (Result == "failure") {
            break;
        };
    };

    if (ClientDisconnect) {
        std::cout << "Occupied Bytes: " << occBytes << "\n";
        std::cout << "Client was disconnected during framing. \n";
        *disconnectClient = *ClientSocket;
        return "Client Disconnected";
    };

    std::cout << "Message Framing was Recieved" << std::endl;
    uint16_t TalkFrame;
    memcpy(&TalkFrame, TalkBuffer, 2);
    
    TalkFrame = ntohs(TalkFrame);
    std::cout << "Frame: " << TalkFrame << "\n";

    while (occBytes < TalkFrame) {
        std::string Result = RecieveStep(ClientSocket, TalkBuffer+occBytes, TalkFrame-occBytes, &ClientDisconnect, &occBytes);
        
        if (Result == "success") {
            continue;
        };

        if (Result == "waiting") {
            return "";
        }
    };

    if (ClientDisconnect) {
        std::cout << "Client was disconnected while waiting for entire buffer." << std::endl; 
        *disconnectClient = *ClientSocket;
        return "Client Disconnected";
    };


    std::string Message(TalkBuffer+2, occBytes-2);
    std::string Final(TalkBuffer, occBytes);

    std::cout << "Socket second check complete" << std::endl;

    return Final;
};


std::string TalkServerNetwork::SendClientNetworkData(const SOCKET* ClientSocket, std::string StringToSend) {

    std::string Final;

    const char* TextMessageBuffer = StringToSend.c_str();
    int occBytes = 0;
    
    while (occBytes < StringToSend.length()) {
        int SendBytes = send(*ClientSocket, TextMessageBuffer+occBytes, StringToSend.length() - occBytes, 0);
        occBytes += SendBytes;
   };

    return Final;
};