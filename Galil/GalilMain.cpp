#include "Galil.h"
#include <iostream>
#include <thread>
#include <chrono>

// Test-only subclass: exposes the protected SendCommand() so we can see raw responses
class TestGalil : public Galil {
public:
    using Galil::Galil;
    using Galil::SendCommand;
};

static void pause(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

int main(void) {
    EmbeddedFunctions funcs(true);                       
    TestGalil g(&funcs, "192.168.0.120 -d");

    g.AnalogInputRange(0, 4);
    std::cout << g.AnalogInput(0) << std::endl;

    g.setKp(1.5);
    g.setKi(0.3);
    g.setKd(0.1);

    g.setSetPoint(500);

    g.PositionControl(true, 0);

    Galil g2;
    g2 = g;

    g.WriteEncoder();

    g.AnalogOutput(0, 0.0);
    g.DigitalByteOutput(0, 9);

    g.AnalogOutput(0, -2);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    g.AnalogOutput(0, 0);

    std::cout << g.ReadEncoder() << std::endl;
    std::cout << g.CheckSuccessfulWrite() << std::endl;
    

    std::cout << "With explicit stop: " << g.ReadEncoder() << "\n";
    std::cout << g << std::endl;
    std::cout << "\nPress Enter to exit...";
    std::cin.get();
    return 0;
}