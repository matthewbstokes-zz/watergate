#include <Wire.h>
#include <RTClib.h>

// which pin to use for reading the sensor? can use any pin!
#define PIN_FLOWSENSOR 2
#define PIN_SOLENOID 3
#define PIN_PUMP 4

// count how many pulses!
volatile uint16_t pulses = 0;
// track the state of the pulse pin
volatile uint8_t lastflowpinstate;
// you can try to keep time of how long it is between pulses
volatile uint32_t lastflowratetimer = 0;
// and use that to calculate a flow rate
volatile float flowrate;

RTC_DS1307 RTC;

// Interrupt is called once a millisecond, looks for any pulses from the sensor!
SIGNAL(TIMER0_COMPA_vect) {
  uint8_t x = digitalRead(PIN_FLOWSENSOR);
  
  if (x == lastflowpinstate) {
    lastflowratetimer++;
    return; // nothing changed!
  }
  
  if (x == HIGH) {
    //low to high transition!
    pulses++;
  }
  lastflowpinstate = x;
  flowrate = 1000.0;
  flowrate /= lastflowratetimer;  // in hertz
  lastflowratetimer = 0;
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
  }
}

void setup() {
   Serial.begin(9600);
   Serial.print("Flow sensor test!");
   pinMode(PIN_FLOWSENSOR, INPUT);
   pinMode(PIN_SOLENOID, OUTPUT);
   pinMode(PIN_PUMP, OUTPUT);
   digitalWrite(PIN_FLOWSENSOR, HIGH);
   digitalWrite(PIN_SOLENOID, LOW);
   digitalWrite(PIN_PUMP, LOW);
   lastflowpinstate = digitalRead(PIN_FLOWSENSOR);
   useInterrupt(true);
   
   Wire.begin();
   RTC.begin();
 
   if (! RTC.isrunning()) {
     Serial.println("RTC is NOT running!");
     // following line sets the RTC to the date & time this sketch was compiled
     //RTC.adjust(DateTime(__DATE__, __TIME__));
  }
}

void loop()                     // run over and over again
{ 
  Serial.print("Freq: "); Serial.println(flowrate);
  Serial.print("Pulses: "); Serial.println(pulses, DEC);
  // Sensor Frequency (Hz) = 7.5 * Q (Liters/min)
  // Liters = Q * time elapsed (seconds) / 60 (seconds/minute)
  // Liters = (Frequency (Pulses/second) / 7.5) * time elapsed (seconds) / 60
  // Liters = Pulses / (7.5 * 60)
  float liters = pulses;
  liters /= 7.5;
  liters /= 60.0;
  Serial.print(liters); Serial.println(" Liters");
  
     DateTime now = RTC.now();
 
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
 
    Serial.print(" since 1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");
 
    // calculate a date which is 7 days and 30 seconds into the future
    DateTime future (now.unixtime() + 7 * 86400L + 30);
 
    Serial.print(" now + 7d + 30s: ");
    Serial.print(future.year(), DEC);
    Serial.print('/');
    Serial.print(future.month(), DEC);
    Serial.print('/');
    Serial.print(future.day(), DEC);
    Serial.print(' ');
    Serial.print(future.hour(), DEC);
    Serial.print(':');
    Serial.print(future.minute(), DEC);
    Serial.print(':');
    Serial.print(future.second(), DEC);
    Serial.println();
 
    Serial.println(); 
    delay(1000);
}
