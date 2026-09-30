#include "Galil.h"

Galil::Galil() : g(0), ownsFunctions(true), savedAddress("192.168.0.120 -d") {
    Functions = new EmbeddedFunctions();
    GReturn result = Functions->GOpen(savedAddress.c_str(), &g);
    if (result != G_NO_ERROR) {
        std::cout << "Galil: failed to open connection, error code: " << result << std::endl;
    }
}

Galil::Galil(EmbeddedFunctions* Funcs, GCStringIn address) : Functions(Funcs), g(0), ownsFunctions(false), savedAddress(address) {
	GReturn result = Functions->GOpen(address, &g);
    if (result != G_NO_ERROR) {
        std::cout << "Galil: failed to open connection, error code: " << result << std::endl;
    }
}

Galil::Galil(const Galil& other) : g(0), ownsFunctions(true), savedAddress(other.savedAddress)  {
    Functions = new EmbeddedFunctions();
    GReturn result = Functions->GOpen(savedAddress.c_str(), &g);
    if (result != G_NO_ERROR) {
        std::cout << "Galil copy: failed to open connection, error code: " << result << std::endl;
    }

    setPoint = other.setPoint;
    ControlParameters[0] = other.ControlParameters[0];
    ControlParameters[1] = other.ControlParameters[1];
    ControlParameters[2] = other.ControlParameters[2];
}

Galil::~Galil() {
	if (Functions != nullptr) {
		Functions->GClose(g);
        if (ownsFunctions) {
            delete Functions;
        }
	}
}

std::string Galil::SendCommand(const std::string& command) {
    char buffer[128] = {};
    GSize bytesReturned;
    Functions->GCommand(g, command.c_str(), buffer, sizeof(buffer), &bytesReturned);

    lastResponse = buffer; // to convert char array to string and store in lastResponse variable
    return lastResponse;
}

// Write to all 16 bits of digital output, 1 command to the Galil
void Galil::DigitalOutput(uint16_t value) {
    // OP n0, n1
    // n0 = Outputs 0–7(the "low byte")
    // n1 = Outputs 8–15(the "high byte")

    //     0000000100000001   (value)
    //   & 0000000011111111   (0xFF = 255)
    //     ----------------
    //     0000000000000001 = 1  (lowByte)
    uint8_t lowByte = value & 0xFF;
    uint8_t highByte = (value >> 8) & 0xFF;

    SendCommand("OP " + std::to_string(lowByte) + "," + std::to_string(highByte) + ";");
}

// Write to one byte, either high or low byte, as specified by user in 'bank'
// 0 = low, 1 = high
void Galil::DigitalByteOutput(bool bank, uint8_t value) {
    SendCommand(std::string("OP ") + (bank ? ",": "") + std::to_string(value) + ";");
}

// Write single bit to digital outputs. 'bit' specifies which bit
void Galil::DigitalBitOutput(bool val, uint8_t bit) {
    SendCommand(std::string(val ? "SB " : "CB ") + std::to_string(bit) + ";");
}

// Return the 16 bits of input data
// Query the digital inputs of the GALIL, See Galil command library @IN
uint16_t Galil::DigitalInput() {
    uint8_t low = DigitalByteInput(0);
    uint8_t high = DigitalByteInput(1);
    return (static_cast<uint16_t>(high) << 8) | low;
}

// Read either high or low byte, as specified by user in 'bank'
// 0 = low, 1 = high
// A bank is one byte (8 bits). The low bank (0) is the first 8
// bits (DI0-DI7) and the high bank (1) is the upper 8 bits
// (DI8-DI15).
uint8_t Galil::DigitalByteInput(bool bank) {
    // TI 0 for low bank, TI 1 for high bank
    //   DI7 DI6 DI5 DI4 DI3 DI2 DI1 DI0 (pin 0 and 2 are high)
    //       0   0   0   0   0   1   0   1 = binary 00000101 = decimal 5
    std::string response = SendCommand("TI " + std::to_string(bank) + ";");
    return static_cast<uint8_t>(std::stoi(response));
}

// Read single bit from current digital inputs. Above functions may use this function
bool Galil::DigitalBitInput(uint8_t bit) {
    return (DigitalInput() >> bit) & 1;
}

bool Galil::CheckSuccessfulWrite() {
    size_t lastPos = lastResponse.find_last_not_of(" \t\r\n");
    return lastPos != std::string::npos
        && lastResponse[lastPos] == ':'
        && lastResponse.find('?') == std::string::npos;
}

