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
    DWORD gnofieRead;

    DWORD mode;
    GetConsoleMode(hOut, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, mode);

    std::unordered_map<std::string, std::string> msg = {
        {"Test", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"}
    };

    std::cout << "Starting..." << std::endl;

    std::string connPoint = "hb930.duckdns.org:930";
    auto dura = std::chrono::seconds(30);
    std::string kinp = "";
    int64_t frames = 0;
    auto programStart = std::chrono::steady_clock::now();
    bool run = true;

    //main loop
    while (run) {

        auto start = std::chrono::steady_clock::now();

        frames++;

        //Render start

        bool success = GetNumberOfConsoleInputEvents(hIn, &gnofieRead);

        if (success) {    
            if (gnofieRead > 0) {
                bool success2 = ReadConsoleInput(
                    hIn,
                    &record,
                    1,
                    &eventsRead
                );

                if (success2) {
                    if (record.Event.KeyEvent.bKeyDown) {
                        kinp += static_cast<char>(record.Event.KeyEvent.uChar.UnicodeChar);
                    };
                };
        
            }
        };


        std::cout << "\033[2J\033[3J\033[H";
        std::cout << "\033[?25l";
        renderGUI(msg, kinp);

        auto elapsedProgram = std::chrono::steady_clock::now() - programStart;

        std::cout << "Frames: " << frames << "\n";
        std::cout << "Frames per second: " << frames/std::chrono::duration<double>(elapsedProgram).count() << "\n";

        //Render end
        auto end = std::chrono::steady_clock::now();

        auto elapsed = end - start;
        auto target = std::chrono::duration<double, std::milli>(12);

        if (elapsed < target) {
            std::this_thread::sleep_for(target-elapsed);
        };

    };

    return 0;
};


//192.168.1.210