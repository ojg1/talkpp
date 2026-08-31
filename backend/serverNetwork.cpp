#include <iostream>
#include <string>
#include <vector>
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include "serverNetwork.hpp"
#include "quickutils.hpp"
#include <unordered_map>

/*
1st recv loop recv(*ClientSocket, TalkBuffer + occBytes, 2 - occBytes, 0);
2nd recv loop recv(*ClientSocket, TalkBuffer+occBytes, TalkFrame-occBytes, 0);
*/

void plogs(std::string ansi, std::string header, std::string message) {
    std::cout << ansi << "[" << header << "]\x1b[0m " << message << "\x1b[0m\n";
    return;
};

std::string RecieveStep(const SOCKET *ClientSocket, char *Buffer, int Length, int *Errno, size_t *occBytes) {


    int RecievedBytes = recv(*ClientSocket, Buffer, Length, 0);
    int Error = (RecievedBytes == SOCKET_ERROR) ? WSAGetLastError() : 0;
    // std::cout << "RecievedBytes (recv()): " << RecievedBytes << "\n";
    std::cout << "";

    if (RecievedBytes == SOCKET_ERROR) {
        if (Error == WSAEWOULDBLOCK) {
            return "waiting";
        } else {
            std::cout << "Error code: " << Error << "\n";
            *Errno = Error;
            return "failure";   
        }
    } else {   
        if (RecievedBytes > 0) {
            *occBytes += RecievedBytes;
        } else if (RecievedBytes == 0) {
            std::cout << "Client-side activated disconnect\n";
            *Errno = Error;
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

ReceiveResult TalkServerNetwork::RecieveClientNetworkData(const SOCKET* ClientSocket) {

    /*
    Returns a full elligible string safely from a TCP client over a network. 
    */
    char TalkBuffer[1024];
    std::string::size_type occBytes = 0;

    ReceiveResult resrecv = {"", 0};
    int errno;

    while (occBytes < 2) {
        
        std::string Result = RecieveStep(ClientSocket, TalkBuffer + occBytes, 2 - occBytes, &errno, &occBytes);
        
        if (Result == "success") {
            continue;
        };

        if (Result == "waiting") {
            return {"ServerWaiting", 0};
        }

        if (Result == "failure") {
            resrecv.status = 1;
            break;
        };
    };

    if (resrecv.status == 1) {
        plogs("\x1b[0;38;5;198;49m", "talkrecv", "Occupied Bytes" + occBytes);
        plogs("\x1b[0;38;5;198;49m", "talkrecv", "Client disconnected or encountered an error during payload framing" + occBytes);
        resrecv.recieveStr = std::to_string(errno);
        return resrecv;
    };

    uint16_t TalkFrame;
    memcpy(&TalkFrame, TalkBuffer, 2);
    
    TalkFrame = ntohs(TalkFrame);
    plogs("\x1b[0;38;5;198;49m", "talkrecv", "!Message Framing was Recieved. Frame: " + TalkFrame);

    while (occBytes < TalkFrame) {
        std::string Result = RecieveStep(ClientSocket, TalkBuffer+occBytes, TalkFrame-occBytes, &errno, &occBytes);
        
        if (Result == "success") {
            continue;
        };

        if (Result == "waiting") {
            return {"ServerWaiting", 0};
        }

        if (Result == "failure") {
            resrecv.status = 1;
            break;
        };
    };

    if (resrecv.status == 1) {
        plogs("\x1b[0;38;5;198;49m", "talkrecv", "Client was disconnected or encountered an error while waiting for entire buffer.");
        resrecv.recieveStr = std::to_string(errno);
        return resrecv;
    };

    std::string Message(TalkBuffer+2, occBytes-2);
    std::string Final(TalkBuffer, occBytes);

    plogs("\x1b[0;38;5;198;49m", "talkrecv", "Payload successfully retrieved");

    //one final check before return
    if (resrecv.recieveStr != "" && resrecv.status == 0) {    
        return resrecv;
    } else {
        return {"ServerUnknownError", 1};
    };

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

bool TalkServerNetwork::ServerClientHandShake(const SOCKET* AcceptedClientSocket, int supported, int maximumRoomNumber) {

    bool successconn;

    std::string fullSendStart = std::to_string(supported) + ";" + std::to_string(maximumRoomNumber);
    this->SendClientNetworkData(AcceptedClientSocket, fullSendStart);

    auto response = this->RecieveClientNetworkData(AcceptedClientSocket);

    
    return successconn;    
};

