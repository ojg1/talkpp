
// your greatly appreciated 
//  mmmmmm                        mm                                  mm 
//  ##""""##                      ##                                  ## 
//  ##    ##   m#####m   m#####m  ## m##"    m####m   ##m####m   m###m## 
//  #######    " mmm##  ##"    "  ##m##     ##mmmm##  ##"   ##  ##"  "## 
//  ##    ##  m##"""##  ##        ##"##m    ##""""""  ##    ##  ##    ## 
//  ##mmmm##  ##mmm###  "##mmmm#  ##  "#m   "##mmmm#  ##    ##  "##mm### 
//  """""""    """" ""    """""   ""   """    """""   ""    ""    """ ""                                                       
// build script

// fueled by     
//        ___            _   
//       / __\___   __ _| |_ 
//      / /  / _ \ / _` | __|
//     / /__| (_) | (_| | |_ 
//     \____/\___/ \__,_|\__| basic 
//     (COmpileAuTO)
//
// sub-project auth: OrangeJuiceGuy1
// reason made: friend doesnt know how to code, also yes there will be a more complex version of coat this is just basic  

//!!!!YOU ONLY MODIFY CERTAIN VARIABLES IN int main() {...};!!!!



#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <format>
#include <string>
#include <vector>
#include <windows.h>

#define plogRED "\x1b[0;38;5;196;49m"
#define plogGREEN "\x1b[1;38;5;40;49m"
#define plogBLUE "\x1b[0;38;5;21;49m"
#define plogYELLOW "\x1b[0;38;5;11;49m"
#define plogORANGE "\x1b[0;38;5;208;49m"
#define plogPURPLE "\x1b[0;38;5;129;49m"
#define plogPINK "\x1b[0;38;5;201;49m"
#define plogCYAN "\x1b[0;38;5;51;49m"

#define plogBOLD "\x1b[1m"
#define plogITALIC "\x1b[3m"

std::vector<std::string> logvec = {};

void writetofilelogvec(std::string filedir) {
    std::ofstream file(filedir, std::ios::app);
    for (const auto& lv : logvec) {
        file << lv;
    }
    file.close();
};

void writetofilesingle(std::string filedir, std::string message) {
    std::ofstream file(filedir, std::ios::app);
    file << "\n" << message;
    file.close();
};

void plog(std::string ansi, std::string header, std::string message){
    auto now = std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now());
    auto local = std::chrono::zoned_time{"America/New_York", now};
    std::string time = std::format("{:%Y-%m-%d %H:%M:%S}", local);

    std::string finalplog = "[" + time + "]" + ansi + "[" + header + "]\x1b[0m " + message + "\x1b[0m\n";

    logvec.push_back(finalplog);

    std::cout << finalplog;
};

int SizeOfLongStrVec(std::vector<std::string> vec) {
    int large;
    for (const auto& str : vec) {
        if (str.length() > large) {
            large = str.length();
        };
    };

    return large;
};

