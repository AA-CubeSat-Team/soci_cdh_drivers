// Written by Nick Gammon
// February 2011


#include <SPI.h>

uint8_t buf [100];
volatile byte pos;
volatile boolean process_it;

void setup (void)
{
  Serial.begin (115200);   // debugging
  SPI.setClockDivider(SPI_CLOCK_DIV32);

  // turn on SPI in slave mode
  SPCR |= bit (SPE);

  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);
  
  // get ready for an interrupt 
  pos = 0;   // buffer empty
  process_it = false;

  // now turn on interrupts
  SPI.attachInterrupt();
  Serial.println ("Slave setup");

}  // end of setup


// SPI interrupt routine
ISR (SPI_STC_vect)
{
  uint8_t c = SPDR;  // grab byte from SPI Data Register: 00, 01, 02...
  SPDR = ~SPDR; //returns the complement of the bytes: FF, FE, FD...
  buf [pos++] = c;
}  // end of interrupt routine SPI_STC_vect

// main loop - wait for flag set in interrupt routine
void loop (void)
{
  if (pos == 7)
    {
      for(int i=0; i<pos; i++){
        Serial.print (buf [i], HEX);
        Serial.print (" ");
      }
      Serial.println ("\n");

      process_it = false;
      pos = 0;
    
    }  // end of flag set
    
}  // end of loop
