
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
#define ADC_MAX_STUFE 4095
#define SIZE 7

// Umwandlung von Integer in String
char* int_to_string(uint16_t number, char* zeichenkette) {
	int position = 0;

	if (number == 0) {
		zeichenkette[position++] = '0';
	}

	while (number > 0) {
		zeichenkette[position++] = '0' + number % 10;
		number /= 10;
	}

	zeichenkette[position] = '\0'; // String-Terminator

	for (int i = 0; i < position / 2; i++) {
		char contener = zeichenkette[i];
		zeichenkette[i] = zeichenkette[position - 1 - i];
		zeichenkette[position - 1 - i] = contener;
	}
	return zeichenkette;
}

void ADC_init(){
	VREF.ADC0REF = VREF_REFSEL_VDD_gc; 
	ADC0.MUXPOS = ADC_MUXPOS_AIN18_gc;
	ADC0.CTRLA = ADC_ENABLE_bm | ADC_RESSEL_12BIT_gc;
	ADC0.CTRLB = ADC_SAMPNUM_NONE_gc;
	ADC0.CTRLC = ADC_PRESC_DIV16_gc;
}

// Funktion zur ADC-Messung
uint16_t ADC_Wandlung(void) {
	ADC0.COMMAND = ADC_STCONV_bm;               // Start der Konvertierung
	while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));   // Warten, bis die Konvertierung fertig ist
	ADC0.INTFLAGS = ADC_RESRDY_bm;              // Interrupt-Flag löschen
	return ADC0.RES;                            // Ergebnis zurückgeben
}


int main(void) {
	char spannung_string[SIZE];
	char prozent_string[SIZE];

	// Initialisierungen
	ADC_init();
	lcd_init();
	lcd_enable(true);

	// Kalibrierung: Maximalwert für 100 %
	uint16_t adc_max_wert = 0; // Kalibrierung durch maximale Helligkeit mit Lampe

	while (1) {
		
		uint16_t ADC_Wert = ADC_Wandlung();

	
		if (adc_max_wert < ADC_Wert) {
			adc_max_wert = ADC_Wert;
		}

	
		uint16_t spannung = (uint16_t)((ADC_Wert * REF_SPANNUNG * 1000) / ADC_MAX_STUFE); // in mV
		uint16_t prozent = (uint16_t)((ADC_Wert * 100) / adc_max_wert);                   // in %

	
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