#include "Galil.h"
#include <iostream>
#include <thread>
#include <chrono>

class TestGalil : public Galil {
public:
    using Galil::Galil;
    using Galil::SendCommand;
};

static void pause(int seconds) { std::this_thread::sleep_for(std::chrono::seconds(seconds)); }

static void check(const char* name, bool ok) {
    std::cout << (ok ? "PASS  " : "FAIL  ") << name << "\n";
}

int main(void) {
    EmbeddedFunctions funcs(true);

    Galil a(&funcs, "192.168.0.120 -d");
    Galil b(&funcs, "192.168.0.120 -d");
    a.setKp(5.0);
    b = a;                                  // operator= fires here
    std::cout << "b's Kp after assignment: " << b.getKp() << std::endl;  // expect 5.0
    b = b;                                  // self-assignment — should not crash or leak

    std::cout << "\nPress Enter to exit...";
    std::cin.get();
    return 0;
}