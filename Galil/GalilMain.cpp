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
    TestGalil g(&funcs, "192.168.0.120 -d");

    // 0. Raw probe: does the simulator understand these commands at all?
    std::cout << "Raw QE 0; -> [" << g.SendCommand("QE 0;") << "]\n";
    std::cout << "Raw WE 0; -> [" << g.SendCommand("WE 0;") << "]\n\n";

    // 1. Zero it first, so we start from a known state
    g.WriteEncoder();
    check("WriteEncoder() write ok", g.CheckSuccessfulWrite());
    std::cout << "  ReadEncoder() after zeroing: " << g.ReadEncoder() << "  (expect 0)\n\n";

    // 2. Spin the motor forward, watch the count climb
    std::cout << "Spinning motor forward for 2s. Watch the 'Encoder counts' display...\n";
    g.AnalogOutput(0, 3.0);
    pause(2);
    g.AnalogOutput(0, 0.0);            // stop
    int forwardCount = g.ReadEncoder();
    std::cout << "  ReadEncoder() after forward spin: " << forwardCount << "  (expect nonzero)\n\n";

    // 3. Zero again, then spin the other direction
    g.WriteEncoder();
    std::cout << "Spinning motor in reverse for 2s...\n";
    g.AnalogOutput(0, -3.0);
    pause(2);
    g.AnalogOutput(0, 0.0);
    int reverseCount = g.ReadEncoder();
    std::cout << "  ReadEncoder() after reverse spin: " << reverseCount << "  (expect negative)\n\n";

    check("Forward spin gave a nonzero count", forwardCount != 0);
    check("Reverse spin gave the opposite sign", (forwardCount > 0) == (reverseCount < 0));

    g.setKp(1.5);
    g.setKi(0.2);
    g.setKd(0.05);
    g.setSetPoint(1000);

    std::cout << "Kp=" << g.getKp()
        << " Ki=" << g.getKi()
        << " Kd=" << g.getKd()
        << " SetPoint=" << g.getSetPoint() << std::endl;

    std::cout << "\nPress Enter to exit...";
    std::cin.get();
    return 0;
}