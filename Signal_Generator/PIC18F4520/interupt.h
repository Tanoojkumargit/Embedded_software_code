#include<p18f4520.h> // Device selected
#pragma config OSC = HS //set osc mode to HS
#pragma config WDT=OFF //Watch Dog Timer disabled
#pragma config LVP=OFF //Low Voltage Programming option OFF
#pragma config MCLRE = ON //Master Clear Pin ON
// Include the folowing headers
#include <p18f4520.h>
#include<timers.h>// Device used is the PICF4520
#include <stdlib.h> // Include standard Library
#include <delays.h> // Include Delays
#define CS PORTBbits.RB4 // Port bit B0 is used as the Chip Select pin CS
void DAC(unsigned int);
void triangle_wave();
void ramp_up();
void ramp_down();
void sine();
unsigned int a,b,j;
const unsigned char sin_wave[100]={127,135,143,151,159,166,174,181,195,202,208,214,220,225,230,234,238,242,245,248,250,252,253,254,254,253,252,250,248,245,242,238,234,230,225,220,214,208,202,195,188,1811,174,16,159,151,143,135,127,119,111,103,95,88,80,73,66,59,52,46,40,34,29,24,20,16,12,9,6,4,2,1,0,0,0,1,2,4,6,9,12,16,20,24,29,34,40,46,52,59,66,73,80,88,95,103,111,119};

void main()
{
TRISB=0; // PORTB is configured as an output port for CS
TRISC=0; // PORTC is configured as an output port for SPI
PORTC=0; // Initialise Ports
PORTB=0; // ""
SSPSTAT=0xC0; //Status Register SSPSTAT=11000000
SSPCON1=0x20; //Enables serial port pins & set the SPI clock as clock = FOSC/4
	//triangle_wave(); 
 OpenTimer0( // Open Timer0 with the following settings
 TIMER_INT_OFF & // Disable Timer0 interrupts
 T0_16BIT & // 16-bit configuration of the timer
 T0_SOURCE_INT & // use the internal clock
 T0_PS_1_256 // divide the internal 5MHz clock by 256
 );
 
 while (1)
	 
     {	 
	sine();
	  }
	
}
void DAC(unsigned int data)
{
 unsigned int c ;
 unsigned int lower_bits;
 unsigned int upper_bits;
 c = ((data+1)*16) -1; // here we obtain 12 bit data //first obtain the upper 8 bits
 upper_bits = c/256; // obtain the upper 4 bits
 upper_bits = (48) | upper_bits; // append 0011 to the above 4 bits
 //now obtain the lower 8 bits
 lower_bits = 255 & c; // ANDing separates the lower 8 bits
 CS=0; // Chip Select RB0 Low
 SSPBUF=upper_bits; // sending the upper 8 bits first serially
 while(!SSPSTATbits.BF); // wait until the upper 8 bits are sent by checking BF flag
 SSPBUF=lower_bits; // sending the lower 8 bits serially
 while(!SSPSTATbits.BF); // wait until the lower 8 bits are sent
 CS=1; // Chip Select ONE
} 

void triangle_wave()
{
     for(a=0; a<255;a++)
		 
	 DAC(a); // 0V
	 
	
	 for(b=255; b>0; b--)
     DAC(b);
}
void ramp_down()
{
	 DAC(255); // 0V
	 for(b=255; b>0; b--)
     DAC(0);
 }
 
 void ramp_up()
 
 {
	
	 for(a=0; a<255;a++)	 
     DAC(a);
	  DAC(0);
    //Delay1KTCYx(10); // Delay
 }
 
 void sine()
 {
	 j++;
      if (j >= 180)
      {
         j = 0;
      }
      a = sin_wave[j];
      DAC(a);
  }
  

 