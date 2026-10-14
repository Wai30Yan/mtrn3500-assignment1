#include "EmbeddedFunctions.h"
#include <iostream>
#include <thread>
#include <chrono>

int main(void) {
    EmbeddedFunctions^ funcs = gcnew EmbeddedFunctions();
    std::cout << "Connecting..." << std::endl;

    // for simulator
    //funcs->GOpen("127.0.0.1", 26000);  

    // for physical hardware
    funcs->GOpen("192.168.0.120", 23);

    std::cin.get();
    return 0;
}