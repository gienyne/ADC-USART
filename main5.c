/*
 *
 * Created: 20.12.2024 10:32:51
 * Author : Ntofeu nyatcha dimitry
 */ 

#define F_CPU 4000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define BAUD_RATE 9600

char USART_buffer[64];
volatile uint8_t usart_index = 0;
volatile uint8_t data_received = 0;


void USART_init(){
	float baud = (float)((F_CPU << 6) / (16L * BAUD_RATE));
	USART3.BAUD = (uint16_t)baud;
	USART3.CTRLB = USART_TXEN_bm | USART_RXEN_bm;
	USART3.CTRLC = USART_CHSIZE_8BIT_gc;
	
	 // RX interupt aktivieren
	 USART3.CTRLA |= USART_RXCIE_bm;
}

void USART_Uebertragung(char charakter){
	while(!(USART3.STATUS & USART_DREIF_bm)){}
		USART3.TXDATAL = charakter;
}

void USART_send_String(char *str){
	
	for(size_t i = 0; i < strlen(str); i++){
		USART_Uebertragung(str[i]);
	}
	
}

ISR(USART3_RXC_vect){
	char received = USART3_RXDATAL; // empfangene Daten lesen
	//data_received = 1;
	if(received == '.'){ // Ende von der Nachricht ...BITTE DATEN MIT . BEENDEN
		USART_buffer[usart_index++] = '\0';
		usart_index = 0;
		data_received = 1; // Signalisiert, dass Daten verfügbar sind
		USART_send_String("Ende der Nachricht erkannt\n");
	}
	else if(usart_index < sizeof(USART_buffer) - 1){
		USART_buffer[usart_index++] = received;
	}
	
}

void pwm_init(){
	
	PORTE.DIRSET = PIN0_bm | PIN1_bm | PIN2_bm;
	
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV4_gc | TCA_SINGLE_ENABLE_bm;
	TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc
	                    |TCA_SINGLE_CMP0EN_bm
						|TCA_SINGLE_CMP1EN_bm
						|TCA_SINGLE_CMP2EN_bm;
						
	TCA0.SINGLE.PER = 255; // 8-Bit PWM
	
	PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTE_gc;
}

void set_r_g_b(uint8_t r, uint8_t g, uint8_t b){
	TCA0.SINGLE.CMP0 = r;
	TCA0.SINGLE.CMP1 = g;
	TCA0.SINGLE.CMP2 = b;
}

int main(){
	
	USART_init();
	pwm_init();
	//Set RX as input
	PORTB.DIRCLR = PIN1_bm;
	
	PORTB.DIRSET = PIN0_bm;
	
	sei();
	
	USART_send_String("RGB Control Ready\n");
	
	while(1){
		if(data_received){  // alle daten oder esrte oder eine komplete Nachricht wurde empfangen
			data_received = 0; //für die Nachste Nachtricht auf 0 zurücksetzen
			
		    // RGB-Werte aus der Eingabe extrahieren
		    uint8_t r = 0, g = 0, b = 0;
		    sscanf(USART_buffer, "%hhu,%hhu,%hhu", &r, &g, &b);
			
			 // RGB-Werte setzen
			 set_r_g_b(r, g, b);
            
			 
			 snprintf(USART_buffer, sizeof(USART_buffer), "Set RGB: %d, %d, %d\n", r, g, b);
			USART_send_String(USART_buffer);
			
		
		}
	}
	
}