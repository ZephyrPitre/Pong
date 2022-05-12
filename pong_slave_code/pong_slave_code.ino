#include <Wire.h>
#include <AccelStepper.h>

// Stepper driver pinouts
//AccelStepper stepper(1, 11, 12); //With EasyDriver
AccelStepper stepper(1, 8, 9); //With TB6600

//------------Variables for the data processing---------------------------------
String str ="";
char commandChar; //command character
String valueFromChars; //value constructed from the chars
char c; //just a simple character
bool printMessage; // bool that allows to print out the received values from Master
//------------Variables for the stepping----------------------------------------
char receivedCommand; //received command charater
long receivedValue; //value from the computer
long NumberOfSteps; //number of steps which will be passed to the AccelStepper library
String receivedString; //the full string received from the Master - this will be split up
bool runMotor = false;

void setup() {
    Serial.begin(9600);  
    Serial.println("This is the Slave Arduino that is receiving the commands from the Master");
  
    //setting up some default values for (max) speed and maximum acceleration
    stepper.setSpeed(2000);   //SPEED = Steps / second
    stepper.setMaxSpeed(2000);    //SPEED = Steps / second
    stepper.setAcceleration(500);   //ACCELERATION = Steps /(second)^2 (1000)
    stepper.disableOutputs();   //disable outputs, so the motor is not getting warm (no current)
    
    Wire.begin(8);    // join i2c bus with address #8  
    Wire.onReceive(receiveEvent);   // register I2C event
}

void loop() {    
    dataProcess();  
    dataDisplay();  
    RunTheMotor(); 
}   


void dataProcess(String ) {  
    if(incomingCommand == true) {    
        delay(2000); //Waiting time is necessary, otherwise the software doesn't read and arrange the variables properly
        //delete the strings before constructing the new variables, otherwise the new characters will be appended  
        commandChar ="";
        valueFromChars ="";  
        
        commandChar = str.charAt(0); //command is the first character of the string from the Master  
        
        for (int i = 1; i < str.length(); i++) {   // i starts from the index 1, since the index 0 is the command character    
            valueFromChars += str.charAt(i); //the value temporary variable is built up one by one in each iteration 
        }
        
        str = "";   //we erase the string what we decomposed above, so upon the new incoming data, we do not combine the old and new data  
        receivedCommand = commandChar;    //ReceivedCommand is a "more permanent" variable, so we put the command there
        receivedValue = valueFromChars.toFloat();   //the receivedValue is a string, so it has to be converted into a number         
    
       switch (receivedCommand) {   //we check what is the command
      
            case 'R': //R uses the move() function of the AccelStepper library, which means that it moves relatively to the current position.           
                Serial.println("Relative direction."); //print the action
                NumberOfSteps = receivedValue; //here, the receivedValue is used as the number of steps
                RotateRelative(); //Run the function, example: R2000 - 2000 steps (5 revolution with 400 step/rev microstepping)
                runMotor = true;
                break;
            
            case 'r': //r uses the moveTo() function of the AccelStepper library, which means that it moves absolutely to the current position.  
                Serial.println("Absolute position."); //print the action
                NumberOfSteps = receivedValue; //here, the receivedValue is used as the number of steps
                RotateAbsolute(); //Run the function, example: r800 - It moves to the position which is located at +800 steps away from 0.
                runMotor = true;
                break;
            
            case 'A': // Updates acceleration        
                stepper.setAcceleration(receivedValue); //update the value of the variable - receivedValue is the acceleration 
                Serial.print("New acceleration value: "); //confirm update by message
                Serial.println(receivedValue); //confirm update by message
                break;
            
            case 'V': // Updates speed
                stepper.setSpeed(receivedValue); //update the value of the variable - receivedValue is the speed 
                Serial.print("New speed value: "); //confirm update by message
                Serial.println(receivedValue); //confirm update by message
                break;
            
            case 'v': // Updates Max speed
                stepper.setMaxSpeed(receivedValue); //update the value of the variable - receivedValue is the max speed 
                Serial.print("New max speed value: "); //confirm update by message
                Serial.println(receivedValue); //confirm update by message
                break;
            
            case 'U':
                stepper.setCurrentPosition(0); //Reset current position. "new home"      
                Serial.print("New home position is set."); //Print message
                Serial.println(stepper.currentPosition()); //Check position after reset.
                break;        
            
            default:
                //skip
                break;
          }
        //after we went through the above tasks, newData is set to false again, so we are ready to receive new commands again.
        incomingCommand = false;  
        printMessage = true; //next iteration of the loop() will show these values  
    }
}

void dataDisplay() {    //this function is only used for checking purposes.
    if(printMessage == true) {
        Serial.println("Received string: ");    //full string from Master
        Serial.println(receivedString);
        Serial.println("Received character: ");   //sliced out command character
        Serial.println(commandChar);
        Serial.println("Received value: ");   //sliced out number 
        Serial.println(receivedValue);  
        printMessage = false;   //reset this, so we don't enter again until a new data comes in
    }  
}


String receiveEvent(int NumberOfBytes) {  
    while (Wire.available()) {    // loop through all received character from the Master
        c = Wire.read();// receive byte as a character    
        if(c == 'n') {    //if we get the letter n, we do not fool around but immediately shut down the motor
            runallowed = false;
            printMessage = false;
            Serial.println("STOPPED");        
            break;    //quick way to not finish the whole reading when we want to stop
        } 
    }    
    receivedString = receivedString + c; //appending the characters in order to build a whole string
    return(receivedString);
}

void RunTheMotor() {    //method for the motor
    stepper.enableOutputs(); //enable pins
    while (abs(stepper.distanceToGo()) > 0) {    //we enter this part as long as there are steps to do
        stepper.run(); //step the motor (this will step the motor by 1 step at each loop)
    }
    
    stepper.disableOutputs(); // disable power
    Serial.print("POSITION: "); 
    Serial.println(stepper.currentPosition()); // print pos -> this will show you the latest position from the origin 
    masterFeedback();
}

void masterFeedback() {
    //Send a feedback to the Master
    Wire.beginTransmission(8);
    Wire.write('d'); // d = done
    Wire.endTransmission();  
}

void RotateRelative() {
    //We move X steps from the current position of the stepper motor in a given direction (+/-).  
    runallowed = true; //allow running - this allows entering the RunTheMotor() function.  
    stepper.move(NumberOfSteps); //set relative distance and direction
}

void RotateAbsolute() {
    //We move to an absolute position. 
    //The AccelStepper library keeps track of the position. 
    runallowed = true; //allow running - this allows entering the RunTheMotor() function. 
    stepper.moveTo(NumberOfSteps); //set absolute distance  
}
