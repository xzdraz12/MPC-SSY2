/*
 * LWM_MSSY.c
 *
 * Created: 6.4.2017 15:42:46
 * Author : Jakub
 */ 

#include <avr/io.h>
/*- Includes ---------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "config.h"
#include "hal.h"
#include "phy.h"
#include "sys.h"
#include "nwk.h"
#include "sysTimer.h"
#include "halBoard.h"
#include "halUart.h"
#include "main.h"
#include "UART_GPS.h"
#include <util/delay.h>


char gpsTime[16]; // Pole pro ukladani GPS dat
char* NMEA_parser(char *NMEA_sentence);


/*- Definice ------------------------------------------------------------*/
#ifdef NWK_ENABLE_SECURITY
#define APP_BUFFER_SIZE     (NWK_MAX_PAYLOAD_SIZE - NWK_SECURITY_MIC_SIZE)
#else
#define APP_BUFFER_SIZE     NWK_MAX_PAYLOAD_SIZE
#endif

/*- Definice Typu ------------------------------------------------------------------*/
typedef enum AppState_t
{
	APP_STATE_INITIAL,
	APP_STATE_IDLE,
} AppState_t;

/*- Prototypy -------------------------------------------------------------*/

static void SendGPS(void);

/*- Promenne --------------------------------------------------------------*/
static AppState_t appState = APP_STATE_INITIAL;
static SYS_Timer_t appTimer;
static SYS_Timer_t GPSTimer;
static NWK_DataReq_t appDataReq;
static bool appDataReqBusy = false;
static uint8_t appDataReqBuffer[APP_BUFFER_SIZE];
static uint8_t appUartBuffer[APP_BUFFER_SIZE];
static uint8_t appUartBufferPtr = 0;

//static SYS_Timer_t_gps appTimer;


/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
*****************************************************************************/
static void appDataConf(NWK_DataReq_t *req)
{
appDataReqBusy = false;
(void)req;
}

/*************************************************************************//**
*****************************************************************************/

static void appSendData(void)
{
if (appDataReqBusy || 0 == appUartBufferPtr)
return;

memcpy(appDataReqBuffer, appUartBuffer, appUartBufferPtr);

appDataReq.dstAddr = 1-APP_ADDR;
appDataReq.dstEndpoint = APP_ENDPOINT;
appDataReq.srcEndpoint = APP_ENDPOINT;
appDataReq.options = NWK_OPT_ENABLE_SECURITY;
appDataReq.data = appDataReqBuffer;
appDataReq.size = appUartBufferPtr;
appDataReq.confirm = appDataConf;
NWK_DataReq(&appDataReq);

appUartBufferPtr = 0;
appDataReqBusy = true;
}

/*************************************************************************//**
*****************************************************************************/

void HAL_UartBytesReceived(uint16_t bytes)
{
for (uint16_t i = 0; i < bytes; i++)
{
uint8_t byte = HAL_UartReadByte();

if (appUartBufferPtr == sizeof(appUartBuffer))
appSendData();

if (appUartBufferPtr < sizeof(appUartBuffer))
appUartBuffer[appUartBufferPtr++] = byte;
}

SYS_TimerStop(&appTimer);
SYS_TimerStart(&appTimer);
}

/*************************************************************************//**
*****************************************************************************/

static void appTimerHandler(SYS_Timer_t *timer)
{
appSendData();
(void)timer;
}


static void GPSTimerHandler(SYS_Timer_t *timer)
{
SendGPS();
//(void)timer;
}

/*************************************************************************//**
*****************************************************************************/

static bool appDataInd(NWK_DataInd_t *ind)
{
for (uint8_t i = 0; i < ind->size; i++)
HAL_UartWriteByte(ind->data[i]);
return true;
}

/*************************************************************************//**
*****************************************************************************/

