// Get serial input and handle a lasercannon
// Author: Luis Bustamante
// Date: 26 December 2012
#include <AccelStepper.h>//Download at https://github.com/adafruit/AccelStepper
#include <AFMotor.h> //Download athttps://github.com/adafruit/Adafruit-Motor-Shield-library
#include <Servo.h>//Default Servo library


Servo servoTilt;  // a maximum of eight servo objects can be created 
// the possible states of the state-machine
typedef enum {  NONE, GOT_A, GOT_E, GOT_F } states;

// current state-machine state
states state = NONE;
// current partial number
unsigned int currentValue;
  
AF_Stepper motor1(200, 1);
int REVOLUTION = 200;
long azimuth=0, elevation = 0, fire = 0;
long pos= 0, lastPos = 0;
boolean control = true;

//laser 
// set pin numbers:
const int laserPin =  10; 
int laserState = LOW;             // ledState used to set the LED
long previousMillis = 0;        // will store last time LED was updated


void forwardstep() {  
  motor1.onestep(FORWARD, INTERLEAVE);
}
void backwardstep() {  
  motor1.onestep(BACKWARD, INTERLEAVE);
}

AccelStepper stepper(forwardstep, backwardstep);//Register stepping methods to the AFMotor lib

void setup ()
{
  Serial.begin (57600);
  state = NONE;
  servoTilt.attach(9);
  stepper.setMaxSpeed(400.0);
  stepper.setAcceleration(100.0);
  pinMode(laserPin, OUTPUT); 
  Serial.println("Receiving... ");
}  // end of setup

void processAzimuth (const unsigned int value)
{
  // do something with Azimuth 
  azimuth = value*2; //duplicate to match INTERLEAVE steps. 180° 
  stepper.moveTo(mapToRange());
  Serial.print ("Azimuth = ");
  Serial.println (value);
} // end of processAzimuth

void processElevation (const unsigned int value)
{
  // do something with Elevation
  elevation = value;
  servoTilt.write(180-elevation); //Invert Due to SERVOMOTOR position. Just passing Elevation should be enough with the right setup 
  Serial.print ("Elevation = ");
  Serial.println (value);
} // end of processElevation

void processFire (const unsigned int value)
{
  // do something with Fire 
  fire = value;
  Serial.print ("Fire = ");
  Serial.println (value);  
  state=NONE;
} // end of processFire

void handlePreviousState ()
{
  switch (state)
  {
  case GOT_A:
    processAzimuth (currentValue);
    break;
  case GOT_E:
    processElevation (currentValue);
    break;
  case GOT_F:
    processFire (currentValue);
    break;
  }  // end of switch  

  currentValue = 0; 
}  // end of handlePreviousState

void processIncomingByte (const byte c)
{
  if (isdigit (c))
  {
    currentValue *= 10;
    currentValue += c - '0';
  }  // end of digit
  else 
  {

    // The end of the number signals a state change
    handlePreviousState ();

    // set the new state, if we recognize it
    switch (c)
    {
    case 'A':
      state = GOT_A;
      break;
    case 'E':
      state = GOT_E;
      break;
    case 'F':
      state = GOT_F;
      break;
    default:
      state = NONE;
      break;
    }  // end of switch on incoming byte
  } // end of not digit  
  
} // end of processIncomingByte

void loop ()
{

  
  if (Serial.available ())
    processIncomingByte (Serial.read ());
    
     
  stepper.run();
  if(stepper.distanceToGo() == 0 && lastPos > 0){
    lastPos = pos;
    
  }
  // do other stuff in loop as required
  if (fire == 1)
      laserState = HIGH;
    if (fire == 0)
      laserState = LOW;
   digitalWrite(laserPin, laserState); 
}  // end of loop


long mapToRange(){
  pos = map(azimuth,0 , 360, 0, REVOLUTION);//(azimuth) * (2048) / (360);
  return pos-lastPos;
  //return pos;  
}
