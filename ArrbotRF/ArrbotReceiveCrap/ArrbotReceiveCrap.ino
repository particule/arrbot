
//  Receive data via Nrf24lo1+ so we can test transmitters. 
// This is derived from the program that runs on the Arrbot.
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


void setup()
{
  Serial.begin(9600);
  Serial.println("in da beginning...");

  // Setup pins / SPI.
  Mirf.spi = &MirfHardwareSpi;
  Mirf.cePin = 9;
  Mirf.csnPin = 10;
  Mirf.init();

  // Configure receiving address. Other side must have TADDR "serv1"
  Mirf.setRADDR((byte *)"serv1");

  // Set the payload length.
  // CLIENT AND SERVER MUST AGREE!!
  Mirf.payload = 5 * sizeof(int);

  // Write channel and payload config then power up reciver.
  Mirf.channel = 10;
  Mirf.config();

  Serial.println("Listening..."); 
}

int nsuccess = 0;
int nfail = 0;

//----------------------------------------------------------------------
void loop() 
{

 int joystick[5];


  // If a packet has been received.
  if(!Mirf.isSending() && Mirf.dataReady()){

     // load the packet into the buffer.
     Mirf.getData( (byte *) joystick);

     delay(35);
     Serial.print("= ");
     for (int i=0; i<5; i++) {
       Serial.print(joystick[i]);
       Serial.print(" ");
     }

     nsuccess++;
     Serial.print("Success: ");      
     Serial.print(nsuccess);
     Serial.print(", Fail: ");
     Serial.println(nfail);
 }
 else {
   nfail++;
   if ( (nfail % 10) == 0) {
       Serial.print("Fail: ");
       Serial.print(nfail);
       Serial.print(", success: ");
       Serial.println(nsuccess);
   }
 }


//  delay(20);

}

