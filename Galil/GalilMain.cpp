#include "Galil.h"
#include <iostream>
#include <thread>
#include <chrono>

static void pause(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

int main(void) {
    EmbeddedFunctions funcs(true);                       
    Galil g(&funcs, "192.168.0.120 -d");

    std::cout << "\nPress Enter to exit...";
    std::cin.get();
    return 0;
}