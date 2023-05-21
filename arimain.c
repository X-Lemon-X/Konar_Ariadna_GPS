#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "UBLOX_NEO_07M.h"

TIM_HandleTypeDef htim11;
UART_HandleTypeDef huart1;

static QueueHandle_t queue_gps_data;
#define GPS_QUEUE_SIZE 10


HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority) {
	
	const uint32_t uwTimclock = HAL_RCC_GetPCLK2Freq();
	const uint32_t uwPrescalerValue = (uwTimclock/1000000) - 1;
	
	__HAL_RCC_TIM11_CLK_ENABLE();

	htim11.Instance = TIM11;
	htim11.Init.Period = 999;
	htim11.Init.Prescaler = uwPrescalerValue;
	htim11.Init.ClockDivision = 0;
	htim11.Init.CounterMode = TIM_COUNTERMODE_UP;

	if(HAL_TIM_Base_Init(&htim11)!=HAL_OK)
		return HAL_ERROR;
	
	HAL_NVIC_SetPriority(TIM1_TRG_COM_TIM11_IRQn, TickPriority, 0);
	HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);

	return HAL_TIM_Base_Start_IT(&htim11);
}

void TIM1_TRG_COM_TIM11_IRQHandler() {
	HAL_TIM_IRQHandler(&htim11);
}

HAL_StatusTypeDef MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  return HAL_UART_Init(&huart1);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if(htim->Instance==TIM11) {
		HAL_IncTick();
	}
}

void InitGPS_QUEUE() {
  queue_gps_data = xQueueCreate(GPS_QUEUE_SIZE, sizeof(UBLOX_NEO_07M));
}

void blink(void *param) {
	(void)param;

	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef led = {
		.Pin = GPIO_PIN_5,
		.Mode = GPIO_MODE_OUTPUT_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_LOW
	};

	HAL_GPIO_Init(GPIOA, &led);

	while(1) {
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		vTaskDelay(1000);

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		vTaskDelay(1000);
	}
}

void ReadGPS(void *param) {
  (void)param;
  UBLOX_NEO_07M data;
  UBLOX_NEO_07M_Init(&data);
  uint8_t buffer[UBLOX_NEO_07M_BUFFER_SIZE];

  while(1) {
    vTaskDelay(50);
    
    HAL_StatusTypeDef stat = HAL_UART_Receive(&huart1, buffer, UBLOX_NEO_07M_BUFFER_SIZE,500);

    if(stat != HAL_OK) continue;
    //if( HAL_UART_Receive(&huart1, buffer, UBLOX_NEO_07M_BUFFER_SIZE, 1000) != HAL_OK) continue;

    if(UBLOX_NEO_07M_Parse(&data, (char*)buffer, UBLOX_NEO_07M_BUFFER_SIZE) != UBLOX_NEO_07M_STATUS_OK) continue;
    
    xQueueSend(queue_gps_data, (void *)&data, 10);
  }
}

void ReadQueueGps(void *param)
{
  UBLOX_NEO_07M data;
  while (1)
  {
    if(xQueueReceive(queue_gps_data, (void *)&data, 10)==pdTRUE)
    {
     // HAL_UART_Transmit(&huart1, (uint8_t *)&data, sizeof(UBLOX_NEO_07M), 1000);
     float leat = data.altitude;
    }
  }
  
}

int main() {

	HAL_Init();

	RCC_OscInitTypeDef oscillator = {
		.OscillatorType = RCC_OSCILLATORTYPE_HSI,
		.HSIState = RCC_HSI_ON,
		.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT,
		.PLL.PLLState = RCC_PLL_ON,
		.PLL.PLLSource = RCC_PLLSOURCE_HSI,
		.PLL.PLLM = 8,
		.PLL.PLLN = 100,
		.PLL.PLLP = RCC_PLLP_DIV2,
		.PLL.PLLQ = 4
	};

	RCC_ClkInitTypeDef clock = {
		.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2,
		.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK,
		.AHBCLKDivider = RCC_SYSCLK_DIV1,
		.APB1CLKDivider = RCC_HCLK_DIV2,
		.APB2CLKDivider = RCC_HCLK_DIV1
	};

	HAL_RCC_OscConfig(&oscillator);
	HAL_RCC_ClockConfig(&clock, FLASH_LATENCY_2);
  MX_USART1_UART_Init();
  InitGPS_QUEUE();

	xTaskCreate(blink, "blink", 1024, NULL, 4, NULL);
  xTaskCreate(ReadGPS, "read gps", 2024, NULL, 4, NULL);
  xTaskCreate(ReadQueueGps, "read dgps", 1024, NULL, 4, NULL);

	vTaskStartScheduler();

	return 0;
}
