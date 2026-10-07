#include "EmbeddedFunctions.h"
#include <iostream>
#include <thread>
#include <chrono>

int main(void) {
    EmbeddedFunctions^ funcs = gcnew EmbeddedFunctions();
    std::cout << "Connecting..." << std::endl;
    //funcs->GOpen("192.168.0.120", 23);
    funcs->GOpen("127.0.0.1", 26000);  // real lab use 192.168.0.120 and 23
    std::cout << "Connected!" << std::endl;
    String^ res1 = funcs->GCommand("OP 255, 255");
    Console::WriteLine(res1);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    String^ res2 = funcs->GCommand("TI 0");
    Console::WriteLine(res2);
    String^ res3 = funcs->GCommand("TI 1");
    Console::WriteLine(res3);
    std::cin.get();
    return 0;
}