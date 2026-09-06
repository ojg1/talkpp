//C++ Headers

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <chrono> 

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
#include <FL/Fl_Pack.H>
#include <FL/Fl_Output.H>
#include <FL/fl_draw.H>

using std::string;
using std::vector;
using std::unordered_map;

vector<std::thread> roomaddressthreads;

//util
void plog(string ansi, string header, string message){
    std::cout << ansi << "[" << header << "]\x1b[0m" << message << "\x1b[0m\n";
};

struct cbckd {
    Fl_Input* TextBox;
    SOCKET* ClientSocket;
};

struct Color3 {
    unsigned char r,g,b;
};

struct Message {
    string user;
    string content;
    Color3 usercolor;
};

struct TalkAddressInfo {
    string AddressLabel;
    string Port;
    string RoomName;
    int maxMembers;
    vector<string> Members;
    vector<Message> Messages;
};

//Functions
string RecieveData(SOCKET *Client) {
    std::string result;
    char chunk[1024] = {0};
    int occBytes = 0;

    while (true) {
        int recieveStatus = recv(*Client, chunk + occBytes, sizeof(chunk)-occBytes, 0);

        if (recieveStatus > 0) {
            occBytes += recieveStatus;
        } else if (recieveStatus == 0) {
            std::cout << "debugging 1";
            break;  
        }
        else { // SOCKET_ERROR
            int err = WSAGetLastError();
            std::cout << "debugging 2: socket error";
            if (err == WSAEWOULDBLOCK) {
                std::cout << "debugging 3: socket would love to block";
                break;
            } else {
                std::cout << "debugging 4: error";
                std::cout << "TalkClient recv: error occured\n";
                break;
            }
        }
    };
    string whole(chunk, occBytes);

    if (whole.length() > 0) {
        result = whole;

        if (result.starts_with("rooms;")) {
            //  rooms datatatatatatatat
        }
    } else {

        result = "none";
    };

    return result;
};

