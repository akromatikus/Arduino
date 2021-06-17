#copyright (c) 2021 Akromatikus

#include <Stepper.h> 

//global variables
float bobbinHeight = 6.5; //all units in mm
float wireWidth = 0.05;
int currentLayer, i = 0;
int layers = 41; //number of layers
float armRadius = 6.94; //guide motor arm radius
const int stepsPerRevolution = 4096;
int stepSize = 2048 / (2 * M_PI) * asin(wireWidth / armRadius); 
int stepsPerLayer = bobbinHeight / wireWidth;
volatile byte cycleTrigger = 1; //volatile stores byte in RAM instead of flash/EEPROM

Stepper guideMotor (stepsPerRevolution, 5, 7, 6, 8);

void setup() {

  guideMotor.setSpeed(5);
  Serial.begin(115200); 
  
  Serial.println("Adjust the guide motor (input values == steps in - or + direction)" );
  Serial.println("until it and the wire are the farthest to the right on the bobbin,");
  Serial.println("Enter 0 to continue");
  float input = 1;
  
  while (input != 0) {
     
     if (Serial.available() > 1) {
       input = Serial.parseFloat();
       
       if (input != 0) {
          guideMotor.step(2048 / (2 * M_PI) * asin(input / armRadius);                   
       }
     }
  }

  Serial.println("Adjust Wind Motor Until The Start Cycle Indicator Lights");
  
  while (digitalRead(3) == 1) {
    
  } 
  
  Serial.println("Enter 0 To Begin Winding");
  input = 1;
  
  while (input != 0) {
     
     if (Serial.available() > 1) {
       input = Serial.parseInt();
     }
  } 

  attachInterrupt(digitalPinToInterrupt(3), detectCycle, FALLING);
  
  Serial.println("Beginning Winding"); 
  analogWrite(10,255); //start motor    
}

void loop() {
  delay(5);
  
  while (currentLayer < layers) { //while there are layers left to wind
    i = 0; // start the wind counter for this layer
    
    while (i < stepsPerLayer) { // while there are winds left in the layer 
      
      if (cycleTrigger == 0) {  //if the cycle indicator magnet has been detected
          guideMotor.step(-stepSize); // move over to the next wind
          i++; 
          cycleTrigger = 1;                     
       }
    }
    
    guideMotor.step(15); //guide direction change lag compensation
    currentLayer += 1;
    i = 0;
    
    while (i < stepsPerLayer) { // while there are winds left in the layer 
      
      if (cycleTrigger == 0) {  //if the cycle indicator magnet has been detected
          guideMotor.step(stepSize); // move over to the next wind
          i++; 
          cycleTrigger = 1;                    
       }
    }
    
    guideMotor.step(-15) ; //guide direction change lag/wiggle room compensation
    currentLayer += 1 ; 
  }
  
  analogWrite(10, 0); //winding motor off
  Serial.println("WINDING COMPLETE!!!");
  
  while (2 + 2 != 5) { //end main code
    
  }   
}

void detectCycle() {  //interrupt service routine for winder rotation cycle
  cycleTrigger = 0;
}
