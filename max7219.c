
#include "io430.h"
#include "stdint.h"


// Port1 USCI pins and CS/Load pin
#define SPI_SIMO	BIT2
#define SPI_CLK		BIT4
#define SPI_CS		BIT3	// Load or /CS

// MAX7219 Register addresses
#define MAX_NOOP	0x00
#define MAX_DIGIT0	0x01
#define MAX_DIGIT1	0x02
#define MAX_DIGIT2	0x04
#define MAX_DIGIT3	0x08
#define MAX_DIGIT4	0x10
#define MAX_DIGIT5	0x20
#define MAX_DIGIT6	0x40
#define MAX_DIGIT7	0x80
#define MAX_DECODEMODE	0x09
#define MAX_INTENSITY	0x0A
#define MAX_SCANLIMIT	0x0B
#define MAX_SHUTDOWN	0x0C
#define MAX_DISPLAYTEST	0x0F


// Function prototypes
void spi_init();
void spi_max(unsigned char address, unsigned char data);



const uint8_t number[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90
	
};
const uint8_t disp[] = {MAX_DIGIT0, MAX_DIGIT1, MAX_DIGIT2, MAX_DIGIT3, MAX_DIGIT4, MAX_DIGIT5, MAX_DIGIT6, MAX_DIGIT7
};

// Program start
int main(void)
{
	WDTCTL = WDTPW + WDTHOLD; 	// Disable WDT
	DCOCTL = CALDCO_1MHZ; 		// 1 Mhz DCO
	BCSCTL1 = CALBC1_1MHZ;

	// Setup Port1 pins
	P1DIR |= SPI_SIMO + SPI_CLK + SPI_CS;
	#ifdef USE_MAX7219
	P1OUT |= SPI_CS;		// MAX7219 Chip Select is inactive high
	#endif

	spi_init();			// Init USCI in SPI mode

	// Initialise MAX7219 with 8x8 led matrix
	spi_max(MAX_NOOP, 0x00); 	// NO OP (seems needed after power on)
        spi_max(MAX_SCANLIMIT, 0x07);
	spi_max(MAX_INTENSITY, 0x0F); 	// Display intensity (0x00 to 0x0F)
	spi_max(MAX_DECODEMODE, 0);	
	// Clear all rows/digits
	spi_max(MAX_DIGIT0, 0);
	spi_max(MAX_DIGIT1, 0);
	spi_max(MAX_DIGIT2, 0);
	spi_max(MAX_DIGIT3, 0);
	spi_max(MAX_DIGIT4, 0);
	spi_max(MAX_DIGIT5, 0);
	spi_max(MAX_DIGIT6, 0);
	spi_max(MAX_DIGIT7, 0);
	spi_max(MAX_SHUTDOWN, 1); 	// Wake oscillators/display up
        

	// Ready to start displaying something!

	// Some vars
	uint8_t i, segment;

	// Loop forever
	while (1) 
	{
		
          for(segment=0;segment<8;segment++){
            for(i=0;i<10;i++){
              spi_max(disp[segment], number[i]);
			
			__delay_cycles(300000);	// Wait a bit before showing next frame   
                        
            }
             
          }
	}

}



void spi_max(uint8_t address, uint8_t data)
{

        UCA0TXBUF = address ;	
	while (UCA0STAT & UCBUSY);		// Wait until done
	UCA0TXBUF = data;			// Send byte of data
	while (UCA0STAT & UCBUSY);
	P1OUT |= SPI_CS;			// /CS inactive or Load high
#ifndef USE_MAX7219
	P1OUT &= ~(SPI_CS);			// MAX7219 pulses Load high
#endif
}


// Enable harware SPI
void spi_init()
{
	UCA0CTL1 |= UCSWRST; 		// USCI in Reset State (for config)
	// Leading edge + MSB first + Master + Sync mode (spi)
	UCA0CTL0 = UCCKPH + UCMSB + UCMST + UCSYNC;
	UCA0CTL1 |= UCSSEL_2; 		// SMCLK as clock source
	UCA0BR0 |= 0x01; 		// SPI speed (same as SMCLK)
	UCA0BR1 = 0;
	P1SEL |= SPI_SIMO + SPI_CLK;	// Set port pins for USCI
	P1SEL2 |= SPI_SIMO + SPI_CLK;
	UCA0CTL1 &= ~UCSWRST; 		// Clear USCI Reset State
}