static void appInit(void)
{
NWK_SetAddr(APP_ADDR);
NWK_SetPanId(APP_PANID);
PHY_SetChannel(APP_CHANNEL);
#ifdef PHY_AT86RF212
PHY_SetBand(APP_BAND);
//PHY_SetModulation(APP_MODULATION);
#endif
PHY_SetRxState(true);

NWK_OpenEndpoint(APP_ENDPOINT, appDataInd);

HAL_BoardInit();

appTimer.interval = APP_FLUSH_TIMER_INTERVAL;
appTimer.mode = SYS_TIMER_INTERVAL_MODE;
appTimer.handler = appTimerHandler;

GPSTimer.interval = 5000;
GPSTimer.mode = SYS_TIMER_PERIODIC_MODE;
GPSTimer.handler = GPSTimerHandler;

SYS_TimerStart(&GPSTimer);
}

/*************************************************************************//**
*****************************************************************************/

// Funkce pro posilani GPS dat  



static void SendGPS(void) {
	char gpsData[1000];
	char *gpsDataParsed;
	uint8_t ch;
	char *ptr = gpsData;
    HAL_UartWriteString("GPS: \n\r");
	
	// Cti GPS data z UARTU
	while((ch = UART_get_char_GPS()) != '\n') {
		*ptr++ = ch;
	}
	*ptr = '\0';
	//*ptr++ = '\0';
	
	
	// Volej funckci pro parsovani dat
	gpsDataParsed = NMEA_parser(gpsData);
	
	// Vypis naparsovana data do terminalu
	HAL_UartWriteString(gpsDataParsed);
	HAL_UartWriteString("\n\r");

	// Nachystej data pro odeslani funkci appSendData()
	strncpy(appDataReqBuffer, gpsDataParsed, APP_BUFFER_SIZE);
	appSendData();
}



/*************************************************************************//**
*****************************************************************************/

/*

static void SendGPS(void) {
	char gpsData[100] = "$GPGGA,122220.065,4913.619,N,01634.492,E,1,12,1.0,0.0,M,0.0,M,,*62";
	char *gpsDataParsed;

	// Parse the GPS data
	gpsDataParsed = NMEA_parser(gpsData);

	if (gpsDataParsed != NULL) {
		HAL_UartWriteString(gpsDataParsed);  // Print the parsed data
		free(gpsDataParsed);  // Free the memory allocated by NMEA_parser
		} else {
		HAL_UartWriteString("Invalid or unsupported NMEA sentence.");
	}

	// Ensure the GPS data fits in the buffer and is null-terminated
	strncpy(appDataReqBuffer, gpsData, APP_BUFFER_SIZE - 1);
	appDataReqBuffer[APP_BUFFER_SIZE - 1] = '\0';
}


/*************************************************************************//**
*****************************************************************************/


char* NMEA_parser(char *NMEA_sentence) {
	char *token;
	char *result = NULL;

	
	token = strtok(NMEA_sentence, ",");
	if (strcmp(token, "$GPGGA") == 0) {  // Zkontroluj, jestli je veta formatu GPGGA
		token = strtok(NULL, ",");  // Dalsi token
		if (token != NULL) {
			result = malloc(strlen(token) + 1);  // Alokuje pamet
			if (result != NULL) {
				strcpy(result, token);  
			}
		}
	}
	return result; 
}

/*************************************************************************//**
*****************************************************************************/

static void APP_TaskHandler(void)
{
switch (appState)
{
case APP_STATE_INITIAL:
{
appInit();
appState = APP_STATE_IDLE;
} break;

case APP_STATE_IDLE:

break;

default:
break;
}
}



int main(void)
{
	SYS_Init();
	HAL_UartInit(9600);
	UART_init_GPS(9600);
	HAL_UartWriteByte('a');
	
	


	while (1)
	{
		SYS_TaskHandler();
		HAL_UartTaskHandler();
		APP_TaskHandler();
		
		//SendGPS();



	}
}



/*************************************************************************//**
*****************************************************************************/




ISR(USART0_RX_vect)
{ 
	UDR0=UDR1;
}









