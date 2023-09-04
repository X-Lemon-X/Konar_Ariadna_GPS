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
#define UBLOX_NEO_07M_BUFFER_BUFFOR_SIZE 150
#define UBLOX_NEO_07M_CONST_KNOTS_TO_METERS_PER_SECOND 0.514444f
#define UBLOX_NEO_07M_SMALL_BUFFOR_SIZE 20
#define UBLOX_NEO_07M_True 1
#define UBLOX_NEO_07M_False 0

//macros 
#define charToint(c) (c - '0') //only for numbers from 0-9



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


typedef uint8_t (*UBLOX_NEO_07M_Interpreter)(UBLOX_NEO_07M *, char*, size_t, int);

typedef struct 
{
  char small_buffor[UBLOX_NEO_07M_SMALL_BUFFOR_SIZE];
  size_t s_index;
  int tokens_count;
  int is_valid;
  UBLOX_NEO_07M_Interpreter interpreter;
} UBLOX_NEO_07M_GPS;




/// @brief initiate UBLOX_NEO_07M structure 
/// @param data - gps data out structure
/// @return status UBLOX_NEO_07M_STATUS ...
uint8_t UBLOX_NEO_07M_Init(UBLOX_NEO_07M *data);

/// @brief initiate UBLOX_NEO_07M_GPS structure
/// @param gps - gps structure
/// @param interpreter - function pointer to interpreter function for specific gps data
uint8_t UBLOX_NEO_07M_GPS_Init(UBLOX_NEO_07M_GPS *gps, UBLOX_NEO_07M_Interpreter interpreter);

/// @brief parse data from gps 
/// @param data - gps data out structure
/// @param buffer - buffer with data from gps
/// @return status UBLOX_NEO_07M_STATUS_WAIT if data is not complete, UBLOX_NEO_07M_STATUS_OK if data is complete, UBLOX_NEO_07M_STATUS_ERROR if error occured
uint8_t UBLOX_NEO_07M_read(UBLOX_NEO_07M_GPS *gps,UBLOX_NEO_07M *data, uint8_t byte);

/// @brief interpteter for GPRMC data use only with UBLOX_NEO_07M_GPS_Init
/// @param data - gps data out structure
/// @param token - token from gps data
/// @param size - size of token
/// @param token_index - index of token
uint8_t UBLOX_NEO_07M_Interpreter_GPRMC(UBLOX_NEO_07M *data, char *token, size_t size, int token_index);


#endif