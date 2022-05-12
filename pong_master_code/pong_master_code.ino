//Connections: A4 to A4, A5 to A5 and GND to GND
#include <Wire.h> //For Arduino-Arduino communication

// Serial communication variables
char receivedCommand; //Command that decides what to do
double receivedValue; //Command that determines the amount of steps/speec/acceleration...
char ReceivedFromSlave; //received character from the Slave

// Motor control variables
bool newData; //bool to indicate new data - This is for the Computer - Master communication (Serial)
bool SendToSlave; //bool to indicate when there is data to be sent to the Slave
bool MotorIsRunning; //bool to check the status from the Slave Arduino 
String wireCommand; // the full string to be sent to the Slave

//Time for non-blocking tasks
float TimeNow1;
float TimeNow2;

void setup() {
  Serial.begin(9600); //Start Serial
  Serial.println("This is the Master Arduino that is sending the commands to the Slave");
  
  Wire.begin(8); // Join the i2c bus 
  Wire.onReceive(receiveEvent); // register I2C event

  delay(200);

  TimeNow1 = millis(); //Start time  
}


void loop() {
  checkSerial();
  SendDataToSlave();
  doSomething();    // we do something
  
}

void checkSerial() {    //This function is taking care of the serial communication between the master and the computer.
  if (Serial.available() > 0) {
    receivedCommand = Serial.read(); //Read the available command
    newData = true; //Allow the code to enter the next if()
    switch (receivedCommand) {    //we check what is the command - we read this a few lines above
    
      case 'R': //R uses the move() function of the AccelStepper library, which means that it moves relatively to the current position.   
      receivedValue = Serial.parseFloat(); //value for the steps        
      SendToSlave = true; // we allow the Master to forward the details to the Slave
      MotorIsRunning = true;
      break;
      
      case 'r': //r uses the moveTo() function of the AccelStepper library, which means that it moves absolutely to the current position.     
      receivedValue = Serial.parseFloat(); //value for the steps        
      SendToSlave = true; // we allow the Master to forward the details to the Slave
      MotorIsRunning = true;
      break;
      
      case 'n': // Stops the motor    
      receivedValue = 0; //Manually forcing the value to 0   
      SendToSlave = true; // we allow the Master to forward the details to the Slave
      MotorIsRunning = false;
      break;
      
      case 'A': // Updates acceleration 
      receivedValue = Serial.parseFloat(); //receive the acceleration from serial
      SendToSlave = true; // we allow the Master to forward the details to the Slave
      MotorIsRunning = false;
      break;
      
      case 'V': // Updates speed        
      receivedValue = Serial.parseFloat(); //receive the acceleration from serial
      SendToSlave = true; // we allow the Master to forward the details to the Slave
      MotorIsRunning = false;
      break;
      
      case 'v': // Updates Max speed        
      receivedValue = Serial.parseFloat(); //receive the acceleration from serial
      SendToSlave = true; // we allow the Master to forward the details to the Slave
      MotorIsRunning = false;
      break;        
      
      default:
      //skip
      break;
    }
  }
}

void SendDataToSlave() {
  if(SendToSlave == true) {   //if the code below is allowed to be performed
    Wire.beginTransmission(8); // transmit to Slave

    //Build up the string that we send to the Slave   
    String wireCommand = String(receivedCommand) + receivedValue; //concatenating string: command character + some value (number)

    //Checking what we send to the Slave by showing it on the terminal
    Serial.print("Submitted command: ");
    Serial.println(wireCommand);                     
    Wire.write(wireCommand.c_str()); //Sending the concatenated string to the Slave  
    Wire.endTransmission();    //Finish transmitting
    SendToSlave = false; //We are not allowed to enter this part of the code until we have new things to send
  }     
  else {
    return; // This else might not be necessary
  }      
}

void receiveEvent(int NumberOfBytes) {    
  while (Wire.available()){   // loop through all received character from the Slave (in this example, there is 1 character only)
    ReceivedFromSlave = Wire.read();// receive byte as a character    
    
    if(ReceivedFromSlave == 'd') {   //d = done       
      Serial.println("The motor finished the task!");
      MotorIsRunning = false;        
      break; //quick way to not finish the whole reading when we want to stop
    }                        
  }
}

void doSomething() {
  if(MotorIsRunning == true)
  {
  Serial.println("The function is doing something...");  
  }
}
