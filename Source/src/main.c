#include <8051.h>
//#include "reg51.h"
//#include "intrins.h"
#include "stc8.h"
#define FOSC 11059200UL
// for timer1
#define	T1MS (65536-FOSC/1000)
#define	T100MS (65536-FOSC/10)
#define	T200MS (65536-FOSC/5) /*560ms*/

// my favorite missing variable types
#define byte unsigned char
#define bool __bit


void  delay_ms(unsigned int ms);

// application
#define RTS			P3_4	/* Pin 1 in */
#define DISABLETxD	P3_5	/* Pin 3 out */
#define TxDproc		P3_0	/* Pin 5 in */
#define DISABLEVCC	P3_1	/* Pin 6 out */
#define TxDpc		P3_2	/* Pin 7 in */
#define LED			P3_3	/* Pin 8 uit */

int cnt=0;
volatile bool blink;
volatile bool blinkfast;
/* Timer0 interrupt routine 10Hz */
void tm0_isr() __interrupt 1 __using 1
{
	if(++cnt>2000){
	  blinkfast=!blinkfast;
	  if(blinkfast)blink=!blink; // slow */
	}
}

byte state=0;

// 0	Idle (RTS=1)
// 1	RTS became 0 test for 0x7f @2400Bd if not goto step 3
// 2    Switch off power, wait 500ms Switch on power
// 3	enable TxDpass, wait for RTS=1 & goto step 0
// ?         failsave check for 0x7F every x time    counter and goto step 1
//            0       123      4         0
// RTS        ^^^^^^^|___________________|^^^^^^^
// TxDpc              ?0x7F ?
// VCCon      ^^^^^^^^^|_500ms_|^^^^^^^^^^^^^^^^^^
// TxDpass    _________________|^^^^^^^^^|______
//
// if RTS = 0 too long start again at 2

void main()
{
	short i=0;
	
	bool ok=0;
	short b;

	// init I/O P3.0 .2 .4 inputs, P3.1 .3 .5 output
	P3M0=0x2A;//00101010
	P3M1=0;

	//timer init 10Hz
	AUXR = 0x80; //timer0 work in 1T mode
	TMOD = 0x00; //set timer0 as mode0 (16-bit auto-reload)
	TL0 = T200MS; //initial timer0 low byte
	TH0 = (byte) T200MS >> 8; //initial timer0 high byte
	TR0 = 1; //timer0 start running
	ET0 = 1; //enable timer0 interrupt
	EA = 1; //open global interrupt switch
	state=0;
	LED=1;
	delay_ms(250);
	LED=0;
	while (1){
		switch (state){
			case 0:
			DISABLEVCC=0;//power onw
			DISABLETxD=1;
			LED=1;
			if(!RTS) state++;  /* RTC pin 1*/
			break;

			/* find out program or debug */
			// signal looks like:   ^^^^^|___|^^^^^^^^^^^^^^|___|^^^^^
			//                             415us    2.9ms    415us
			case 1:
			if(RTS)state=0; // go back
			if(!TxDpc)state++; // wait for ^^|__ on TxD pin 7
			break;

			#define BITTIME 190 /* ideal 208us */
			// Check for 0x7f @ 2400Bd
			case 2:			
			// start bit = 0
			LED=1;					// LED used for diagnostic only
			for(i=0;i<BITTIME;i++); // short wait 208us
			LED=0;
			ok=!TxDpc;				// check must be 0
			for(i=0;i<BITTIME;i++); // short wait 200us
			
			// data bit 1-7 = 1    pattern = 0x7f
			for(b=0;b<7;b++){
				LED=1;
				for(i=0;i<BITTIME;i++); // short wait 200us
				if(!TxDpc) ok=0;		// check must be 1
				LED=0;
				for(i=0;i<BITTIME;i++); // short wait 200us
			}
			// last data bit 8 = 0  pattern = 0x7f
			LED=1;
			for(i=0;i<BITTIME;i++); // short wait 200us
			if(TxDpc) ok=0;			// check must be 0
			LED=0;
			for(i=0;i<BITTIME;i++); // short wait 200us
			// stop bit = 1
			LED=1;
			for(i=0;i<BITTIME;i++); // short wait 200us
			if(!TxDpc) ok=0;		// check must be 1
			LED=0;
			for(i=0;i<BITTIME;i++); // short wait 200us
			
			LED=ok;					// diagnose
			if(ok)state++;else state = 4;
			break;

			case 3:
			if(ok)		LED=blinkfast;		else LED=blink;
			DISABLEVCC=1;
			delay_ms(500);
			DISABLEVCC=0;
			state++;
			break;

			case 4:// transfer
			DISABLETxD=0;
			LED=blink;
			if(RTS)state=0;

			//delay_ms(500);state++;
			break;

		}
	}
}


void  delay_ms(unsigned int ms)
{
	unsigned int i;

	if(ms<1)return;	//added to handle 0 no delay. RH
	do{
		i = FOSC / 18000;//19520=922ms;//18732=961ms;//  2*13000;//26000=694ms
		while(--i)    ;   //14T per loop
	}while(--ms);
}