// ANALOG FUNCTIONS
// Read Analog channel and return voltage
float Galil::AnalogInput(uint8_t channel) {
    std::string response = SendCommand("MG @AN[" + std::to_string(channel) + "];");
    return std::stof(response);
}

// Write to any channel of the Galil, send voltages as 2 decimal place in the command string
void Galil::AnalogOutput(uint8_t channel, double voltage) {
    char voltStr[16];
    sprintf_s(voltStr, "%.2f", voltage);
    SendCommand("AO " + std::to_string(channel) + "," + voltStr + ";");
}

// Configure the range of the input channel with the desired range code
void Galil::AnalogInputRange(uint8_t channel, uint8_t range) {
    SendCommand("AQ " + std::to_string(channel) + "," + std::to_string(range) + ";");
}

// ENCODER
// Manually Set the motor encoder value to zero (encoder channel 0)
void Galil::WriteEncoder() {
    SendCommand("WE 0;");
}

// Read from motor Encoder (encoder channel 0)
int Galil::ReadEncoder() {
    std::string response = SendCommand("QE 0;");
    return std::stoi(response);
}


// CONTROL FUNCTIONS
// Set the desired setpoint for control loops, counts or counts/sec
// This should set it within the class not on the actual Galil.
void Galil::setSetPoint(int s) {
    setPoint = s;
}

// Gets the current setpoint stored in the class
double Galil::getSetPoint() {
    return setPoint;
}

// Set the proportional gain of the controller used in controlLoop() of Position/SpeedControl
// This should set it within the class not on the actual Galil.
void Galil::setKp(double gain) {
    ControlParameters[0] = gain;
}

// Gets the current proportional gain stored in the class
double Galil::getKp() {
    return ControlParameters[0];
}

// Set the integral gain of the controller used in controlLoop()  of Position/SpeedControl
// This should set it within the class not on the actual Galil.
void Galil::setKi(double gain) {
    ControlParameters[1] = gain;
}

// Gets the current integral gain stored in the class
double Galil::getKi() {
    return ControlParameters[1];
}

// Set the derivative gain of the controller used in controlLoop()  of Position/SpeedControl
// This should set it within the class not on the actual Galil.
void Galil::setKd(double gain) {
    ControlParameters[2] = gain;
}

// Gets the current derivative gain stored in the class
double Galil::getKd() {
    return ControlParameters[2];
}

// Run the control loop. ReadEncoder() is the input to the loop. The motor is the output.
// The loop will run using the PID values specified in the data of this object, and has an 
// automatic timeout of 10s. You do NOT need to implement this function, it is defined in
// GalilControl.lib

void Galil::PositionControl(bool debug, int Motorchannel) {

}

// same as above. Setpoint interpreted as counts per second
void Galil::SpeedControl(bool debug, int Motorchannel) {
}	


// OPERATOR OVERLOADS
// Operator overload for '<<' operator. So the user can say cout << Galil;
// This function should print out the output of GInfo and GVersion, with
// two newLines after each.
std::ostream& operator<<(std::ostream& output, Galil& galil) {
    char infoBuf[256];
    char verBuf[256];

    galil.Functions->GInfo(galil.g, infoBuf, sizeof(infoBuf));
    galil.Functions->GVersion(verBuf, sizeof(verBuf));

    output << infoBuf << "\n\n" << verBuf << "\n\n";
    return output;
}

// Copy assignment operator. This acts in the same way as the copy constructor
Galil& Galil::operator=(const Galil& other) {
    if (this == &other) return *this;

    if (Functions != nullptr) {
        Functions->GClose(g);
        if (ownsFunctions) {
            delete Functions;
        }
    }

    savedAddress = other.savedAddress;
    Functions = new EmbeddedFunctions();
    ownsFunctions = true;
    g = 0;
    GReturn result = Functions->GOpen(savedAddress.c_str(), &g);
    if (result != G_NO_ERROR) {
        std::cout << "Galil assignment: failed to open connection, error code: " << result << std::endl;
    }

    setPoint = other.setPoint;
    ControlParameters[0] = other.ControlParameters[0];
    ControlParameters[1] = other.ControlParameters[1];
    ControlParameters[2] = other.ControlParameters[2];

    return *this;
}
// (refer above for details).