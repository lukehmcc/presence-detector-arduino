#include <NewPing.h>
//Defining where the components are attached
#define TRIG_0 12
#define TRIG_1 7
#define ECHO_0 13
#define ECHO_1 8

#define iterations 5 //Number of readings in the calibration stage
#define MAX_DISTANCE 300 // Maximum distance (in cm) for the sensors to try to read.
#define DEFAULT_DISTANCE 45 // Default distance (in cm) is only used if calibration fails.
#define MIN_DISTANCE 15 // Minimum distance (in cm) for calibrated threshold.

float calibrate_0 = 0, calibrate_1 = 0; // The calibration in the setup() function will set these to appropriate values.
float distance_0, distance_1; // These are the distances (in cm) that each of the Ultrasonic sensors read.
int count = 0, limit = 20; //Occupancy limit should be set here: e.g. for maximum 8 people in the shop set 'limit = 8'.
bool prev_blocked0 = false, prev_blocked1 = false; //These booleans record whether the entry/exit was blocked on the previous reading of the sensor.

NewPing sonar[2] = {   // Sensor object array.
  NewPing(TRIG_0, ECHO_0, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
  NewPing(TRIG_1, ECHO_1, MAX_DISTANCE)
};

/*
   A quick note that the sonar.ping_cm() function returns 0 (cm) if the object is out of range / nothing is detected.
   We will include a test to remove these erroneous zero readings later.
*/

void setup() {
  Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
  pinMode(2, OUTPUT); pinMode(5, OUTPUT); pinMode(A0, OUTPUT); pinMode(A3, OUTPUT); pinMode(11, OUTPUT);
  digitalWrite(2, HIGH); digitalWrite(5, LOW); digitalWrite(A0, HIGH); digitalWrite(A3, LOW); digitalWrite(11, LOW);
  Serial.println("Calibrating...");
  delay(1500);
  for (int a = 0; a < iterations; a++) {
    delay(50);
    calibrate_0 += sonar[0].ping_cm();
    delay(50);
    calibrate_1 += sonar[1].ping_cm();
    delay(200);
  }
  calibrate_0 = 0.75 * calibrate_0 / iterations; //The threshold is set at 75% of the average of these readings. This should prevent the system counting people if it is knocked.
  calibrate_1 = 0.75 * calibrate_1 / iterations;

  if (calibrate_0 > MAX_DISTANCE || calibrate_0 < MIN_DISTANCE) { //If the calibration gave a reading outside of sensible bounds, then the default is used
    calibrate_0 = DEFAULT_DISTANCE;
  }
  if (calibrate_1 > MAX_DISTANCE || calibrate_1 < MIN_DISTANCE) {
    calibrate_1 = DEFAULT_DISTANCE;
  }

  Serial.print("Entry threshold set to: ");
  Serial.println(calibrate_0);
  Serial.print("Exit threshold set to: ");
  Serial.println(calibrate_1);
  delay(1000);
}

void loop() {
  Serial.print("Count: ");
  Serial.println(count);
  // inital ping
  distance_0 = sonar[0].ping_cm();
  delay(40); // Wait 40 milliseconds between pings. 29ms should be the shortest delay between pings.
  distance_1 = sonar[1].ping_cm();
  delay(40);
  Serial.print("Distance 0: ");
  Serial.println(distance_0);
  Serial.print("Distance 1: ");
  Serial.println(distance_1);
  
  // if sensor0 is triggered, wait and see if sensor1 is triggered for 8 cycles
  if (distance_0 < calibrate_0 && distance_0 > 0) {
    if (prev_blocked0 == false) {
      // now check to see if sensor1 is triggered a couple times
      for (int i = 0; i < 20; i++){
        distance_1 = sonar[1].ping_cm();
        if (distance_1 < calibrate_1 && distance_1 > 0){
          count++;
          delay(1000);
          break;
        }
        delay(40);
      }
    }
    prev_blocked0 = true;
  } else {
    prev_blocked0 = false;
  }
  

  // if sensor1 is triggered, wait and see if sensor0 is triggered for 8 cycles
  if (distance_1 < calibrate_1 && distance_1 > 0) {
    if (prev_blocked1 == false) {
      // now check to see if sensor1 is triggered a couple times
      for (int i = 0; i < 20; i++){
        distance_0 = sonar[0].ping_cm();
        if (distance_0 < calibrate_0 && distance_0 > 0){
          if (count > 0){
            count--;
            delay(1000);
            break;
          }
        }
        delay(40);
      }
    }
    prev_blocked1 = true;
  } else {
    prev_blocked1 = false;
  }
}