#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef _UBLOX_NEO_07M_H_
#define _UBLOX_NEO_07M_H_


//status values
#define UBLOX_NEO_07M_STATUS_OK 0
#define UBLOX_NEO_07M_STATUS_ERROR 1
#define UBLOX_NEO_07M_STATUS_INVALID 2
#define UBLOX_NEO_07M_STATUS_WAIT 3
#define UBLOX_NEO_07M_DATA_STATUS_VALID 'A'
#define UBLOX_NEO_07M_DATA_STATUS_INVALID 'V'


//constants

#define UBLOX_NEO_07M_BUFFER_SIZE 200
#define UBLOX_NEO_07M_HALF_BUFFOR_SIZE 100

#define UBLOX_NEO_07M_CONST_KNOTS_TO_METERS_PER_SECOND 0.514444f
#define UBLOX_NEO_07M_SMALL_BUFFOR_SIZE 20
#define UBLOX_NEO_07M_True 1
#define UBLOX_NEO_07M_False 0

//macros 
#define charToint(c) (c - '0') //only for numbers from 0-9


// gps data structure
// hour, minute, second, status, latitude, longitude, velocity, date, month, year
typedef struct
{
  // GPS position
  uint8_t status; // A - valid, V - invalid
  float latitude;
  float longitude;
  float altitude;

  // GPS velocity
  float velocity;

  // GPS time
  uint8_t hour;
  uint8_t minute;
  uint8_t second;

  //GPS date
  uint8_t date;
  uint8_t month;
  uint8_t year;
} UBLOX_NEO_07M;


/// @brief function pointer to interpreter function for specific gps data
typedef uint8_t (*UBLOX_NEO_07M_Interpreter)(UBLOX_NEO_07M *, char*, size_t, int);

/// @brief structure for gps interpreter
typedef struct 
{
  char small_buffor[UBLOX_NEO_07M_SMALL_BUFFOR_SIZE];
  size_t s_index;
  int tokens_count;
  int is_valid;
  UBLOX_NEO_07M_Interpreter interpreter;
} UBLOX_NEO_07M_GPS;



/// @brief initiate UBLOX_NEO_07M structure 
/// @param data gps data out structure
/// @return status UBLOX_NEO_07M_STATUS ...
uint8_t UBLOX_NEO_07M_Init(UBLOX_NEO_07M *data);

/// @brief initiate UBLOX_NEO_07M_GPS structure
/// @param gps - gps structure
/// @param interpreter - function pointer to interpreter function for specific gps data
uint8_t UBLOX_NEO_07M_GPS_Init(UBLOX_NEO_07M_GPS *gps, UBLOX_NEO_07M_Interpreter interpreter);

/// @brief parse data from gps 
/// @param data gps data out structure
/// @param buffer buffer with data from gps
/// @return status UBLOX_NEO_07M_STATUS_WAIT if data is not complete, UBLOX_NEO_07M_STATUS_OK if data is complete, UBLOX_NEO_07M_STATUS_ERROR if error occured
uint8_t UBLOX_NEO_07M_read(UBLOX_NEO_07M_GPS *gps,UBLOX_NEO_07M *data, uint8_t byte);

/// @brief interpteter for GPRMC data use only as argument of UBLOX_NEO_07M_GPS_Init
/// @param data gps data out structure
/// @param token token from gps data
/// @param size size of token
/// @param token_index index of token
uint8_t UBLOX_NEO_07M_Interpreter_GPRMC(UBLOX_NEO_07M *data, char *token, size_t size, int token_index);

/// @brief Run this function on UART DMA HAL_UART_RxCpltCallback and HAL_UART_RxHalfCpltCallback
/// @param buff_begin beginning of the buffor  [0 or half] 
/// @param buff_end end of the buffor [half -1 or full -1]  tells which  half of the bufor it is
/// @param uart_rxBuffer buffor of the DMA 
/// @param uart_rxQueue queue where tu puch data from DMA buffor
void UBLOX_NEO_07M_UART_HANDLER(size_t buff_begin,size_t buff_end, uint8_t uart_rxBuffer, QueueHandle_t *uart_rxQueue)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  void *halfBuffor = malloc(UBLOX_NEO_07M_HALF_BUFFOR_SIZE*sizeof(uint8_t));
  if (halfBuffor == NULL) return;

  memcpy(halfBuffor, uart_rxBuffer+buff_begin, buff_end-buff_begin);
  // memcpy(halfBuffor, UART6_rxBuffer+buff_begin, buff_end-buff_begin);

  BaseType_t stat =xQueueCRSendFromISR(*uart_rxQueue, (void *)&halfBuffor, xHigherPriorityTaskWoken);
  if(stat) free(halfBuffor);
  if( xHigherPriorityTaskWoken ) portYIELD_FROM_ISR( xHigherPriorityTaskWoken ); // Actual macro used here is port specific.
}


/*

void ReadGPS_DMA(void *param) {
  (void)param;
  uint8_t *buffer= NULL;
  UBLOX_NEO_07M data;
  UBLOX_NEO_07M_GPS gps;
  UBLOX_NEO_07M_Init(&data);
  UBLOX_NEO_07M_GPS_Init(&gps, UBLOX_NEO_07M_Interpreter_GPRMC);
  UART6_rxQueue = xQueueCreate(10, sizeof(uint8_t*));
  HAL_UART_Receive_DMA(&huart6,UART6_rxBuffer, UBLOX_NEO_07M_BUFFER_SIZE);

  UBLOX_NEO_07M temp;
  while (1){
    BaseType_t stat = xQueueReceive(UART6_rxQueue, (void *)&buffer, 50);
    if(stat != pdTRUE) continue;
    
    for (size_t i = 0; i < UBLOX_NEO_07M_HALF_BUFFOR_SIZE; i++){
      uint8_t status = UBLOX_NEO_07M_read(&gps, &temp, buffer[i]);
      if(status == UBLOX_NEO_07M_STATUS_OK)
        data = temp;  // TODO: do something HERE with data
    }
    free(buffer);    
  }
}

*/


#endif