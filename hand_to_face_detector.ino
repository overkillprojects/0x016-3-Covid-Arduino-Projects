#include "SparkFunLSM6DS3.h"
#include <Wire.h>

LSM6DS3 myIMU; // The default constructor is I2C, addr 0x6B

int redLED = 7;
int timerCount = 0;
float zAccel = 0;
float zGyro = 0;

void setup() {
  /* Turn on Serial port to display on monitor/plotter */
  Serial.begin(115200);

  /* Red LED/buzzer is output */
  digitalWrite(redLED, LOW);
  pinMode(redLED, OUTPUT);

  /* The begin method configures the IMU */
  myIMU.begin();

  /**
   *  Timer1 initialization
   *  Counter is 0 (no preload)
   *  Prescaler is 64
   *  Overflow interrupt is disabled to start (enable on detection)
   */
  TCCR1A = 0;                   // reset control registers
  TCCR1B = 0;

  TCNT1 = 0;                    // set counter to 0
  TCCR1B |= (1 << CS11);        // 64 prescaler (~262ms to overflow, close enough to 250)
  TCCR1B |= (1 << CS10);
  
  TIMSK1 &= ~(1 << TOIE1);      // disable timer overflow interrupt
}

void loop() {
  /** 
   *  The readFloatAccelZ method returns the
   *  z-axis linear acceleration as a floating
   *  point value, where 1 equals 1g = 9.8m/s^2.
   *  So a value near 0 means the z-axis is nearly
   *  parallel to the horizon/floor.
   */
  zAccel = myIMU.readFloatAccelZ();
  zAccel = zAccel < 0 ? -1 * zAccel : zAccel;

  /**
   *  The readFloatGyroZ method returns the
   *  rotational acceleration about the z-axis.
   *  These values can be higher and depend on the
   *  gyroRange setting. See the library code or 
   *  SparkFun site for more info.
   */
  zGyro = myIMU.readFloatGyroZ();

  /**
   *  If the z-axis is nearly horizontal and the z-axis
   *  rotational acceleration is high, we might be going
   *  for our face!!
   */
  if (zAccel < 0.4 && zGyro > 120)
  {
    digitalWrite(redLED, HIGH);   // WARNING!
    TIMSK1 |= (1 << TOIE1);       // enable timer overflow interrupt
  }
  
  delay(200);
}

/**
 *  Timer1 will overflow every ~250ms, so let it
 *  run 8 times (~2s), then turn off red LED/buzzer.
 */
ISR(TIMER1_OVF_vect)
{
  timerCount++;
  if (timerCount == 8)
  {
    timerCount = 0;
    TIMSK1 &= ~(1 << TOIE1);      // disable timer overflow interrupt
    TCNT1 = 0;
    digitalWrite(redLED, LOW);
  }
}
