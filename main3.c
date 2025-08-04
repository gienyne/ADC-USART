/*
 *
 * Created: 20.12.2024 10:32:51
 * Author : Ntofeu nyatcha dimitry
 */ 

#define F_CPU 4000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#define BAUD_RATE 9600

volatile uint8_t counter_4 = 0;
volatile uint8_t counter_5 = 0;
volatile uint8_t counter_6 = 0;
volatile uint8_t counter_7 = 0;

void USART_init(){
	
	float baud = (float)((F_CPU << 6) / (16L * BAUD_RATE));
	USART3.BAUD = baud; // baudrate setzen
	USART3.CTRLB |= (USART_TXEN_bm | USART_RXEN_bm); // TX und RX aktivieren
	USART3.CTRLC = USART_CHSIZE_8BIT_gc; // Datenformat( 8-bit )
}

void USART_Uebertragung(char caracter){
	
	while(!(USART3.STATUS & USART_DREIF_bm)){}// Warten bis das Register leer ist
	USART3.TXDATAL = caracter; // Charakter senden
}

ISR(PORTC_PORT_vect)
{
	
	if (PORTC.INTFLAGS & PIN4_bm) {
		if (PORTC.IN & PIN4_bm) {  
			
			counter_4++;
		}
		PORTC.INTFLAGS = PIN4_bm; 
	}

	
	if (PORTC.INTFLAGS & PIN5_bm) {
		if (PORTC.IN & PIN5_bm) { 
			
			counter_5++;
		}
		PORTC.INTFLAGS = PIN5_bm;  
	}

	
	if (PORTC.INTFLAGS & PIN6_bm) {
		if (PORTC.IN & PIN6_bm) { 
			
			counter_6++;
		}
		PORTC.INTFLAGS = PIN6_bm;  
	}

	if (PORTC.INTFLAGS & PIN7_bm) {
		if (PORTC.IN & PIN7_bm) {  
			
			counter_7++;
		}
		PORTC.INTFLAGS = PIN7_bm;  
	}
}

int main(void){
	
	PORTC.DIRCLR = PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm; //Pins C4 bis C7 als Eingänge mit Pull-up-Widerständen
	PORTC.PIN4CTRL = PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	PORTC.PIN5CTRL = PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	PORTC.PIN6CTRL = PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	PORTC.PIN7CTRL = PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	
	PORTF.DIRSET = PIN4_bm;
	//PORTB.OUTSET = PIN0_bm;
	USART_init();
	
	sei();

	
	while(1){
		if (counter_4 > 0) {
			USART_Uebertragung('K');
			counter_4--;
		}
		if (counter_5 > 0) {
			USART_Uebertragung('A');
			counter_5--; 
		}
		if (counter_6 > 0) {
			USART_Uebertragung('M');
			counter_6--; 
		}
		if (counter_7 > 0) {
			USART_Uebertragung('U');
			counter_7--; 
		}
	}
	
}
