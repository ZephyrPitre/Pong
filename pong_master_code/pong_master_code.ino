//Connections: A4 to A4, A5 to A5 and GND to GND
#include <Wire.h> //For Arduino-Arduino communication

// Serial communication variables
char receivedFromSlave; //received character from the Slave

bool MotorIsRunning; //bool to check the status from the Slave Arduino 
String wireCommand; // the full string to be sent to the Slave

//Time for non-blocking tasks
float TimeNow1;
float TimeNow2;

void setup() {
    Serial.begin(9600); //Start Serial
    Serial.println("This is the Master Arduino that is sending the commands to the Slave");
    
    Wire.begin(8); // Join the i2c bus 
    Wire.onReceive(receiveEvent); // Do recieveEvent when I2C communication from slave

}

void loop() {
    if (Serial.available() > 0) {
        SendDataToSlave(Serial.readString());
    }
    doSomething();    // we do something
}

void SendDataToSlave(String receivedCommand) {
    Wire.beginTransmission(8);    // transmit to Slave
    Serial.print("Submitted command: ");    // Checking what we send to the Slave by showing it on the terminal
    Serial.println(receivedCommand);                     
    Wire.write(receivedCommand.c_str());    // Sending the string to the Slave  
    delay(1000);
    Wire.endTransmission();    // Finish transmitting
}

void receiveEvent(int NumberOfBytes) {    
    while (Wire.available()){   // loop through all received character from the Slave (in this example, there is 1 character only)
        receivedFromSlave = Wire.read();// receive byte as a character    
        if (receivedFromSlave == 'd') {   //d = done       
            Serial.println("The motor finished the task!");
            MotorIsRunning = false;        
            break; //quick way to not finish the whole reading when we want to stop
        }                        
    }
}

void doSomething() {
    if(MotorIsRunning == true) {
        Serial.println("The function is doing something...");  
    }
}