int main() {

    SetConsoleOutputCP(CP_UTF8);

    plog((std::string)plogRED+(std::string)plogBOLD, "buildtpp", "Starting build...");

    //aslong as it works in the end then you can change and/or add variables
    //-------------------------THESE ARE THE THINGS YOU MIGHT NEED TO CHANGE!!!---------------------
    std::string BuildName = "Backend Build";
    std::string BuildDescription = "Talk++ build for backend";
    std::string BuildLogSuffix = "b";

    std::string BuildDirectory = "D:\\Projects\\TalkProject\\talk++dev";
    std::string mainCompile = "server.cpp";
    std::string outputName = "Talk++be.exe";
    std::string outputBuildLogFile = BuildDirectory + "\\build\\.build_logs";
    std::string outputBuildCountFile = BuildDirectory + "\\build\\.build_count";
    std::string outputExecutableDirectory = BuildDirectory + "\\build\\compiled\\";

    std::vector<std::string> ExtraNessecaryCppFiles = {
        "serverNetwork.cpp"
    };

    std::vector<std::string> IncludeDirectory = {
    
    }; //-I


    std::vector<std::string> LibraryDirectory = {
    
    }; //-L


    std::vector<std::string> SpecificLibrary = {
        "ws2_32"
    };

    bool IncludeHeaderAdded = false;
    bool SpecificLibraryHeaderAdded = false; //False if the strings in SpecificLibrary do not have a leading -l and same for the next bools
    bool GeneralLibraryHeaderAdded = false;

    bool BuildCountAppendage = true;

    //stop changing when you reach here, or if you have balls then do it

    plog((std::string)plogYELLOW+(std::string)plogBOLD, "info", "------------BUILD INFORMATION------------");
    plog((std::string)plogYELLOW+(std::string)plogBOLD, "info", "Build Name: " + BuildName);
    plog((std::string)plogYELLOW+(std::string)plogBOLD, "info", "Build Description: " + BuildDescription);
    plog((std::string)plogYELLOW+(std::string)plogBOLD, "info", "Build Suffix: " + BuildLogSuffix);
    plog((std::string)plogYELLOW+(std::string)plogBOLD, "info", "");
    plog((std::string)plogYELLOW+(std::string)plogBOLD, "info", "Build Directory: " + BuildDirectory);
    plog((std::string)plogYELLOW+(std::string)plogBOLD, "info", "Output Name: " + outputName);
    plog((std::string)plogYELLOW+(std::string)plogBOLD, "info", "Output Executable Directory: " + outputExecutableDirectory);
    plog((std::string)plogYELLOW+(std::string)plogBOLD, "info", "Output Build Logs Directory: " + outputBuildLogFile);
    plog((std::string)plogYELLOW+(std::string)plogBOLD, "info", "Output Build Count Directory: " + outputBuildCountFile);

    plog((std::string)plogYELLOW+(std::string)plogBOLD, "info", "");

    plog((std::string)plogYELLOW+(std::string)plogBOLD, "info", "Additional: ");
    // plog((std::string)plogYELLOW+(std::string)plogBOLD, "info", "In order: C++ Files, Includes, Library Directories, Specific Libraries");

    std::vector<std::string> lines = {};
    int appendOffset = 5;
    std::string repeat(appendOffset, ' ');

    std::vector<std::string> headers = {
        "C++ Files", "Include Directories", "Library Directories", "Specific Libraries"
    };

    auto maxLength = std::max({
        ExtraNessecaryCppFiles.size(),
        IncludeDirectory.size(),
        LibraryDirectory.size(),
        SpecificLibrary.size()
    });

    const std::vector<size_t> MaxColumnStringSize = {
        std::max((size_t)SizeOfLongStrVec(ExtraNessecaryCppFiles), headers[0].length()),
        std::max((size_t)SizeOfLongStrVec(IncludeDirectory), headers[1].length()),
        std::max((size_t)SizeOfLongStrVec(LibraryDirectory), headers[2].length()),
        std::max((size_t)SizeOfLongStrVec(SpecificLibrary), headers[3].length())
    };

    std::vector<std::string> headerColors = {
    (std::string)plogRED + plogBOLD,
    (std::string)plogGREEN + plogBOLD,
    (std::string)plogBLUE + plogBOLD,
    (std::string)plogPURPLE + plogBOLD
};

    std::string headerLine = "";

    for (size_t i = 0; i < headers.size(); i++) {
        headerLine += headerColors[i] + headers[i] + "\x1b[0m";
        headerLine += std::string(MaxColumnStringSize[i] - headers[i].length() + 3, ' ') + repeat;
    };

    plog((std::string)plogYELLOW + (std::string)plogBOLD, "info", headerLine);

    for (size_t i = 0; i < maxLength; i++) {

        std::string cppFile = i < ExtraNessecaryCppFiles.size() ? ExtraNessecaryCppFiles[i] : "";
        std::string include = i < IncludeDirectory.size() ? IncludeDirectory[i] : "";
        std::string library = i < LibraryDirectory.size() ? LibraryDirectory[i] : "";
        std::string specificLibrary = i < SpecificLibrary.size() ? SpecificLibrary[i] : "";
        std::string finalLine = "";

        finalLine += "├─" + cppFile + std::string(MaxColumnStringSize[0] - cppFile.length() + 1, ' ') + repeat;
        finalLine += "├─" + include + std::string(MaxColumnStringSize[1] - include.length() + 1, ' ') + repeat;
        finalLine += "├─" + library + std::string(MaxColumnStringSize[2] - library.length() + 1, ' ') + repeat;
        finalLine += "├─" + specificLibrary + std::string(MaxColumnStringSize[3] - specificLibrary.length() + 1, ' ') + repeat;

        plog((std::string)plogYELLOW + (std::string)plogBOLD, "info", finalLine);
    };


    std::string bd = BuildDirectory;
    std::string finalstr = "g++ -std=c++23 -Wall -O2 ";

    finalstr += bd + "\\src\\backend\\" + mainCompile + " ";

    std::ofstream bc(outputBuildLogFile, std::ios::app);

    for (const auto& file : ExtraNessecaryCppFiles) {
        finalstr = finalstr + bd + "\\src\\backend\\" + file + " ";
    };

    for (const auto& inc : IncludeDirectory) {
        if (!IncludeHeaderAdded) {
            finalstr = finalstr + "-I" + inc + " ";
            continue;
        };

        finalstr = finalstr + inc;
    };

    for (const auto& lib : LibraryDirectory) {
        if (!GeneralLibraryHeaderAdded) {
            finalstr = finalstr + "-L" + lib + " ";
            continue;
        };

        finalstr = finalstr + lib;
    };

    for (const auto& splib : SpecificLibrary) {
        if (!SpecificLibraryHeaderAdded) {
            finalstr = finalstr + "-l" + splib + " ";
            continue;
        };

        finalstr = finalstr + splib;
    };

    plog((std::string)plogYELLOW+(std::string)plogBOLD, "info", "");

    finalstr = finalstr + "-o " + outputExecutableDirectory + outputName;
    finalstr = finalstr + " && " + outputExecutableDirectory + outputName;

    plog((std::string)plogRED+(std::string)plogBOLD, "buildtpp", "Compiling and running: " + (std::string)plogORANGE+(std::string)plogBOLD + finalstr);
    int status = std::system(finalstr.c_str());

    if (status == 0) {
        plog((std::string)plogRED+(std::string)plogBOLD, "buildtpp", "build \x1b[1;38;5;46;49msuccess");
    } else {
        plog((std::string)plogRED+(std::string)plogBOLD, "buildtpp", "build \x1b[1;38;5;196;49mfailure");
    }

    if (BuildCountAppendage) {
        plog((std::string)plogPINK+(std::string)+plogBOLD, "buildpost", "writing to .build_count recent build...");

        std::string buildclog = "build ";

        if (status != 0) {
            buildclog += "!";
        } else {
            buildclog += "+";
        };

        auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
        auto local = std::chrono::zoned_time{"America/New_York", now};
        std::string time = std::format("{:%H%M%S%m%d%y}", local);
        buildclog += time;
        buildclog += BuildLogSuffix;

        writetofilesingle(outputBuildCountFile, buildclog);
    };

    writetofilelogvec(outputBuildLogFile);

    return 0;
}; 