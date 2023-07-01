//#include "C:\Users\patdu\Desktop\IT\Konar\GPS_Ariadna\lib\inc\UBLOX_NEO_07M.h"

#include "UBLOX_NEO_07M.h"

uint8_t UBLOX_NEO_07M_read_RMC(UBLOX_NEO_07M *data, char *buffer, size_t size );
uint8_t UBLOX_NEO_07M_addGapsBetweenSeparator(char *bufferOld,size_t size, char **bufferNew,size_t *newSize, char separator);


uint8_t UBLOX_NEO_07M_Init(UBLOX_NEO_07M *data)
{
  if(data == NULL) return UBLOX_NEO_07M_STATUS_ERROR;
  data->status = UBLOX_NEO_07M_STATUS_INVALID;
  data->latitude = 0;
  data->longitude = 0;
  data->altitude = 0;
  data->speed = 0;
  data->hour = 0;
  data->minute = 0;
  data->second = 0;
  data->date = 0;
  data->month = 0;
  data->year = 0;
  return UBLOX_NEO_07M_STATUS_OK;
}


uint8_t UBLOX_NEO_07M_Parse(UBLOX_NEO_07M *data, char *buffer, size_t size )
{
  if( buffer == NULL || data == NULL || size == 0 ) return UBLOX_NEO_07M_STATUS_ERROR;
  char *bufferNew=NULL;
  size_t newSize=0;
  uint8_t res = UBLOX_NEO_07M_STATUS_ERROR;
  char keyvalue[UBLOX_NEO_07M_BUFFER_KEY_SIZE + 1];
  memcpy(keyvalue, buffer, UBLOX_NEO_07M_BUFFER_KEY_SIZE);
  keyvalue[UBLOX_NEO_07M_BUFFER_KEY_SIZE] = '\0';

  if(UBLOX_NEO_07M_addGapsBetweenSeparator(buffer, size, &bufferNew, &newSize, ',')) return UBLOX_NEO_07M_STATUS_ERROR;

  //intrprets data dependent from key value
  if(!strcmp(keyvalue, "$GPRMC")) res = UBLOX_NEO_07M_read_RMC(data, bufferNew, newSize);
  else res = UBLOX_NEO_07M_STATUS_INVALID;

  free(bufferNew);
  return res;
}


/// @brief convert GPRMS data to UBLOX_NEO_07M struct
/// @param data 
/// @param buffer 
/// @param size 
/// @return 
uint8_t UBLOX_NEO_07M_read_RMC(UBLOX_NEO_07M *data, char *buffer, size_t size )
{
  if(data == NULL || buffer == NULL || size == 0) return UBLOX_NEO_07M_STATUS_ERROR;
  char *token;
  size_t posCount = 0;
  token = strtok(buffer, ",");

  while (token) {
    switch (posCount)
    {
    case 1: 
      data->hour = charToint(token[0])*10 + charToint(token[1]);
      data->minute = charToint(token[2])*10 + charToint(token[3]);
      data->second = charToint(token[4])*10 + charToint(token[5]);
      break;
    case 2: data->status = (uint8_t)token[0];
      break;  
    case 3: data->latitude = atof(token);
      break;
    case 5: data->longitude = atof(token);
      break;
    case 7: data->speed = atof(token) * UBLOX_NEO_07M_CONST_KNOTS_TO_METERS_PER_SECOND;
      break;
    case 9: 
      data->date = charToint(token[0])*10 + charToint(token[1]);
      data->month = charToint(token[2])*10 + charToint(token[3]);
      data->year = charToint(token[4])*10 + charToint(token[5]);
      break;
    default:
      break;
    } 
    posCount++;
    token = strtok(NULL, ",");
  }

  return UBLOX_NEO_07M_STATUS_OK;
}

/// @brief add gaps between separators taht repeat one after onother np. ",," -> ", ,"
/// @param bufferOld buffer with data
/// @param size size of buffer
/// @param bufferNew new buffer with gaps between separators
/// @param newSize size of new buffer
/// @param separator separator
uint8_t UBLOX_NEO_07M_addGapsBetweenSeparator(char *bufferOld,size_t size, char **bufferNew, size_t *newSize, char separator)
{
  if( bufferOld == NULL || size == 0 || newSize == NULL ) return UBLOX_NEO_07M_STATUS_ERROR;
  //alloc tempo buffer
  char *temp = newString(2*size);
  size_t countSeparators = 0;
  //add gaps between separators taht repeat one after onother np. ",," -> ", ,"
  for (size_t i = 0; i < size; i++){
    if( (i+1 < size) && bufferOld[i] == separator && bufferOld[i+1] == separator){
      temp[i+countSeparators] = bufferOld[i];
      countSeparators++;
      temp[i+countSeparators] = ' ';
    }
    else temp[i+countSeparators] = bufferOld[i];
  }

  //finsh string with '\0'
  temp[size+countSeparators] = '\0';
  //resize buffer for only necessary size
  *bufferNew = (char *)realloc(temp, (size+countSeparators)*sizeof(char));
  *newSize = size+countSeparators;
  
  if(*bufferNew == NULL){
    free(temp);
    return UBLOX_NEO_07M_STATUS_ERROR;
  }
  return UBLOX_NEO_07M_STATUS_OK;
}

uint8_t UBLOX_NEO_07M_Parse(UBLOX_NEO_07M_GPS *gps,UBLOX_NEO_07M *data, char sign)
{
  if( data == NULL || gps == NULL ) return UBLOX_NEO_07M_STATUS_ERROR;

  gps->buffer[gps->index] = sign;
  if(sign == '\n'){
    uint8_t status = UBLOX_NEO_07M_Parse(data, gps->buffer, gps->index);
    memcpy(gps->buffer, gps->buffer + gps->index , UBLOX_NEO_07M_BUFFER_BUFFOR_SIZE - gps->index);
    gps->index = UBLOX_NEO_07M_BUFFER_BUFFOR_SIZE - gps->index;
    return status;
  }  
  else gps->index++;
  
  return UBLOX_NEO_07M_STATUS_OK;
}