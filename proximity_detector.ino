/* Pin setup (could also use define directive) */
int buzzer = 8;
int echo = 4;
int trig = 3;

/* Globals */
uint32_t duration = 0;
uint32_t distance = 0;

void setup() {
  /* Turn on Serial port to display on monitor/plotter */
  Serial.begin(115200);

  /* buzzer and trigger are outputs */
  digitalWrite(buzzer, LOW);
  pinMode(buzzer, OUTPUT);
  digitalWrite(trig, LOW);
  pinMode(trig, OUTPUT);

  /* echo is input */
  pinMode(echo, INPUT);
}

void loop() {
  /* Set the trigger high for 10 microseconds to get a measurement */
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  
  /** 
   *  Read the pulse duration from the echo pin:
   *  This tells how long it took the sound wave
   *  to come back to the transceiver.
   */
  duration = pulseIn(echo, HIGH);
  /** 
   *  Calculate the distance:
   *  This is the speed of the sound wave divided
   *  by two since it goes out and back.
   */
  distance = duration * 0.034 / 2;

  /* We can display the distance on the serial monitor/plotter */
  Serial.print(distance);
  Serial.println(" cm");

  /* Buzz if they are too close!! */
  if (distance < 200) digitalWrite(buzzer, HIGH);
  else digitalWrite(buzzer, LOW);
  
  delay(50);    // probably should replace with a timer
}
