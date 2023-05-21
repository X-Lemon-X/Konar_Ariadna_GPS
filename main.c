
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "UBLOX_NEO_07M.h"


int main(int argc, char *argv[])
{
  UBLOX_NEO_07M data;
  UBLOX_NEO_07M_Init(&data);
  
  char line[150];
	FILE *file = fopen("/home/lemonx/IT/Konar/Konar_Ariadna_GPS/testdata" , "r");
  while(fgets(line, sizeof(line), file) != NULL)
	{
    uint8_t rs = UBLOX_NEO_07M_Parse(&data, line, strlen(line) + 1);

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
      printf("speed: %f\n", data.speed);
      printf("date: %d\n", data.date);
      printf("month: %d\n", data.month);
      printf("year: %d\n", data.year);
      
    }

    printf("rs: %d\n", rs);
      printf("\n");
  }

  
  return 0;
}

