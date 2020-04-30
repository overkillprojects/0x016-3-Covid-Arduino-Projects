/* Pin setup (could also use define directive) */
int irLED = 8;
int redLED = 6;
int greenLED = 5;
int irDiode = A0;

/* Globals */
int diodeVal = 0;       // Photodiode value
int diodePrevVal = 0;   // Previoius photodiode value
int LEDCounter = 0;     // Keeps time

void setup() {
  /* Turn on Serial port to display on monitor/plotter */
  Serial.begin(115200);

  /** 
   *  The LEDs are outputs,
   *  photodiode is on analog pin, no need to initialize 
   */
  digitalWrite(irLED, LOW);     // Using NPN transistor, off is low
  pinMode(irLED, OUTPUT);
  digitalWrite(redLED, HIGH);   // Using PNP transistor, off is high
  pinMode(redLED, OUTPUT);
  digitalWrite(greenLED, HIGH); // Using PNP transistor, off is high
  pinMode(greenLED, OUTPUT);

  /* disable inteerupts while setting up timer registers */
  noInterrupts();

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

  /**
   *  Timer2 initialization
   *  Counter is 0 (no preload)
   *  Prescaler is 1024
   *  Overflow interrupt is enabled to start (disable on detection)
   */
  TCCR2A = 0;                   // reset control registers
  TCCR2B = 0;

  TCNT2 = 0;                    // set counter to 0
  TCCR2B |= (1 << CS20);        // 1024 prescaler (~16ms to overflow_
  TCCR2B |= (1 << CS21);
  TCCR2B |= (1 << CS22);
  
  TIMSK2 |= (1 << TOIE2);       // enable timer overflow interrupt

  /* enable inteerupts after setting up timer registers */
  interrupts();
}

void loop() {
  /* Funny, nothing to see here! It's all in the timers. */
}

/**
 *  Timer2 overflow interrupt
 *  Triggered every ~16ms
 *  Turns on IR LED and checks for local bounce back (hand detection)
 *  On two consecutive detections (like debounce) turn off Timer2 and
 *  turn on Timer1 for LED blinky ~30 second timer
 */
ISR(TIMER2_OVF_vect)
{
  diodePrevVal = diodeVal;            // Rememebr previous value
  
  digitalWrite(irLED, HIGH);          // Turn on IR LED
  diodeVal = analogRead(irDiode);     // Read from photodiode
  digitalWrite(irLED, LOW);           // Turn off IR LED
  
  Serial.println(diodeVal);           // Print the value if you like
  Serial.println();

  /* Two consecutive detections, switch to blinky timer (Timer1) */
  if ((diodeVal > 90) && (diodePrevVal > 90))
  {
    digitalWrite(redLED, HIGH);       // Turn on Red LED for first blink
    TIMSK2 &= ~(1 << TOIE2);          // disable timer2 overflow interrupt
    TIMSK1 |= (1 << TOIE1);           // enable timer1 overflow interrupt
  }
}

/**
 *  Timer1 overflow interrupt
 *  Triggered every ~262ms which is close enough to 250ms.
 *  Blink Red LED for ~20s
 *  Then blink Yellow LED for ~10s
 *  Then blink Green LED for ~10s
 *  Finally turn off Timer1 and re-enable Timer2
 *  
 *  On my arduino I get a nearly exactly 30s timer, but it will 
 *  vary from unit to unit since the clocks aren't very accurate.
 *  To improve, would need to use a good watch crystal (32.768kHz)
 */
ISR(TIMER1_OVF_vect)
{
  LEDCounter++;
  if (LEDCounter % 2)
  {
    /* Could use case/switch */
    if (LEDCounter <= 80)             // 80 / 4 = 20s
    {
      /* toggle red LED */
      digitalWrite(redLED, !digitalRead(redLED));
    }
    else if (LEDCounter <= 120)       // (120-80)/4 = 10s
    {
      /* toggle red and green LED to get yellow-ish */
      digitalWrite(redLED, !digitalRead(redLED));
      digitalWrite(greenLED, digitalRead(redLED));
    }
    else if (LEDCounter <= 160)       // (160-120)/4 = 10s
    {
      /* toggle green LED, keep red LED off */
      digitalWrite(redLED, HIGH);
      digitalWrite(greenLED, !digitalRead(greenLED));
    }
    else
    {
      /* turn off green LED */
      digitalWrite(greenLED, HIGH);

      /* reset counters and return to starting state */
      TCNT1 = 0;
      TCNT2 = 0;
      
      TIMSK1 &= ~(1 << TOIE1);   // disable timer overflow interrupt
      TIMSK2 |= (1 << TOIE2);   // enable timer overflow interrupt
      
      LEDCounter = 0;
      diodeVal = 0;
      diodePrevVal = 0;
    }
  }
}
