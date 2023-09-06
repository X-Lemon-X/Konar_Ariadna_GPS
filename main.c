
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "UBLOX_NEO_07M.h"


int main(int argc, char *argv[])
{
  UBLOX_NEO_07M data;
  UBLOX_NEO_07M_GPS gps;
  UBLOX_NEO_07M_Init(&data);
  UBLOX_NEO_07M_GPS_Init(&gps,UBLOX_NEO_07M_Interpreter_GPRMC);
  uint8_t rs = UBLOX_NEO_07M_STATUS_ERROR;
/*
$GPGLL,5103.74607,N,01658.83108,E,205812.00,A,D*6D
$GPVTG,,T,,M,0.021,N,0.040,K,D*21
*/


  char line[150];
	FILE *file = fopen("testdata" , "r");
  while(fgets(line, sizeof(line), file) != NULL)
	{
    for (int i = 0; i < strlen(line); i++)
    {
      rs = UBLOX_NEO_07M_read(&gps, &data, line[i]);
      printf("rs: %d, %c\n", rs, line[i]);
      // rs = UBLOX_NEO_07M_Single_char_parse(&gps, &data, line[i]);
    }

    // uint8_t rs = UBLOX_NEO_07M_Parse(&data, line, strlen(line) + 1);

    printf("-------------------------------------------------------\n");
    printf("line: %s\n", line);
    if(rs == UBLOX_NEO_07M_STATUS_OK)
    {
      printf("hour: %d\n", data.hour);
      printf("minute: %d\n", data.minute);
      printf("second: %d\n", data.second);
      printf("status: %d\n", data.status);
      printf("latitude: %f\n", data.latitude);
      printf("longitude: %f\n", data.longitude);
      printf("altitude: %f\n", data.altitude);
      printf("velocity: %f\n", data.velocity);
      printf("date: %d\n", data.date);
      printf("month: %d\n", data.month);
      printf("year: %d\n", data.year);
      
    }

    printf("rs: %d\n", rs);
      printf("\n");
  }

  
  return 0;
}

