
//  Transmit ascending numbers via Nrf24lo1+ so we can test receivers.
// This is derived from the program that runs on the Nunchuk.
//
// By Kurt Fleischer 8/21/14, derived from Mirf nrf_test1_tx libraries. 
//http://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo


//  - CONNECTIONS: nRF24L01 Modules See:
//http://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo
//   1 - GND
//   2 - VCC 3.3V !!! NOT 5V
//   3 - CE to Arduino pin 9
//   4 - CSN to Arduino pin 10
//   5 - SCK to Arduino pin 13
//   6 - MOSI to Arduino pin 11
//   7 - MISO to Arduino pin 12
//   8 - UNUSED
//
// if you have a pro micro, the SCK, MOSI and MISO are
// different. See https://learn.sparkfun.com/tutorials/pro-micro--fio-v3-hookup-guide


#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

int joystick[5];  //  array holding Joystick readings


void
setup() {
  Serial.begin(9600);
  Serial.println("Start setup.");
  nunchuk.init2();
  Serial.println("Nunchuk initialized.");
  
  // Setup pins / SPI.
  Mirf.cePin = 9;
  Mirf.csnPin = 10;
  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();
  
  // This string must match TADDR on the other.
  Mirf.setRADDR((byte *)"clie1");
  
  // Set the payload length.
  // CLIENT AND SERVER MUST AGREE!!
  Mirf.payload = 5 * sizeof(int);
   
  // Make sure channel is legal in your area
  Mirf.channel = 10;
  Mirf.config();
  Mirf.setTADDR((byte *)"serv1");

  Serial.println("Mirf initialized.");
}

unsigned long last;  // last time
int x = 0;

void
loop() {
  
  int now = millis();
  unsigned long time = millis();
  Serial.print( now - last );  // print elapsed ms
  last = now;

  joystick[0] = x;
  joystick[1] = x*x;
  joystick[2] = 0;
  joystick[3] = 1;
  joystick[4] = now;

  Mirf.send((byte *)&joystick);
  Serial.print("Send: ");
  Serial.print(joystick[0]);
  Serial.print(" ");
  Serial.println(joystick[1]);
  while(Mirf.isSending()){
//      delay(5);
  }
  Serial.println("Finished sending");

  // reduce this delay once things are debugged.
  delay(50);
}





