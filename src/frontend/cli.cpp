//------------C++ Headers------------

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <chrono> 

//------------Windows Headers with Networking------------
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#define DEFAULT_ADDR "hb930.duckdns.org"
#define DEFAULT_PORT "930"

//------------Threading------------
#include <thread>
#include <mutex>

//------------FLTK------------
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

//------------using------------
using std::string;
using std::vector;
using std::unordered_map;

vector<std::thread> roomaddressthreads;

//------------Utilities------------
void plog(string ansi, string header, string message){
    std::cout << ansi << "[" << header << "]\x1b[0m" << message << "\x1b[0m\n";
};

//------------Struct------------
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

struct AwakeData {
    Message ucont;
    Fl_Pack* pack;
};

//------------Network Functions------------
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

//------------Custom Widgets------------

//helper func:

struct WidthLns {
    int lns;
    std::vector<std::string> content;
};
std::vector<std::string> GetWidthOffLines(std::string ln, int w) {
    int cwidth = 0;
    int lines = 1;

    std::vector<std::string> cont = {};
    std::vector<char> chars(ln.begin(), ln.end());

    int start = 0;
    int end = 0;

    for (int i = 0; i < chars.size(); i++) {
        cwidth += fl_width(chars[i]);
        if (cwidth >= w) {
            end = i - 1;
            cont.push_back(ln.substr(start, end - start));
            start = i;
            cwidth = 0;
        };
    };

    cont.push_back(ln.substr(start, chars.size() - start)); 
    return cont; 
};

class UserMessageWidget : public Fl_Widget {
    public:
        
        Message message;
        std::vector<std::string> calcHeight;
        bool wrapped = false;

        UserMessageWidget(int x, int y, int w, Message msg) 
            : Fl_Widget(x,y,w, 16), message(msg)
        {
            box(FL_NO_BOX);
        };

        void draw() override {

            fl_font(FL_HELVETICA, 16);

            if (!wrapped) {
                calcHeight = GetWidthOffLines(message.content, w());
                int newH = static_cast<int>(calcHeight.size()) * 16;
                if (newH != h()) {
                    size(w(), newH);
                    if (parent()) parent()->init_sizes(); 
                }
                wrapped = true;
            }

            int baseline = y() + h(); // leave a little padding from the bottom
            //draw username
            fl_font(FL_HELVETICA_BOLD, 16);
            fl_color(message.usercolor.r, message.usercolor.g, message.usercolor.b);
            fl_draw(message.user.c_str(), x() + 2, baseline);

            //draw content
            fl_font(FL_HELVETICA, 16);
            fl_color(FL_BLACK);

            for (int i = 0; i < calcHeight.size(); i++) {
                if (i == 0) {
                    fl_draw(message.content.c_str(), x() + 4 + fl_width(message.user.c_str()) + 5, baseline);
                } else {
                    fl_draw(message.content.c_str(), x() + 4, baseline*16);
                }
            }
            
        };
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

//------------Other Functions------------
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

void AddNewMessage(const Message ucont, Fl_Pack* MessagePack) {
    std::cout << "AddNewMessage: start\n" << std::flush;

    int widgetWidth = MessagePack->w();
    std::cout << "AddNewMessage: got width " << widgetWidth << "\n" << std::flush;

    UserMessageWidget* msgWidget = new UserMessageWidget(
        0, 0, widgetWidth, ucont
    );
    std::cout << "AddNewMessage: widget constructed\n" << std::flush;

    if (msgWidget->w() > MessagePack->w()) {
        std::cout << "well thats a problem for another time!";
    };

    MessagePack->add(msgWidget);
    std::cout << "AddNewMessage: added to pack\n" << std::flush;

    MessagePack->init_sizes();
    std::cout << "AddNewMessage: init_sizes done\n" << std::flush;

    MessagePack->redraw();
    std::cout << "AddNewMessage: redraw called\n" << std::flush;

    if (MessagePack->parent()) {
        MessagePack->parent()->redraw();
    };
    std::cout << "AddNewMessage: done\n" << std::flush;
};


void HandleIncomingMessage(void* data) {
    AwakeData* awakeData = static_cast<AwakeData*>(data);

    AddNewMessage(awakeData->ucont, awakeData->pack);

    delete awakeData; // we made this with 'new', so we clean it up here
}

void QueueMessage(Message message, Fl_Pack* pack) {
    AwakeData* data = new AwakeData;
    data->ucont = message;
    data->pack = pack;

    Fl::awake(HandleIncomingMessage, data);
}; 

int networkThread(Fl_Return_Button* SendButton, Fl_Input* TextBox, Fl_Pack* MessagePack, string ADDRESS, string PORT){
    std::cout << "netthread started \n" << std::flush;

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

        // temp data
        Message preucont = {};
        preucont.content = recvResult;
        preucont.user = "Anonymous";
        preucont.usercolor = {0,0,0};

        // AddNewMessage(preucont, MessagePack);
        // AwakeData* dataToSend = new AwakeData;
        // dataToSend->ucont = preucont;
        // dataToSend->pack = MessagePack;

        // Fl::awake(HandleIncomingMessage, dataToSend);
        QueueMessage(preucont, MessagePack);
    };

    WSACleanup();

    return 0;
};

void windowCloseCallback(Fl_Widget* Widget, void* data) {
    Widget->hide();
    for (auto& connection : roomaddressthreads) {
        if (connection.joinable()) {
            connection.join();
        };
    };
};

//------------my fun testing------------

