#include <iostream>
#include <windows.h>
#include <unordered_map>
#include <vector>
#include <cstdio>
#include <string>
#include <chrono>
#include <thread>

void renderGUI(std::unordered_map<std::string,std::string> messages, std::string kinput){

    std::string barline(100, '=');
    std::string emptytxln(98, ' ');
    std::cout << barline << "\n";

    int lineCount = 0;

    for (const auto& [user, message] : messages) {
        lineCount++;
        
        std::string vslen = "   <" + user + "> " + message; 
        std::string userFormatString = "= <\033[33m" + user + "\033[0m>";
        std::string obString = userFormatString + " " + message;

        size_t obstr = 99-vslen.length();

        std::string spcBuffer(obstr,' ');
        
        std::string oString =  obString + spcBuffer + " =\n";

        std::cout << oString;
    };

    for (int j = 0 ; j < 15-lineCount ; j++){
        std::cout << "=" << emptytxln <<"=\n";
    };

    std::cout << barline << "\n";
    std::cout << "|>" << kinput << "\n";

    size_t klen = kinput.length();
    if (klen > 75) {
        std::cout << "\033[41m!!! Message exceeded 75 char limit !!!\033[0m\n";
    };

    return;
};

int main(){

    AllocConsole();

    FILE* NewConsole;
    freopen_s(&NewConsole, "CONOUT$", "w", stdout);
    freopen_s(&NewConsole, "CONOUT$", "w", stderr);
    freopen_s(&NewConsole, "CONIN$", "r", stdin);

    HANDLE hOut = CreateFileW(
        L"CONOUT$",
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD record;
    DWORD eventsRead;

    DWORD mode;
    GetConsoleMode(hOut, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, mode);

    std::unordered_map<std::string, std::string> msg = {
        {"Test", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"}
    };

    std::cout << "Starting..." << std::endl;

    auto dura = std::chrono::seconds(30);

    bool run = true;
    
    std::string kinp = "";

    //main loop
    while (run) {

        auto start = std::chrono::steady_clock::now();

        //Render start
        
        bool success = ReadConsoleInput(
            hIn,
            &record,
            1,
            &eventsRead
        );

        if (success) {
            if (record.Event.KeyEvent.bKeyDown) {
                kinp += static_cast<char>(record.Event.KeyEvent.uChar.UnicodeChar);
            };
        };

        std::cout << "\033[2J\033[3J\033[H";
        std::cout << "\033[?25l";
        renderGUI(msg, kinp);

        //Render end
        auto end = std::chrono::steady_clock::now();

        auto elapsed = end - start;
        auto target = std::chrono::duration<double, std::milli>(16.666);

        if (elapsed < target) {
            std::this_thread::sleep_for(target-elapsed);
        };

    };

    return 0;
};