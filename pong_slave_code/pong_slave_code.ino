#include <Wire.h>
#include <AccelStepper.h>

// Stepper driver pinouts
//AccelStepper stepper(1, 11, 12); //With EasyDriver
AccelStepper stepper(AccelStepper::DRIVER, 8, 9);  //With TB6600
#define rightLimit  4
#define leftLimit   5

#define stepsPerMM -17

//------------Variables for the stepping----------------------------------------
char    c;                          // stores the individual characters as they are received from the master
char    receivedCommand;            // received command charater
long    receivedValue;              // value from the computer
int     numberOfSteps;              // number of steps which will be passed to the AccelStepper library, determined by command, 200 steps / rotation with full steps
bool    steppingComplete = true;    // false while stepping is occuring. Used to print end position message
long    time1;
bool    rightLimitSet = true;



void setup() {
    Serial.begin(9600);  
    Serial.println("This is the Slave Arduino that is receiving the commands from the Master");
  
                                                            //setting up some default values for (max) speed and maximum acceleration
    stepper.setSpeed(2000);                                 //SPEED = Steps / second, this is the speed used with runSpeed().
    stepper.setMaxSpeed(1500);                              //SPEED = Steps / second. max speed should not exceed 1500 when full stepping at 9V 1.5A
    stepper.setAcceleration(250);                           //ACCELERATION = Steps /(second)^2 (1000). not sure what the max acceleration without losing steps is
    stepper.disableOutputs();                               //disable outputs, so the motor is not getting warm (no current)
    
    Wire.begin(8);                                          // join i2c bus with address #8  
    Wire.onReceive(receiveEvent);                           // register I2C event
}

void loop() {
    if (!rightLimitSet) {
        limitProtocol();
    }
    runMotor(); 
    //delay(1);
}   


void dataProcess(char charsToProcess[]) {  
    receivedCommand = charsToProcess[0];          //command is the first character of the string from the Master  
    charsToProcess[0] = '0';
    
    if (charsToProcess[1] == '-') {
        charsToProcess[1] = '0';
        receivedValue = atol(charsToProcess);     //the value temporary variable is built up one by one in each iteration
        receivedValue = -receivedValue;
    }
    else {
        receivedValue = atol(charsToProcess);     //the value temporary variable is built up one by one in each iteration
    }
    Serial.print("command: ");
    Serial.println(receivedCommand);
    Serial.print("value: ");
    Serial.println(receivedValue);
    switch (receivedCommand) {                              //we check what is the command
  
        case 'R':                                           //R uses the move() function of the AccelStepper library, which means that it moves relatively to the current position.           
            //Serial.println("Relative direction.");          //print the action
            rotateRelative(receivedValue);                               //Run the function, example: R2000 - 2000 steps (5 revolution with 400 step/rev microstepping)
            break;
        
        case 'r':                                           //r uses the moveTo() function of the AccelStepper library, which means that it moves absolutely to the current position.  
            //Serial.println("Absolute position.");           //print the action
            rotateAbsolute(receivedValue);                               //Run the function, example: r800 - It moves to the position which is located at +800 steps away from 0.
            break;
        
        case 'A':                                           // Updates acceleration        
            stepper.setAcceleration(receivedValue);         //update the value of the variable - receivedValue is the acceleration 
            Serial.print("New acceleration value: ");       //confirm update by message
            Serial.println(receivedValue);                  //confirm update by message
            break;
        
        case 'V':                                           // Updates speed
            stepper.setSpeed(receivedValue);                //update the value of the variable - receivedValue is the speed 
            Serial.print("New speed value: ");              //confirm update by message
            Serial.println(receivedValue);                  //confirm update by message
            break;
        
        case 'v':                                           // Updates Max speed
            stepper.setMaxSpeed(receivedValue);             //update the value of the variable - receivedValue is the max speed 
            Serial.print("New max speed value: ");          //confirm update by message
            Serial.println(receivedValue);                  //confirm update by message
            break;
        
        case 'U':
            stepper.setCurrentPosition(0);                  //Reset current position. "new home"      
            Serial.print("New home position is set.");      //Print message
            Serial.println(stepper.currentPosition());      //Check position after reset.
            break;        

        case 'L':
            Serial.println("setting right limit");
            rightLimitSet = false;
            limitProtocol();
            
            break; 
    }
}

void receiveEvent(int bytesToReceive) {
    time1 = micros();
    char receivedChars[bytesToReceive - 1];
    for (int i = 0; i < bytesToReceive - 1; i++) {
        receivedChars[i] = Wire.read();
    }
    Wire.read();    // reads the terminating char at the end of the transmission
    dataProcess(receivedChars);
}

void runMotor() {    //method for the motor
    //stepper.enableOutputs(); //enable pins
    if (!stepper.run() && steppingComplete == false) {
        steppingComplete = true;
        Serial.print("Position: "); 
        Serial.println(stepper.currentPosition() / stepsPerMM); // print pos -> this will show you the latest position from the origin 
        Serial.println();
        //masterFeedback();    // tells the master that stepping is complete
    }

}

void masterFeedback() {    //Send a feedback to the Master
    Wire.beginTransmission(8);
    Wire.write('d'); // d = done
    Wire.endTransmission();  
}

void rotateRelative(long mmToMove) {    //We move X steps from the current position of the stepper motor in a given direction (+/-).  
    Serial.println("rotating ");
    Serial.println(mmToMove);
    stepper.move(mmToMove * stepsPerMM); //set relative distance and direction
    steppingComplete = false;
    runMotor();
}

void rotateAbsolute(long steps) {    //We move to an absolute position. 
    stepper.moveTo(steps * stepsPerMM); //set absolute distance  
    steppingComplete = false;
    runMotor();
}

void limitProtocol() {      // moves carriages until limit switches are set and then initiallizes new home positions
    if (digitalRead(rightLimit)) {   // until limit is hit, move to the right 1mm
        rotateRelative(-1);
    }
    else {
        rightLimitSet = true;
        stepper.setCurrentPosition(-5*stepsPerMM);
        rotateAbsolute(0);
        Serial.println("limit reached");
    }    
}
