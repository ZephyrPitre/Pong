//Connections to slave: A4s, A5s, GNDs, and 5Vs
 
#include <Wire.h> //For Arduino-Arduino communication

// Pins
#define rightRelay  2
#define leftRelay   3




long time1;
bool rightStepping;

void setup() {
    Serial.begin(9600);                                                                             //Start Serial
    Serial.println("This is the Master Arduino that is sending the commands to the Slave");
    Wire.begin(8);                                                                                  // Join the i2c bus 
//    Wire.onReceive(receiveEvent);                                                                   // register I2C event
    //pinMode(rightLimit, INPUT_PULLUP);
    //pinMode(leftLimit, INPUT_PULLUP);
}

void loop() {
    if (Serial.available() > 0) {
        interpretCommand(Serial.readString());
    }
    delay(1);
}

void interpretCommand(String receivedCommand) {     // decides between moving the motor, launching a ball, or initiating a limit sequence
    switch (receivedCommand.charAt(0)) {                              // this interpereets the command from Serial. Would be faster using chars instead of Strings
        /*case 'L':
            Serial.println("Limit protocol");
            limitProtocol();
            break;
*/
        case 'A':
            Serial.println("Right piston up");
            digitalWrite(rightRelay, HIGH);
            break;

        case 'B':
            Serial.println("Right piston down");
            digitalWrite(rightRelay, LOW);
            break;

        case 'C':
            Serial.println("Left piston up");
            digitalWrite(leftRelay, HIGH);
            break;

        case 'D':
            Serial.println("Left piston down");
            digitalWrite(leftRelay, LOW);
            break;

        default:
            controlRightMotor(receivedCommand);     // if the command is not to set limit or to move a paddle, pass it to the motor slave
    } 
}

void controlRightMotor(String receivedMotorCommand) {
    time1 = micros();
    Wire.beginTransmission(8);    // transmit to Slave
    Serial.print("Submitted motor command: ");    // Checking what we send to the Slave by showing it on the terminal
    Serial.println(receivedMotorCommand);                     
    Wire.write(receivedMotorCommand.c_str());    // Sending the string to the Slave  
    Wire.endTransmission();    // Finish transmitting
    Serial.println(micros() - time1);
}


/*void receiveEvent(int bytesToReceive) {
    time1 = micros();
    char receivedChars[bytesToReceive - 1];
    for (int i = 0; i < bytesToReceive - 1; i++) {
        receivedChars[i] = Wire.read();
    }
    Wire.read();    // reads the terminating char at the end of the transmission
    dataProcess(receivedChars);
}*/
