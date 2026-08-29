//C++ Headers
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

//Windows Headers with Networking
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#define DEFAULT_ADDR "hb930.duckdns.org"
#define DEFAULT_PORT "930"

//Threading
#include <thread>
#include <mutex>

//FLTK
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Return_Button.H>

using std::string;
using std::vector;
using std::unordered_map;

//util
void plog(string ansi, string header, string message){
    std::cout << ansi << "[" << header << "]\x1b[0m" << message << "\x1b[0m\n";
};

struct cbckd {
    Fl_Input* TextBox;
    SOCKET* ClientSocket;
};
//plog("\x1b[0;38;5;10;49m", "stuff", "stuff did something");
//output:
//[stuff] stuff did something

//Functions
string RecieveData(SOCKET *Client) {
    std::string result;
    char chunk[1024];
    int occBytes = 0;

    while (true) {
        int recieveStatus = recv(*Client, chunk + occBytes, sizeof(chunk)-occBytes, 0);

        if (recieveStatus > 0) {
            occBytes += recieveStatus;
        } else if (recieveStatus == 0) {
            break;
        }
        else { // SOCKET_ERROR
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) {
                break;
            } else {
                std::cout << "TalkClient recv: error occured\n";
                break;
            }
        }
    };

    string whole(chunk, occBytes);

    if (whole.length() > 0) {
        result = whole;
    } else {
        result = "none";
    };

    return result;
};

//https://learn.microsoft.com/en-us/windows/win32/winsock/sending-and-receiving-data-on-the-client
//PLANNED: use correct framing logic so server can parse it
string SendData(SOCKET* ClientSocket, string message) {

    int totalLength = message.length() + 2;

    char messageBuffer[totalLength];

    uint16_t length = htons(totalLength);
    memcpy(messageBuffer, &length, 2);

    memcpy(messageBuffer + 2, message.data(), message.length());

    int occBytes = 0;
    bool err = false;

    while (occBytes < totalLength) {

        int bytesSent = send(
            *ClientSocket,
            messageBuffer + occBytes,
            totalLength - occBytes,
            0
        );

        if (bytesSent == SOCKET_ERROR) {
            int error = WSAGetLastError();
            std::cout << "An error occurred: " << error << "\n";
            err = true;
            break;
        }

        occBytes += bytesSent;
    }

    if (err) {
        return "failure";
    } else {
        return "success";
    }
}

void SendDataCallback(Fl_Widget* widget, void* data) {
    cbckd* callback = static_cast<cbckd*>(data);
    SendData(callback->ClientSocket, callback->TextBox->value());
};

int networkThread(Fl_Return_Button* SendButton, Fl_Input* TextBox){
    std::cout << "netthread started\n" << std::flush;

    WSADATA wsadata;
    int StartupStatus = WSAStartup(MAKEWORD(2,2), &wsadata);

    if (StartupStatus != 0) {
        std::cout << "TalkClient: startup failed; " << StartupStatus << "";
        return -1;
    };

    SOCKET TalkSocket = INVALID_SOCKET;

    //type of connection
    struct addrinfo talkaddr, *result, *ptr = NULL;
    ZeroMemory(&talkaddr, sizeof(talkaddr));

    talkaddr.ai_family = AF_INET;
    talkaddr.ai_socktype = SOCK_STREAM;
    talkaddr.ai_protocol = IPPROTO_TCP;

    //get address info of domain:port
    int gaiResult = getaddrinfo(DEFAULT_ADDR, DEFAULT_PORT, &talkaddr, &result);

    if (gaiResult != 0) {
        std::cout << "TalkClient: getaddrinfo failed; " << gaiResult << "\n"; 
        return -1;
    };

    ptr=result;

    TalkSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

    int conn = connect(TalkSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    int err = WSAGetLastError();
    if (conn == SOCKET_ERROR) {
        std::cout << "Connection to the server failed\n";
        std::cout << "Error code: " << std::to_string(err) << "\n";
        closesocket(TalkSocket);
        TalkSocket = INVALID_SOCKET;
    };

    freeaddrinfo(result);

    if (TalkSocket == INVALID_SOCKET) {
        std::cout << "TalkClient: cannot connect to server\n";
        WSACleanup();
        return -1;
    } else {
        std::cout << "TalkClient: socket successfully connected\n";
    };

    cbckd* cl = new cbckd{
        TextBox,
        &TalkSocket
    };

    SendButton->callback(SendDataCallback, cl);

    while (true) {
        RecieveData(&TalkSocket);
    };

    WSACleanup();

    return 0;
};

int main(int argc, char** argv) {

    Fl_Window *TalkFLTKWindow = new Fl_Window(800,500);

    Fl_Group* Rooms = new Fl_Group(10,40,150,450, "Enlisted Rooms");
    Rooms->end();
    Fl_Group* MainChat = new Fl_Group(170,40,500,450, "Chat");
    MainChat->end();
    Fl_Group* People =  new Fl_Group(680,40,110,450, "Members");
    People->end();

    Rooms->box(FL_PLASTIC_DOWN_BOX);
    MainChat->box(FL_PLASTIC_DOWN_BOX);
    People->box(FL_PLASTIC_DOWN_BOX);

    Rooms->begin();
        Fl_Button* AddNew = new Fl_Button(Rooms->x()+10,Rooms->y()+415, 130, 25, "+ Add Room");
        AddNew->box(FL_PLASTIC_UP_BOX);
    Rooms->end();

    MainChat->begin();
        Fl_Input* TextBox = new Fl_Input(MainChat->x()+10,MainChat->y()+415, 370, 25);
        TextBox->placeholder("Type your message here...");
        TextBox->box(FL_PLASTIC_UP_BOX);
        

        Fl_Return_Button* TextBoxSend = new Fl_Return_Button(MainChat->x()+390, MainChat->y()+415, 100, 25, "Send");
        TextBoxSend->box(FL_PLASTIC_UP_BOX);

        Fl_Scroll* TextBoxText = new Fl_Scroll(MainChat->x()+10,MainChat->y()+10, 480, 400);
        TextBoxText->box(FL_PLASTIC_UP_BOX);
    MainChat->end();    

    Fl_Box *notifier = new Fl_Box(0, 0, 60, 20,"Talk++");
    notifier->box(FL_FLAT_BOX);
    notifier->color(FL_GREEN);
    notifier->labelsize(18);

    Fl_PNG_Image* icon = new Fl_PNG_Image("assets/talk.png");
    TalkFLTKWindow->icon(icon);

    TalkFLTKWindow->end();
    TalkFLTKWindow->show(argc, argv);

    std::thread talknet(networkThread, TextBoxSend, TextBox);
    talknet.detach();

    Fl::run();

    return 0;
}