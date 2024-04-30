/*
 * muj_usart.h
 *
 * Created: 3/12/2024 15:26:37
 *  Author: Student
 */ 


#ifndef MUJ_USART_H_
#define MUJ_USART_H_

#include <stdio.h>
void UART_init_PC(uint16_t Baudrate);
void UART_send_char_PC(uint8_t data);
uint8_t UART_get_char_PC( void );
void UART_send_string_PC(char *text);

void UART_init_GPS(uint16_t Baudrate);
void UART_send_char_GPS(uint8_t data);
uint8_t UART_get_char_GPS( void );
void UART_send_string_GPS(char *text);



#endif /* MUJ_USART_H_ */