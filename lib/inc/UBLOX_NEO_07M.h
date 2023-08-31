#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef _UBLOX_NEO_07M_H_
#define _UBLOX_NEO_07M_H_

/*
DEFINE FREE_RTOS_UBLOX_NEO_07M if you want to use this library with FreeRTOS
*/
// #define FREE_RTOS_UBLOX_NEO_07M

//status values
#define UBLOX_NEO_07M_STATUS_OK 0
#define UBLOX_NEO_07M_STATUS_ERROR 1
#define UBLOX_NEO_07M_STATUS_INVALID 2
#define UBLOX_NEO_07M_STATUS_WAIT 3
#define UBLOX_NEO_07M_DATA_STATUS_VALID 'A'
#define UBLOX_NEO_07M_DATA_STATUS_INVALID 'V'


//constants
#define UBLOX_NEO_07M_BUFFER_BUFFOR_SIZE 150
#define UBLOX_NEO_07M_BUFFER_KEY_SIZE 6
#define UBLOX_NEO_07M_CONST_KNOTS_TO_METERS_PER_SECOND 0.514444f


//macros 
#define charToint(c) (c - '0') //only for numbers from 0-9


#ifndef FREE_RTOS_UBLOX_NEO_07M
  #define MemporyAlocaotor(type, size) (type*)malloc(size*sizeof(type))
  #define MemporyFree(ptr) free(ptr)
  #define MemporyReAlocaotor(ptr, type, size) (type*)realloc(ptr,size*sizeof(type))
#else
  #include "FreeRTOS.h"
  #define MemporyAlocaotor(type, size) pvPortMalloc(size*sizeof(type))
  #define MemporyFree(ptr) vPortFree(ptr)
  #define MemporyReAlocaotor(ptr, type, size) pvPortRealloc(ptr,size*sizeof(type))
#endif

// Most of RMC data - Recommended Minimum data
// hour, minute, second, status, latitude, longitude, speed, date, month, year
typedef struct
{
  // GPS position
  uint8_t status; // A - valid, V - invalid
  float latitude;
  float longitude;
  float altitude;

  // GPS speed
  float speed;

  // GPS time
  uint8_t hour;
  uint8_t minute;
  uint8_t second;

  //GPS date
  uint8_t date;
  uint8_t month;
  uint8_t year;
} UBLOX_NEO_07M;

enum UBLOX_NEO_07M_GPS_STATUS
{
  SIGN_BEGIN = 0,
  UBLOX_NEO_07M_GPS_STATUS_ERROR = 1,
  UBLOX_NEO_07M_GPS_STATUS_INVALID = 2
}; 

typedef struct 
{
  char buffer[UBLOX_NEO_07M_BUFFER_BUFFOR_SIZE ];
  size_t index;
} UBLOX_NEO_07M_GPS;



/// @brief initiate UBLOX_NEO_07M structure 
/// @param data - gps data out structure
/// @return status UBLOX_NEO_07M_STATUS ...
uint8_t UBLOX_NEO_07M_Init(UBLOX_NEO_07M *data);


/// @brief parse data from uart to UBLOX_NEO_07M structure
/// data for example "$GPRMC,205815.00,A,5103.74608,N,01658.83137,E,0.041,,130523,,,D*73"
/// @param data UBLOX_NEO_07M structure
/// @param buffer uart data buffer
/// @param size uart data buffer size
/// @return status UBLOX_NEO_07M_STATUS ...
uint8_t UBLOX_NEO_07M_Parse(UBLOX_NEO_07M *data, char *buffer, size_t size );

/// @brief convert GPRMS data to UBLOX_NEO_07M struct
/// @param data data out structure
/// @param gps Gps data structure
/// @param sing single character from some buffer loop over enitre buffer to fill data structure
/// @return status UBLOX_NEO_07M_STATUS ...
uint8_t UBLOX_NEO_07M_Single_char_parse(UBLOX_NEO_07M_GPS *gps,UBLOX_NEO_07M *data, char sign);

#endif