string SendData(SOCKET* ClientSocket, string message) {
    if (!ClientSocket || *ClientSocket == INVALID_SOCKET) {
        return "failure";
    }

    int totalLength = static_cast<int>(message.length()) + 2;
    std::vector<char> messageBuffer(totalLength);

    uint16_t length = htons(static_cast<uint16_t>(totalLength));
    memcpy(messageBuffer.data(), &length, 2);
    
    memcpy(messageBuffer.data() + 2, message.data(), message.length());

    int occBytes = 0;
    bool err = false;

    while (occBytes < totalLength) {
        int bytesSent = send(
            *ClientSocket,
            messageBuffer.data() + occBytes,
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

    return err ? "failure" : "success";
}

class UserMessageWidget : public Fl_Widget {
    public:
        
        Message message;

        UserMessageWidget(int x, int y, int w, int h, Message msg) 
            : Fl_Widget(x,y,w,h), message(msg)
        {
            box(FL_NO_BOX);
        };

        void draw() override {

            fl_color(message.usercolor.r, message.usercolor.g, message.usercolor.b);
            fl_draw(message.user.c_str(), x(), y());
            
            fl_color(FL_BLACK);
            fl_draw(message.content.c_str(), x()+fl_width(message.user.c_str()), y());

        };
};


void SendDataCallback(Fl_Widget* widget, void* data) {
    cbckd* callback = static_cast<cbckd*>(data);
    const char* val = callback->TextBox->value();
    std::string textToSend = (val != nullptr) ? val : "";

    if (!textToSend.empty()) {
        SendData(callback->ClientSocket, textToSend);
        callback->TextBox->value("");
    }
};

void CreateNewRoomCallback(Fl_Widget* widget, void* data) {
    if (!data) return;
    cbckd* callback = static_cast<cbckd*>(data);
    if (callback && callback->ClientSocket) {
        SendData(callback->ClientSocket, "NewRoom");
    }
};

void AddNewMessage(const std::string& message, Fl_Pack* MessagePack) {

    int widgetWidth = MessagePack->w();
    int widgetHeight = 30;

    UserMessageWidget* msgWidget = new UserMessageWidget(
        0,
        0,
        widgetWidth,
        widgetHeight,
        Message{
            "baller",
            message,
            Color3{255, 255, 25}
        }
    );

    msgWidget->draw();

    MessagePack->add(msgWidget);

    MessagePack->init_sizes();

    MessagePack->redraw();

    if (MessagePack->parent()) {
        MessagePack->parent()->redraw();
    };
};
/*
reference

struct Message {
    string user;
    string content;
    Color3 usercolor;
};
class UserMessageWidget : public Fl_Widget {
    public:
        
        Message message;

        UserMessageWidget(int x, int y, int w, int h, Message msg) 
            : Fl_Widget(x,y,w,h), message(msg)
        {
            box(FL_NO_BOX);
        };

        void draw() override {

            fl_color(message.usercolor.r, message.usercolor.g, message.usercolor.b);
            fl_draw(message.user.c_str(), x(), y());
            
            fl_color(FL_BLACK);
            fl_draw(message.content.c_str(), x()+fl_width(message.user.c_str()), y());

        };
};

*/

int networkThread(Fl_Return_Button* SendButton, Fl_Input* TextBox, Fl_Pack* MessagePack, string ADDRESS, string PORT){
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

    int gaiResult;

    if (ADDRESS != "{default}") {
        gaiResult = getaddrinfo(ADDRESS.c_str(), PORT.c_str(), &talkaddr, &result);
    } else {
        gaiResult = getaddrinfo(DEFAULT_ADDR, DEFAULT_PORT, &talkaddr, &result);
    }
    if (gaiResult != 0) {
        std::cout << "TalkClient: getaddrinfo failed; " << gaiResult << "\n"; 
        std::cout << "TalkClient: maybe check if you have the correct address or port?\n";
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
        std::string recvResult = RecieveData(&TalkSocket);
        AddNewMessage(recvResult, MessagePack);
    };

    WSACleanup();

    return 0;
};

class RoomDisplayWidget : public Fl_Widget {
    public:

        TalkAddressInfo addressinfo;

        RoomDisplayWidget(int x, int y, int w, int h, TalkAddressInfo addressinfo)
            : Fl_Widget(x,y,w,h), addressinfo(addressinfo)
        {
            box(FL_PLASTIC_DOWN_BOX);
        };

        void draw() override {
            //Show Room Label
            fl_color(FL_BLACK);
            fl_draw(addressinfo.RoomName.c_str(), x(), y());

            //Show MembersW
            fl_color(FL_GRAY);
            string meminfo = std::to_string(addressinfo.Members.size()) + "/" + std::to_string(addressinfo.maxMembers);
            fl_draw(meminfo.c_str(),x()+w()-fl_width(meminfo.c_str())-5, y()+h()-fl_height()-5);
            
            //Show Address and Port
            fl_color(FL_GRAY);
            string addrinfotext = addressinfo.AddressLabel + ":" + addressinfo.Port;
            fl_draw(addrinfotext.c_str(), x()+5, y()+h()-fl_height()-5);
        };


};

void windowCloseCallback(Fl_Widget* Widget, void* data) {
    Widget->hide();
    for (auto& connection : roomaddressthreads) {
        if (connection.joinable()) {
            connection.join();
        };
    };
};

int main(int argc, char** argv) {
//   string user;
//   string content;
//   Color3 usercolor;
//;

    vector<TalkAddressInfo> RoomAddresses = { //example data
       TalkAddressInfo {
        "hb930.duckdns.org",
        "930",
        "cool example room",
        5,
        {"bob", "baller", "hb"},
        {
            {
                "bob",
                "im bob and im trying to reach that cube shaped thing",
                {80, 150, 255}
            },
            {
                "hb",
                "hahhahah i made you endure this torture",
                {80, 150, 255}
            },
            {
                "baller",
                "im just a spectator if yall get in trouble this aint my fault ",
                {80, 150, 255}
            }
        }

       }
    };

    //RoomAddresses contain a TalkAddressInfo
    //Start a new thread for each TalkAddressInfo
    //and if one socket disconnects, make sure the whole client doesnt screw up because one died of many

    Fl_Window *TalkFLTKWindow = new Fl_Window(800,500);

    TalkFLTKWindow->callback(windowCloseCallback);

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
        AddNew->callback(CreateNewRoomCallback);
    Rooms->end();

    MainChat->begin();
        Fl_Input* TextBox = new Fl_Input(MainChat->x()+10,MainChat->y()+415, 370, 25);
        TextBox->placeholder("Type your message here...");
        TextBox->box(FL_PLASTIC_UP_BOX);

        Fl_Return_Button* TextBoxSend = new Fl_Return_Button(MainChat->x()+390, MainChat->y()+415, 100, 25, "Send");
        TextBoxSend->box(FL_PLASTIC_UP_BOX);

        Fl_Scroll* ChatScroll = new Fl_Scroll(MainChat->x()+10,MainChat->y()+10,480,400);
        ChatScroll->box(FL_PLASTIC_UP_BOX); 
        ChatScroll->type(Fl_Scroll::VERTICAL);
        ChatScroll->begin();
            Fl_Pack* TextBoxText = new Fl_Pack(MainChat->x()+10,MainChat->y()+10, 480, 400);
            TextBoxText->type(Fl_Pack::VERTICAL);
            TextBoxText->spacing(5); 
            TextBoxText->begin();
                
                

            TextBoxText->end();
        ChatScroll->end();
    MainChat->end();    

    Fl_Box *notifier = new Fl_Box(0, 0, 60, 20,"Talk++");
    notifier->box(FL_FLAT_BOX);
    notifier->color(FL_GREEN);
    notifier->labelsize(18);    

    Fl_PNG_Image* icon = new Fl_PNG_Image("assets/talk.png");
    TalkFLTKWindow->icon(icon);

    TalkFLTKWindow->end();
    TalkFLTKWindow->show(argc, argv);

    AddNewMessage("balls", TextBoxText);

    //start looping thread

    for (const auto& add : RoomAddresses) {
        std::thread addrthreadgen(networkThread, TextBoxSend, TextBox, TextBoxText, add.AddressLabel, add.Port);
        roomaddressthreads.push_back(std::move(addrthreadgen));
    };

    Fl::run();

    return 0;
}