//#include "C:\Users\patdu\Desktop\IT\Konar\GPS_Ariadna\lib\inc\UBLOX_NEO_07M.h"

#include "UBLOX_NEO_07M.h"

uint8_t UBLOX_NEO_07M_Init(UBLOX_NEO_07M *data)
{
  if(data == NULL) return UBLOX_NEO_07M_STATUS_ERROR;
  data->status = UBLOX_NEO_07M_STATUS_INVALID;
  data->latitude = 0;
  data->longitude = 0;
  data->altitude = 0;
  data->velocity = 0;
  data->hour = 0;
  data->minute = 0;
  data->second = 0;
  data->date = 0;
  data->month = 0;
  data->year = 0;
  return UBLOX_NEO_07M_STATUS_OK;
}

uint8_t UBLOX_NEO_07M_GPS_Init(UBLOX_NEO_07M_GPS *gps, UBLOX_NEO_07M_Interpreter interpreter)
{
  if(gps == NULL || interpreter == NULL) return UBLOX_NEO_07M_STATUS_ERROR;

  memset(gps->small_buffor, 0, UBLOX_NEO_07M_SMALL_BUFFOR_SIZE);
  gps->s_index = 0;
  gps->tokens_count = 0;
  gps->is_valid = UBLOX_NEO_07M_STATUS_INVALID;
  gps->interpreter = interpreter;
  return UBLOX_NEO_07M_STATUS_OK;
}

int containsChar(const char *str, char target) {
  while (*str != '\0') {
    if (*str == target) return UBLOX_NEO_07M_True; 
    str++;
  }
  return UBLOX_NEO_07M_False;
}

uint8_t UBLOX_NEO_07M_Interpreter_GPRMC(UBLOX_NEO_07M *data, char *token, size_t size, int token_index)
{
  if(data == NULL || token == NULL || size == 0) return UBLOX_NEO_07M_STATUS_ERROR;
  if (token_index > 12) return UBLOX_NEO_07M_STATUS_INVALID;

  switch (token_index)
  {
  case 0:
    if (strcmp(token, "GPRMC")) return UBLOX_NEO_07M_STATUS_INVALID;
    else return UBLOX_NEO_07M_STATUS_OK;
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
  case 7: data->velocity = atof(token) * UBLOX_NEO_07M_CONST_KNOTS_TO_METERS_PER_SECOND;
    break;
  case 9: 
    data->date = charToint(token[0])*10 + charToint(token[1]);
    data->month = charToint(token[2])*10 + charToint(token[3]);
    data->year = charToint(token[4])*10 + charToint(token[5]);
    break;
  default:
    break;
  }

  if (containsChar(token, '\n') == UBLOX_NEO_07M_True) return 12 == token_index? UBLOX_NEO_07M_STATUS_OK : UBLOX_NEO_07M_STATUS_INVALID;

  return UBLOX_NEO_07M_STATUS_OK;
}

uint8_t UBLOX_NEO_07M_read(UBLOX_NEO_07M_GPS *gps,UBLOX_NEO_07M *data, uint8_t byte)
{
  if(gps == NULL || data == NULL ) return UBLOX_NEO_07M_STATUS_ERROR;

  if(byte == ',' || byte == '\n'){
    gps->small_buffor[gps->s_index] =  byte == ','? '\0' : byte;
    uint8_t result =  gps->interpreter(data, gps->small_buffor, UBLOX_NEO_07M_SMALL_BUFFOR_SIZE,gps->tokens_count);
    gps->s_index = 0;
    gps->tokens_count++;
    memset(gps->small_buffor, 0, UBLOX_NEO_07M_SMALL_BUFFOR_SIZE);
    if (result != UBLOX_NEO_07M_STATUS_OK)
      UBLOX_NEO_07M_GPS_Init(gps,gps->interpreter);

    if (byte == '\n')
      return gps->is_valid;
  }
  else if(byte == '$'){
    UBLOX_NEO_07M_GPS_Init(gps,gps->interpreter);
    gps->is_valid = UBLOX_NEO_07M_STATUS_OK;
  }
  else{
    gps->small_buffor[gps->s_index] = byte;
    gps->s_index++;
  }

  return UBLOX_NEO_07M_STATUS_WAIT;
}

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

void Task_Read_GPS_DMA_(void *param) {
  (void)param;
  QueueHandle_t *uart_rxQueue=NULL; // REPLACE with queue for data from UART HANDLER
  UART_HandleTypeDef *huart=NULL; // REPLACE with UART handler

  uint8_t *buffer= NULL;
  UBLOX_NEO_07M data;
  UBLOX_NEO_07M_GPS gps;
  UBLOX_NEO_07M_Init(&data);
  UBLOX_NEO_07M_GPS_Init(&gps, UBLOX_NEO_07M_Interpreter_GPRMC);
  *uart_rxQueue = xQueueCreate(10, sizeof(uint8_t*));
  HAL_UART_Receive_DMA(huart,*uart_rxQueue, UBLOX_NEO_07M_BUFFER_SIZE);

  UBLOX_NEO_07M temp;
  while (1){
    BaseType_t stat = xQueueReceive(*uart_rxQueue, (void *)&buffer, 50);
    if(stat != pdTRUE) continue;
    
    for (size_t i = 0; i < UBLOX_NEO_07M_HALF_BUFFOR_SIZE; i++){
      uint8_t status = UBLOX_NEO_07M_read(&gps, &temp, buffer[i]);
      if(status == UBLOX_NEO_07M_STATUS_OK)
        data = temp;  // TODO: do something HERE with data
    }
    free(buffer);    
  }
}