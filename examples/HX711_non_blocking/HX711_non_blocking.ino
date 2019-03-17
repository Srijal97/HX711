/*
 * This is a piece of code from my project which tries to maintain constant tension on a string using a stepper motor and the HX711. 
 * Note that the actual readings from the HX711 will arrive slowly but now the modified code does not wait for 
 * new readings to arrive (like the default library) and instead runs multiple iterations of loop() for the same reading.
 */


#include "Arduino.h"
#include "HX711.h"
#include "AccelStepper.h"

HX711 scale(3, 2);  // (DOUT, CLK)

double tension;
double tension_setpoint;
double output_rpm;
int dir = 1;

double Kp = 1;

bool stop_motor = false;

int calibration_factor = 2240;

int trim_in_pin = A0;
int trim_value = 0;

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 400

// Microstepping mode. If you hardwired it to save pins, set to the same value here.
#define MICROSTEPS 1

#define DIR 8
#define STEP 9
#define ENABLE 13 // optional (just delete ENABLE from everywhere if not used)


AccelStepper stepper(AccelStepper::DRIVER, STEP, DIR); // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5


void setup() {

  stepper.setMaxSpeed(1000);
  stepper.setSpeed(400);

  Serial.begin(9600);
  //  Serial.println("HX711 calibration sketch");
  //  Serial.println("Remove all weight from scale");
  //  Serial.println("After readings begin, place known weight on scale");

  //  scale.set_scale();
  scale.tare(10); // Reset the scale to 0; Reads value 10 times, calculates average and uses it as offset.

  //  long zero_factor = scale.read_average(); //Get a baseline reading
  //  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  //  Serial.println(zero_factor);
  //
  scale.set_scale(calibration_factor); //Adjust to this calibration factor

  tension_setpoint = 50;

}

void loop() {

  // NOTE: LOOP HAS TO RUN QUICKLY FOR STEPPER TO MOVE SMOOTHLY. DO NOT ADD DELAYS


  if (scale.is_ready()){
    Serial.print("Tension: ");
    tension = scale.get_units_direct();
    Serial.print(tension);
    Serial.print(" grams.");

    double delta_tension = tension - tension_setpoint;

    if (abs(delta_tension) < 10) {
      stop_motor = true;
      Serial.print("Stop! ");
      Serial.println();
    }
    else {
      stop_motor = false;

      dir = (delta_tension > 0) ? 1 : -1;

      stepper.setSpeed(dir * (abs(delta_tension) * Kp + 400));

      Serial.print("RPM: ");
      Serial.print(dir * (abs(delta_tension) * Kp + 400));
      Serial.println();

    }

  }

  if (stop_motor) {
    stepper.setSpeed(0);
  }
  else{
    stepper.runSpeed();
  }

}

