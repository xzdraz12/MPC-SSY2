/*
 * muj_usart.c
 *
 * Created: 2/13/2024 14:54:05
 *  Author: Student
 */ 


#include <avr/io.h>
#include <stdint.h>
//#include "D:/AtmelStudio/xzdraz12/LAB2/230917/230917/makra.h"
#include "../makra.h"

void UART_init_PC(uint16_t Baudrate)
{
	int ubrr=((F_CPU/16/Baudrate)-1); //vypocet baudrate --- delat radsi takhle, jiny vypocet zvysuje cislo a vede k preteceni
	
	//nastavuju uart1 pro pripojeni k putty pc
	sbi(UCSR1C, UCSZ11); 
	sbi(UCSR1C, UCSZ10);
	
	UBRR1H = (uint8_t)(ubrr>>8); //8 bitu
	UBRR1L = (uint8_t)ubrr;
	
	sbi(UCSR1B,RXEN1); //zapnu rx
	sbi(UCSR1B,TXEN1); //zapnu tx
	
}

void UART_send_char_PC(uint8_t data)
{
	while( !(UCSR1A & (1<<UDRE1))) //cekej, dokud v registru ucsr1a neni bit udre0 = 1 -> registr je prazdny
	;
	UDR1 = data;
}


void UART_send_string_PC(char *text)
{
	while (*text != 0x00) //dokud adresa textu neni 0x00
	{
		UART_send_char_PC(*text);
		text++;
	}
}


uint8_t UART_get_char_PC(void)
{
	while( !(UCSR1A & (1<<RXC0))) //cekej, dokud v registru ucsr1a neni bit rxc0 = 1 -> dokoncen prenos
	;
	return UDR1;
}


//-------------------------------------------------------------------------



//nastavuju UART0 pro pripojeni ke gps
void UART_init_GPS(uint16_t Baudrate)
{
	int ubrr=((F_CPU/16/Baudrate)-1); //vypocet baudrate --- delat radsi takhle, jiny vypocet zvysuje cislo a vede k preteceni
	
	//nastavuju uart
	sbi(UCSR0C, UCSZ01);
	sbi(UCSR0C, UCSZ00);
	
	UBRR0H = (uint8_t)(ubrr>>8); //8 bitu
	UBRR0L = (uint8_t)ubrr;
	
	sbi(UCSR0B,RXEN0); //zapnu rx
	sbi(UCSR0B,TXEN0); //zapnu tx
	
}

void UART_send_char_GPS(uint8_t data)
{
	while( !(UCSR0A & (1<<UDRE0))) //cekej, dokud v registru ucsr1a neni bit udre0 = 1 -> registr je prazdny
	;
	UDR0 = data;
}


void UART_send_string_GPS(char *text)
{
	while (*text != 0x00) //dokud adresa textu neni 0x00
	{
		UART_send_char_GPS(*text);
		text++;
	}
}


uint8_t UART_get_char_GPS(void)
{
	while( !(UCSR0A & (1<<RXC0))) //cekej, dokud v registru ucsr1a neni bit rxc0 = 1 -> dokoncen prenos
	;
	return UDR0;
}