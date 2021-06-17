#copyright (c) 2021 Akromatikus

#include <NewPing.h>
#include <Servo.h>

//PIN DEFINITIONS
#define TRIGGER_PIN_1     7
#define ECHO_PIN_1        6
#define TRIGGER_PIN_2     5
#define ECHO_PIN_2        4
#define MAX_DISTANCE_LEFT 15
#define servo_pin         9

//OBJECTS
NewPing sonar_left(TRIGGER_PIN_1, ECHO_PIN_1, MAX_DISTANCE_LEFT);
NewPing sonar_right(TRIGGER_PIN_2, ECHO_PIN_2, MAX_DISTANCE_LEFT);
Servo servo_obj;

// GLOBAL VARIABLES
int DISTANCE_LEFT = 0; //measured distances of surface for each sensor
int DISTANCE_RIGHT = 0; 
int SURVEY_INCREMENT = 60; //increments in degrees
int SURVEY_DURATION = 5; //durations in ms
int PERIPH_INCREMENT = 13;
int PERIPH_DURATION = 5;

// START PROGRAM
void setup() { 
  servo_obj.attach(servo_pin);  
  servo_obj.write(0);
}

//MAIN LOOP
void loop() {
  survey(SURVEY_INCREMENT, SURVEY_DURATION, 1);    
}


// FUNCTIONS-----------------------


void servo_move(int range, int duration) {  
  
  //Range is halved because the servo will move in degrees of 2
  //in order to offset any "stickyness" of the servo. 
  range = range / 2;  
  int initial = servo_obj.read();
  int target = initial + range; 
  
  // if the servo needs to move clockwise
  if (target < initial) {      
    
    // if that movement would put the servo out of range    
    if (target < 0) {
      
      //limit the range to achieve position degree 0
      range = initial / 2;      
    }
    
    for (int i = 1 ; i <= abs(range) ; i += 1) {      
      servo_obj.write(initial - (2 * i));             
      delay(duration);     
    }   
  }  
  
  // else if the servo needs to move counterclockwise, repeat previous steps in the reverse direction
  else if (target > initial) {    
    if (target > 180) {     
      range = (180 - initial) / 2;     
    }
        
    for (int i = 1 ; i <= range ; i += 1) {      
      servo_obj.write(initial + (2 * i));      
      delay(duration);      
    }    
  }
}

//while no surfaces are found the servo will 'survey' the area
void survey(int measure_increment, int duration, int rotation_direction) { 
  target_seek(rotation_direction);
    
  while (1 == 1) {
    servo_move(measure_increment * rotation_direction, duration);         
    target_seek(rotation_direction);

    if ( servo_obj.read() > (180 - PERIPH_INCREMENT) || servo_obj.read() < PERIPH_INCREMENT) {
         rotation_direction = rotation_direction * -1;
    }      
  }   
}

//as the servo surveys, check for a surface reading at the desired rotation increments
void target_seek(int rotation_direction) { 
  DISTANCE_LEFT = sonar_left.convert_cm( sonar_left.ping_median() );  
  DISTANCE_RIGHT = sonar_right.convert_cm( sonar_right.ping_median() );
  
  //if surface in range of either sensor is measured
  if (DISTANCE_LEFT > 0 || DISTANCE_RIGHT > 0) {               
    peripheral_check(PERIPH_INCREMENT, PERIPH_DURATION);    
  } 
}

//if either sensor detects a surface, peripheral check will hone in on the surface so that both sensors detect it
void peripheral_check(int increment, int duration) { 
  
  //WHILE THE DIFFERENCE BETWEEN SENSOR DISTANCES IS GREATER THAN TWO AND NEITHER SENSOR EQUALS 0
  while ( abs((DISTANCE_RIGHT - DISTANCE_LEFT)) > 2 and DISTANCE_LEFT * DISTANCE_RIGHT != 0) {
    servo_move(PERIPH_INCREMENT * ((DISTANCE_RIGHT-DISTANCE_LEFT) / abs(DISTANCE_RIGHT-DISTANCE_LEFT) ), PERIPH_DURATION);
    delay(5);
    
    DISTANCE_LEFT = sonar_left.convert_cm( sonar_left.ping_median() );  
    DISTANCE_RIGHT = sonar_right.convert_cm( sonar_right.ping_median() ); 
  }

  // if both sensors do not read zero
  if (DISTANCE_LEFT*DISTANCE_RIGHT > 0) {
    target_lock();    
  }

  // if sensor distances are not equal and neither equals zero
  else if (DISTANCE_LEFT != DISTANCE_RIGHT and DISTANCE_LEFT*DISTANCE_RIGHT != 0) {
    survey(SURVEY_INCREMENT, SURVEY_DURATION, -1 * (DISTANCE_LEFT - DISTANCE_RIGHT) / abs(DISTANCE_LEFT - DISTANCE_RIGHT) );
  }
}

//keeps the sensors locked on the surface by moving the servo appropriately
void target_lock() {
  int bias = (DISTANCE_LEFT - DISTANCE_RIGHT) / abs(DISTANCE_LEFT - DISTANCE_RIGHT); 

  //while both sensors have similar readings and neither equals zero
  while ( abs(DISTANCE_LEFT - DISTANCE_RIGHT) < 2 and DISTANCE_LEFT*DISTANCE_RIGHT > 0) {
    if (DISTANCE_LEFT != DISTANCE_RIGHT) {
      
      //integrate the biases while a target is locked so that if the target is lost the servo will move in
      // the most likely direction of the lost target
      bias += (DISTANCE_LEFT - DISTANCE_RIGHT) / abs(DISTANCE_LEFT - DISTANCE_RIGHT) / 2;
    } 
    
    DISTANCE_LEFT = sonar_left.convert_cm( sonar_left.ping_median() );  
    DISTANCE_RIGHT = sonar_right.convert_cm( sonar_right.ping_median() );
    delay(5);   
  }  
    survey(SURVEY_INCREMENT, SURVEY_DURATION, -bias );
}
