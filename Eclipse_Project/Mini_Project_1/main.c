/*
 * main.c
 *
 *  Created on: May 11, 2021
 *      Author: Mohamed Taha
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/*******************************************
 * 			GLOBAL VARIABLES			   *
 *******************************************/
unsigned char g_sec = 0 ;
unsigned char g_min = 0 ;
unsigned char g_hour = 0 ;

/*******************************************
 * 				EXTERNAL INT0 			   *
 *******************************************/
/* External INT0 ISR */ /* Reset Stop Watch */
ISR(INT0_vect)
{
	TCNT1 = 0; // Timer Counter Register
	/* Reset all counting value*/
	g_sec = 0;
	g_min = 0;
	g_hour = 0;
	GIFR  |= (1<<INTF0); // Clear MIF (Interrupt Flag)
}

/* External INT0 enable and configuration function */
void INT0_Init(void)
{
	DDRD  &= ~(1<<PD2); //Configure PD2 as input Port (Switch)
	PORTD |=  (1<<PD2); //Enable pull up resistor
	/* Configure MCU Control Register to Control INT0 to work with Falling Edge */
	MCUCR |=  (1<<ISC01);
	MCUCR &= ~(1<<ISC00);
	GICR  |=  (1<<INT0); //Set Module Interrupt Enable (MIE)
	SREG  |=  (1<<7); //Enable Global Interrupt (I-bit)
}

/*******************************************
 * 				EXTERNAL INT1 			   *
 *******************************************/
/* External INT1 ISR */ /* Pause Stop Watch */
ISR(INT1_vect)
{
	/* Configure Timer control register to choose No clock source to stop counting */
	TCCR1B &= ~(1<<CS12) & ~(1<<CS11) & ~(1<<CS10);
	GIFR   |= (1<<INTF1); // Clear MIF (Interrupt Flag)
}

/* External INT1 enable and configuration function */
void INT1_Init(void)
{
	DDRD  &= ~(1<<PD3); //Configure PD3 as input Port (Switch)
	/*Configure MCU Control Register to Control INT1 to work with Rising Edge */
	MCUCR |=  (1<<ISC11) | (1<<ISC10);
	GICR  |=  (1<<INT1); //Set Module Interrupt Enable (MIE)
	SREG  |=  (1<<7); //Enable Global Interrupt (I-bit)
}

/*******************************************
 * 				EXTERNAL INT2 			   *
 *******************************************/
/* External INT2 ISR */ /* Resume Stop Watch */
ISR(INT2_vect)
{
	/* Configure Timer control register to choose clock source again to resume counting */
	TCCR1B  = (1<<WGM12)  | (1<<CS12) | (1<<CS10);
	GIFR  |= (1<<INTF2); // Clear MIF (Interrupt Flag)
}

/* External INT2 enable and configuration function */
void INT2_Init(void)
{
	DDRB  &= ~(1<<PB2); //Configure PB2 as input Port (Switch)
	PORTB |=  (1<<PB2); //Enable pull up resistor
	/* Configure MCU Control Register to Control INT2 to work with Falling Edge */
	MCUCSR &= ~(1<<ISC2);
	GICR  |=  (1<<INT2); //Set Module Interrupt Enable (MIE)
	SREG  |=  (1<<7); //Enable Global Interrupt (I-bit)
}

/*******************************************
 * 				TIMER1 CTCA   			   *
 *******************************************/
/* Timer1 CTC_A ISR */
ISR(TIMER1_COMPA_vect)
{
	/* when i enter the ISR that means i got 1 sec from timer */
	g_sec++; //increment sec's
	if(g_sec == 60)
	{
		g_sec = 0;
		g_min++;
	}
	if(g_min == 60)
	{
		g_sec = 0;
		g_min = 0;
		g_hour++;
	}
	if(g_hour == 24)
	{
		g_sec = 0;
		g_min = 0;
		g_hour = 0;
	}
	TIFR  |= (1<<OCF0); // Clear MIF (Interrupt Flag)
}

/* Timer1 CTC_A enable and configuration function */
void Timer1_CTC_A_Init(void)
{
	/*Configure TCCR1A and TCCR1B Register to Control timer1 to work on CTC_A mode with pre-scaler 1024 */
	TCCR1A  = (1<<FOC1A) ;
	TCCR1B  = (1<<WGM12)  | (1<<CS12) | (1<<CS10);

	TCNT1 = 0; // Timer Counter Register
	/* F_CPU = 1000000 , Pre-scaler  = 1024 , F_Timer = F_CPU/Pre-scaler = 976(ticks per second) */
	OCR1A = 976 ; // Output Compare Register 1 A

	TIMSK |= (1<<OCIE1A); //Enable Timer1 CTC A Interrupt enable (MIE)
	SREG  |= (1<<7); //Enable Global Interrupt (I-bit)
}

int main(void)
{
	/* Initialization Part*/

	/*******************
	 *    Interrupts   *
	 *******************/
	INT0_Init(); //EXT INT0 Enable
	INT1_Init(); //EXT INT1 Enable
	INT2_Init(); //EXT INT2 Enable
	Timer1_CTC_A_Init(); //Timer1 CTC_A Enable

	/*******************
	 *    7 Segment    *
	 *******************/
	/* 7 Segment decoder is connected to PC0 -> PC4 */
	/* Set direction of PINS as output -> (DDR = '1') */
	DDRC |= 0x0F;
	/* Put initial value in the PIN (0) , for only the first 4 bits */
	PORTC &= (0xF0) ;
	/* Configure PORTA Direction (PA0) -> (PA5) to be output to control 7 segments */
	DDRA |= (1<<PA0) | (1<<PA1) | (1<<PA2) | (1<<PA3) | (1<<PA4) | (1<<PA5) ;

	/* End of Initialization */

	while(1)
	{
		/* Application */

		/* For the multiplexing of the 7 segments , we have to loop on all the segments */
		PORTA = (1<<PA0); //SEC1
		PORTC = (g_sec % 10) ; // take first digit
		_delay_ms(5);
		PORTA = (1<<PA1); //SEC2
		PORTC = g_sec / 10 ; // take second digit
		_delay_ms(5);
		PORTA = (1<<PA2); //MIN1
		PORTC = g_min % 10 ; // take first digit
		_delay_ms(5);
		PORTA = (1<<PA3); //MIN2
		PORTC = g_min / 10 ; // take second digit
		_delay_ms(5);
		PORTA = (1<<PA4); //HOUR1
		PORTC = g_hour % 10 ; // take first digit
		_delay_ms(5);
		PORTA = (1<<PA5); //HOUR2
		PORTC = g_hour / 10 ; // take second digit
		_delay_ms(5);

	}
}

