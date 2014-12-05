//----------------------------------------------------------------------
// 8/16/14 -  First light using Mirf radio control for arrbot
//  Kurt Fleischer
//
// Modified from ServoTester and Mirf/ping_server
//
// 8/21/14 Added fail counter, and halt servos if 
//   communication is flakey.
//----------------------------------------------------------------------


/**
* An Mirf receiver for nunchuk commands.
*
* Pins:
* Hardware SPI:
* MISO -> 12
* MOSI -> 11
* SCK -> 13
*
* Configurable:
* CE -> 9
* CSN -> 10
*
*/

#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>


// Stuff for the servos
#include <Servo.h> 

#define LBK 1050    // lowest servo value we can safely set
#define LHALT 1370  // servo value to hold still
#define LFWD  1700    // highest servo value we can safely set
#define RBK 1650    // lowest servo value we can safely set
#define RHALT 1370  // servo value to hold still
#define RFWD  1050    // highest servo value we can safely set

// #define DBG

#define BEEP
#ifdef BEEP
int speakerPin = 6;
#endif
int ledPin = 2;
int led = LOW;

//----------------------------------------------------------------------
// globals
//----------------------------------------------------------------------

Servo lserv;                 // left wheel servo object
Servo rserv;                 // right wheel servo object

int left = LHALT;
int right = RHALT;

//----------------------------------------------------------------------
void setup() {

  Serial.begin(9600);
  Serial.println("in da beginning...");

  left = LHALT; right = RHALT;         // halt

  // Setup pins / SPI.
  Mirf.spi = &MirfHardwareSpi;
  Mirf.cePin = 9;
  Mirf.csnPin = 10;
  Mirf.init();

  // Configure receiving address.
  Mirf.setRADDR((byte *)"serv1");

 /* Set the payload length to sizeof(unsigned long) the
  * return type of millis().
  *
  * NB: payload on client and server must be the same.
  */
  Mirf.payload = 5 * sizeof(int);

 // Write channel and payload config then power up reciver.
 Mirf.channel = 10;
 Mirf.config();

#define SERVO 1
#if SERVO
  // See "BUG" note above.
  lserv.attach(7);
  rserv.attach(8);
#endif

#if SERVO
  Serial.println("Halting");
  // Once RF bugs are worked out, we want to do this to start off halted.
  lserv.writeMicroseconds(left);
  rserv.writeMicroseconds(right);
#endif

#ifdef BEEP
  pinMode(speakerPin, OUTPUT);
#endif
  pinMode(ledPin, OUTPUT);     

  Serial.println("Listening..."); 
}

int nsuccess = 0;
int nfail = 0;
int failSinceLastSuccess = 0;


#ifdef BEEP

void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(tone);
  }
}

void playNote(char note, int duration) {
  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
  int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };
  
  // play the tone corresponding to the note name
  for (int i = 0; i < 8; i++) {
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
}

void playBeep(char note1, int duration1,
              int pauseDuration,
              char note2, int duration2)
{
  playNote(note1, duration1);
  delay(pauseDuration);   // pause between notes
  playNote(note2, duration2);
}

void playCharge()
{
  int d1 = 50, d2 = 150, d3 = 400;
  int shortPause = 20;
  playNote('c', d1);
  delay(shortPause);   // pause between notes
  playNote('f', d1);
  delay(shortPause);   // pause between notes
  playNote('a', d1);
  delay(shortPause);   // pause between notes
  playNote('C', d2);
  delay(220);   // pause between notes
  playNote('a', d1);
  delay(shortPause);   // pause between notes
  playNote('C', d3);
  delay(100);   // pause between notes

}
#endif

long lastbeep = 0;

//----------------------------------------------------------------------
void loop() 
{
 int oldleft = left;
 int oldright = right;
 int joystick[5];

 // if signal is lost for too long, we halt:
 if ( failSinceLastSuccess > 1000) {
     left = LHALT; right = RHALT;
 }

  // If a packet has been received.
 int delayt = 0;  // No delay needed!
 if(!Mirf.isSending() && Mirf.dataReady()){
    failSinceLastSuccess = 0;

    // load the packet into the buffer.
     Mirf.getData( (byte *) joystick);

     delay(delayt);
#ifdef DBG
     Serial.print("= ");
     for (int i=0; i<5; i++) {
       Serial.print(joystick[i]);
       Serial.print(" ");
     }
#endif

#ifdef BEEP
     // Avoid multiple beeps per button push w/time lockout.
     long now = millis();
     if (joystick[2] and ((now - lastbeep) > 600)) {
       playBeep('g', 50, 50, 'g', 200);
       lastbeep = now;
     }
     else if (joystick[3] and ((now - lastbeep) > 1500)) {
       playCharge();
       lastbeep = now;
     } else
#endif
     // Joystick is sending bad data
     if (joystick[0] < 0 or joystick[1] < 0
       or joystick[0] > 255 or joystick[1] > 255) {
         Serial.println("Skipping out of range joystick data.");
         left = LHALT; right = RHALT;
     }

     // CRUDE MAPPING from joystick to servos.
     else if (joystick[0] < 30) {
       Serial.println("TURN LEFT");
         left = LBK; right = RFWD;     // left
        // try to keep left tread from coming off
         left = (3*LHALT + LBK)/4;
     }
     else if (joystick[0] > 200) {
       Serial.println(">>right>>");
       left = LFWD; right = RBK;     // right
        // try to keep right tread from coming off
        right = (3*RHALT + RBK)/4;
     }
     else if (joystick[1] < 50) {
        left = LBK; right = RBK;   // reverse
     }
     else if (joystick[1] > 200) {
       left = LFWD; right = RFWD;
     }
     else {
       left = LHALT; right = RHALT;
     }
     
     nsuccess++;
     // Blink LED every several successful receives
     if ( (nsuccess % 5) == 0) {
       led = (led == HIGH) ? LOW : HIGH;
       Serial.print("my led = ");
       Serial.println(led);
       digitalWrite(ledPin, led);
     }

#ifdef DBG
     Serial.print("\tl ");
     Serial.print(left);
     Serial.print(",\tr ");
     Serial.println(right);
     Serial.print("Success: ");      
     Serial.print(nsuccess);
     Serial.print(", Fail: ");
     Serial.println(nfail);
#endif
 }
 else {
   failSinceLastSuccess++;
   nfail++;
   if ( (nfail % 10) == 0) {
#ifdef DBG
       Serial.print("Fail: ");
       Serial.print(nfail);
       Serial.print(", success: ");
       Serial.println(nsuccess);
#endif
   }
 }
#if SERVO
  // See "BUG" note above.
  if (oldleft != left) {
    lserv.writeMicroseconds(left);
  }
#endif


#if SERVO
  if (oldright != right) {
    rserv.writeMicroseconds(right);
  }
#endif
//  delay(20);

}
