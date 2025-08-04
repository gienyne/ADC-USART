/*
 *
 * Created: 20.12.2024 10:32:51
 * Author : Ntofeu nyatcha dimitry
 */ 

#define F_CPU 4000000UL
#include <avr/io.h>
#include "AVR128DB48_I2C.h"
#include "I2C_LCD.h"
#include <util/delay.h>

#define REF_SPANNUNG 3.3
#define ADC_MAX_STUFE 4095  // 2^N - 1 = 4095 mit N (bit-auflösung) = 12
#define SIZE 7


char* int_to_string(uint16_t number, char* zeichenkette) {
	int position = 0;

	if (number == 0) {
		zeichenkette[position++] = '0';
	}

	while (number > 0) {
		zeichenkette[position++] = '0' + number % 10;
		number /= 10;
	}

	zeichenkette[position] = '\0'; // string terminator

	for (int i = 0; i < position / 2; i++) {
		char contener = zeichenkette[i];
		zeichenkette[i] = zeichenkette[position - 1 - i];
		zeichenkette[position - 1 - i] = contener;
	}
	return zeichenkette;
}


void ADC_init() {
	VREF.ADC0REF = VREF_REFSEL_VDD_gc;          // Référenzspannung = VDD
	ADC0.MUXPOS = ADC_MUXPOS_AIN19_gc;           // chanel AIN3 (PF3) Potentiometer = AIN19
	ADC0.CTRLA = ADC_ENABLE_bm | ADC_RESSEL_12BIT_gc; // //adc aktivieren und  Auflösung auf 12-bit Setzen
	ADC0.CTRLB = ADC_SAMPNUM_NONE_gc;           // //keine Akkumulation
	ADC0.CTRLC = ADC_PRESC_DIV16_gc;             // Prescaler 
}

uint16_t ADC_Wandlung(void) {
	ADC0.COMMAND = ADC_STCONV_bm;               // Convertion starten
	while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));   // Warten bis die Konvertion beendet ist
	ADC0.INTFLAGS = ADC_RESRDY_bm;              //Interrupt Flag löschen
	return ADC0.RES;                            //  Ergebnis von ADC lesen
}

int main(void) {
	char spannung_string[SIZE];
	char prozent_string[SIZE];

	
	ADC_init();
	lcd_init();
	lcd_enable(true);

	while (1) {
		// Wert von ADC lesen
		uint16_t ADC_Wert = ADC_Wandlung();
		
		
		uint16_t spannung = (uint16_t)((ADC_Wert * REF_SPANNUNG * 100) / ADC_MAX_STUFE); // en mV
		uint16_t prozent = (uint16_t)((ADC_Wert * 100) / ADC_MAX_STUFE);                // en %


		lcd_clear();

		lcd_moveCursor(0, 0);
		lcd_putString("Spannung: ");
		lcd_putString(int_to_string(spannung, spannung_string));
		lcd_putString(" mV");

		lcd_moveCursor(0, 1);
		lcd_putString("Prozent: ");
		lcd_putString(int_to_string(prozent, prozent_string));
		lcd_putString(" %");

		_delay_ms(500);
	}
}
