/*
 *
 * Created: 20.12.2024 10:32:51
 * Author : Ntofeu nyatcha dimitry
 */ 

#define F_CPU 4000000UL
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#define BAUD_RATE 9600
#define SCALING_FACTOR 4096

volatile uint32_t sekunde = 0;
char usart_buffer[64];

char* int_to_string(uint16_t number, char* zeichenkette){
	int position = 0;
	
	if(number == 0){
		zeichenkette[position++] = '0';
	}
	if(number < 0){
		//number = ~number & 1;
		zeichenkette[position++] = '-';
	}
	while(number > 0){
		zeichenkette[position++] = '0' + number % 10 ;
		number /= 10;
	}
	
	zeichenkette[position] = '\0'; // string terminator
	
	for(int i = 0; i < position /2; i++){
		char contener = zeichenkette[i];
		zeichenkette[i] = zeichenkette[position - 1  -i];
		zeichenkette[position - 1 - i] = contener;
	}
	return zeichenkette;
}

void Timer_init(){
	
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm; // Overflow-interrupt für den Timer
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV64_gc | TCA_SINGLE_ENABLE_bm; // timer aktivieren und Precaler setzen
	TCA0.SINGLE.PER = 62500;
}

ISR(TCA0_OVF_vect){
	sekunde++;
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm; // interupt-flags deaktivieren
}

void ADC_init(){
	
    VREF.ADC0REF = VREF_REFSEL_2V048_gc; // interne referenzspannung für den Temperatursensor
	ADC0.CTRLA = ADC_ENABLE_bm | ADC_RESSEL_12BIT_gc; // 12-bit aufloesung und aktivierung
	ADC0.CTRLC = ADC_PRESC_DIV16_gc; // prescaller ADC
	ADC0.MUXPOS = ADC_MUXPOS_TEMPSENSE_gc; // Interner Tempratur sensor
	
	ADC0.CTRLB = ADC_SAMPNUM_NONE_gc;  // keine Akkumulation
    ADC0.CTRLD = ADC_INITDLY_DLY64_gc; // initialisierungsverzoegerung >= 25 us
    ADC0.SAMPCTRL = 28; // sample zeit longueur d echantillonage  >= 28 us
	
}

uint16_t ADC_wandlung(){
	
	ADC0.COMMAND = ADC_STCONV_bm;
	while(!(ADC0.INTFLAGS & ADC_RESRDY_bm)){}
	ADC0.INTFLAGS = ADC_RESRDY_bm;
	return ADC0.RES;
}

float ADC_Temperatur(uint16_t adc_wert){
	
	uint16_t sigrow_offset = SIGROW.TEMPSENSE1;    // Offset 
	uint16_t sigrow_slope =  SIGROW.TEMPSENSE0;     // Pente 
	
	uint32_t temp = sigrow_offset - adc_wert;
	temp *= sigrow_slope; // Application de la pente
	temp += SCALING_FACTOR / 2;  // Aufrunden
	temp /= SCALING_FACTOR;  // Division pour obtenir la temperature
	
	temp =  temp - 273;
	
	return (float)temp; // temperatur in celcius
	
}

void USART_init(void){
	float baud = (float)((F_CPU << 6) / (16L * BAUD_RATE));
	USART3.BAUD = baud;
	USART3.CTRLB |= (USART_TXEN_bm | USART_RXEN_bm);
	USART3.CTRLC =  USART_CHSIZE_8BIT_gc;
}

void USART_Uebertragung(char charakter){
	
	while(!(USART3.STATUS & USART_DREIF_bm)){}
	USART3.TXDATAL = charakter;
	
}

void temp_uebertragung(uint32_t sekunde){
	
	uint16_t adc_wert = ADC_wandlung();
	float temp_c = ADC_Temperatur(adc_wert); // convertion in celcius
	float temp_k = temp_c + 273; // conversion in kelvin
	
	char temp_c_str[16];
	char temp_k_str[16];
	int_to_string((uint16_t)(temp_c ), temp_c_str);
	int_to_string((uint16_t)(temp_k ), temp_k_str);
	
	snprintf(usart_buffer, sizeof(usart_buffer),
		"Time: %lu s, Temp: %s.%d degC, %s.%d K\n",
		sekunde,
		temp_c_str , ((uint16_t)(temp_c * 10)) % 10,
		temp_k_str , ((uint16_t)(temp_k * 10)) % 10); // eine dezimal behalten

		for (int i = 0; usart_buffer[i] != '\0'; i++) {
			USART_Uebertragung(usart_buffer[i]);
		}
	
}

int main(void){
	
	USART_init();
	Timer_init();
	ADC_init();
	
	PORTB.DIRSET = PIN0_bm;
	sei();
	
	while(1){
		temp_uebertragung(sekunde);
		_delay_ms(1000);
	}
}