void SendAll(Fl_Pack* testpack, std::string line) {
    Color3 j = {255,0,0};
    Color3 z = {0,0,255};
    Color3 r = {255,255,0};

    QueueMessage({"Joe", line, j}, testpack);
    QueueMessage({"Zubin", line, z}, testpack);
    QueueMessage({"Rob", line, r}, testpack);

};

void Ruler(Fl_Pack* testpack) {
    Color3 j = {255,0,0};

    QueueMessage({"Joe", "JUNO WAS MAD, HE KNEW HE'D BEEN HAD", j}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    QueueMessage({"Joe", "SO HE SHOT AT THE SUN WITH A GUN", j}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    QueueMessage({"Joe", "SHOT AT THE SUN WITH A GUN", j}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    QueueMessage({"Joe", "SHOT AT HIS WILY ONE, ONLY FRIEND", j}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    QueueMessage({"Joe", "YOU UNDERSTAND MECHANICAL HANDS", j}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    QueueMessage({"Joe", "ARE THE RULER OF EVERYTHING", j}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    QueueMessage({"Joe", "RULER OF EVERYTHING", j}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    QueueMessage({"Joe", "I'M THE RULER OF EVERYTHING IN THE END", j}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    Color3 z = {0,0,255};

    QueueMessage({"Zubin", "DO YOU LIKE HOW I DANCE? I'VE GOT ZIRCONIUM PANTS", z}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    QueueMessage({"Joe", "CONSEQUENTIAL ENOUGH TO SLIP YOU INTO A TRANCE", j}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    QueueMessage({"Zubin", "DO YOU LIKE HOW I WALK? DO YOU LIKE HOW I TALK?", z}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    QueueMessage({"Joe", "DO YOU LIKE HOW MY FACE DISINTEGRATES INTO CHALK?", j}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    QueueMessage({"Zubin", "I HAVE A WONDERFUL WIFE, I HAVE A POWERFUL JOB", z}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    QueueMessage({"Zubin", "SHE CRITICIZES ME FOR BEING EGOCENTRIC", z}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    QueueMessage({"Joe", "YOU PRACTICE YOUR MANNERISMS INTO THE WALL", j}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    QueueMessage({"Zubin", "IF THIS MIRROR WERE CLEAR, I'D BE STANDING SO TALL", z}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    Color3 r = {255,255,0};

    QueueMessage({"Zubin", "I WAS OBSERVING THE BIRDS", z}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    QueueMessage({"Joe", "CIRCLE IN FOR THE KILL", j}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    QueueMessage({"Rob", "CIRCLE IN FOR THE KILL", r}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    QueueMessage({"Joe", "I'VE BEEN YOU, I KNOW YOU, YOUR FACADE IS A SCAM", j}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    QueueMessage({"Zubin", "YOU KNOW YOU'RE MAKING ME CRY, THIS IS THE WAY THAT I AM", z}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    QueueMessage({"Zubin", "I'VE BEEN LIVING A LIE, A METAMORPHICAL SCHEME", z}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    QueueMessage({"Joe", "DETECTIVE UNDERCOVER, BROTHERHOOD, OBJECTIVE, OBSCENE", j}, testpack);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::vector<std::string> mainmono = {
        "DO YOU HEAR THE FLIBBITY JIBBITY JIBBER JABBER",
        "WITH AN, \"OH MY GOD, I'VE GOT TO GET OUT OF HERE OR I'LL HAVE ANOTHER",
        "WORD TO SELL, ANOTHER STORY TO TELL",
        "ANOTHER TIME PIECE RINGING THE BELL\"",
        "DO YOU HEAR THE CLOCK STOP WHEN YOU REACH THE END?",
        "NO, YOU KNOW IT MUST BE NEVER ENDING, COMPREHEND IF YOU CAN",
        "BUT WHEN YOU TRY TO PRETEND TO UNDERSTAND",
        "YOU RESEMBLE A FOOL, ALTHOUGH YOU'RE ONLY A MAN",
        "SO GIVE IT UP AND SMILE"
    };

    for (const auto& ln : mainmono) {
        SendAll(testpack, ln);
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    };

};

//main
int main(int argc, char** argv) {
    //   string user;
    //   string content;
    //   Color3 usercolor;
    //;

    Fl::lock();


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
            TextBoxText->box(FL_PLASTIC_UP_BOX); 
            TextBoxText->spacing(5); 
        ChatScroll->end();
    MainChat->end();    

    Fl_Box *notifier = new Fl_Box(0, 0, 60, 20,"Talk++");
    notifier->box(FL_FLAT_BOX);
    notifier->color(FL_GREEN);
    notifier->labelsize(18);    

    // Fl_PNG_Image* icon = new Fl_PNG_Image("assets/talk.png");
    // TalkFLTKWindow->icon(icon);
    

    TalkFLTKWindow->end();
    TalkFLTKWindow->show(argc, argv);

    Message PersonA = {};
    PersonA.user = "A";
    PersonA.content = "test";
    PersonA.usercolor = {0,200,0};
    
    Message PersonB = {};
    PersonB.user = "B";
    PersonB.content = "test";
    PersonB.usercolor = {0,130,0};


    QueueMessage(PersonA, TextBoxText);
    QueueMessage(PersonB, TextBoxText);
    
    std::thread testrender(Ruler, TextBoxText);

    //start looping thread

    for (const auto& add : RoomAddresses) {
        std::thread addrthreadgen(networkThread, TextBoxSend, TextBox, TextBoxText, add.AddressLabel, add.Port);
        roomaddressthreads.push_back(std::move(addrthreadgen));
    };

    std::cout<<"test\n";

    Fl::run();

    return 0;
}
