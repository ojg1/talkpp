#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <fstream>

void printLog(std::string StringToPrint) {

    std::cout << "\x1b[0;38;5;1;49m[buildtpp]\x1b[0m " << StringToPrint << "\x1b[0m\n";

    return;
};

void writeNewBuildc(std::string Fn, std::vector<std::string> FileLines, int beCount, int feCount) {

    std::ofstream File;
    File.open(Fn);
  
    FileLines[0] = std::to_string(beCount);
    FileLines[1] = std::to_string(feCount); 

    for (const auto& ln : FileLines) {
        File << ln << std::endl;
    };

    return;
};

int main(int argc, char* argv[]) {

    bool WriteWhenError = false;

    printLog("IGNORE: testlog");

    if ((std::string)argv[1] == "frontend" || (std::string)argv[1] == "backend") {

        std::string buildchoice = argv[1];
        std::string buildDirectory = "D:/Projects/TalkProject/Talk++dev/";
        std::string gcclib = "C:/gcclib";
        std::string line;
        std::vector<std::string> FileLines = {};
        std::ifstream BuildCount;
        BuildCount.open(".build_count");

        printLog("build choice: \x1b[1;38;5;13;49m" + buildchoice);
        printLog("fetching build count content");

        while (std::getline(BuildCount,line)) {
            FileLines.push_back(line);
        };

        int backendBuildCount = std::stoi(FileLines[0]);
        int frontendBuildCount = std::stoi(FileLines[1]);

        if (buildchoice == "frontend") {
            frontendBuildCount++;

            std::string TotalStringCommand = "g++ -std=c++23 -Wall -O2 "
            +buildDirectory+"frontend/cli.cpp "

            //FLTK Include And Libraries
            +" -IC:\\gcclibs\\fltk\\include" 
            +" -LC:\\gcclibs\\fltk\\lib" 
            // FLTK and Windows Linkers
            +" -lfltk"
            +" -lfltk_images"
            +" -lfltk_png"
            +" -lfltk_z"
            +" -lcomctl32" 
            +" -lgdi32"
            +" -lgdiplus"
            +" -lcomdlg32"
            +" -luser32"
            +" -lole32"
            +" -luuid"
            +" -lwinspool"

            +" -lws2_32" //WinSock2
            +" -o D:\\Projects\\TalkProject\\talk++dev\\frontend\\Talk++fe";

            printLog("--------------------------------------------------");
            printLog("\x1b[1;38;5;202;49mFrontend Build "+std::to_string(frontendBuildCount));
            printLog("running \x1b[1;38;5;46;49m" + TotalStringCommand);
            int status = std::system(TotalStringCommand.c_str());

            if (status != 0) {
                printLog("\x1b[1;38;5;196;49mfatal\x1b[0m: build failed, compiling returned errors");
                if (WriteWhenError) {
                    writeNewBuildc(".build_count", FileLines, backendBuildCount, frontendBuildCount);
                };
                return 4;
            } else {
                std::system("D:\\Projects\\TalkProject\\talk++dev\\frontend\\Talk++fe.exe");
                printLog("\x1b[1;38;5;46;49mbuild success");
                writeNewBuildc(".build_count", FileLines, backendBuildCount, frontendBuildCount);
                return 0;
            }; 

        } else if (buildchoice == "backend") {
            backendBuildCount++;

            std::string TotalStringCommand = "g++ -std=c++23 -Wall -O2 "
            +buildDirectory+"backend/server.cpp "
            +buildDirectory+"backend/serverNetwork.cpp "
            +" -lws2_32"
            +" -o Talk++be";
            
            
            printLog("--------------------------------------------------");
            printLog("\x1b[1;38;5;202;49mBackend Build "+std::to_string(backendBuildCount));
            printLog("running \x1b[1;38;5;46;49m" + TotalStringCommand);
            int status = std::system(TotalStringCommand.c_str());

            
            if (status != 0) {
                printLog("\x1b[1;38;5;196;49mfatal\x1b[0m: build failed, compiling returned errors");
                if (WriteWhenError) {
                    writeNewBuildc(".build_count", FileLines, backendBuildCount, frontendBuildCount);
                };
                return 4;
            } else {
                std::system(".\\Talk++be");
                printLog("\x1b[1;38;5;46;49mbuild success");
                writeNewBuildc(".build_count", FileLines, backendBuildCount, frontendBuildCount);
                return 0;
            }; 
        }   


    } else {
        printLog("\x1b[1;38;5;196;49mfatal\x1b[0m: Invalid first argument.");
        return 2;
    }

    return 0;
};