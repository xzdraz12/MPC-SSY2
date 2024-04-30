/*
 * UART_GPS.h
 *
 * Created: 4/25/2024 12:15:14
 *  Author: Student
 */ 


#ifndef UART_GPS_H_
#define UART_GPS_H_

#include <stdio.h>
#include "makra.h"
void UART_init_GPS(uint16_t Baudrate);
void UART_send_char_GPS(uint8_t data);
uint8_t UART_get_char_GPS( void );
void UART_send_string_GPS(char *text);




#endif /* UART_GPS_H_ */