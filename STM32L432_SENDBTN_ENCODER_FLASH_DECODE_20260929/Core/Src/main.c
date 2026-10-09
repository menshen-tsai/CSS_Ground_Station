/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/**
  Click on the Option Bytes tab on the left menu.

  Expand the User Configuration drop-down menu.

  Locate the following two bits and verify their settings:

  nSWBOOT0: This box MUST be unchecked (value = 0).
            This tells the MCU to ignore the physical PA14 pin for
            booting and use the software bit instead.

  nBOOT0: This box MUST be checked (value = 1). When nSWBOOT0 is 1,
          this bit acts as your virtual BOOT0 pin.
          Setting it to 1 forces the MCU to boot directly into your
          main Flash memory code.

**/
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include "stdlib.h"
#include "string.h"
#include "math.h"

#include "project_info.h"


#include "dma_printf.h"
#include "dma_scanf.h"
#include "log.h"

#include "menu.h"
#include "SR105U.h"


#include "dtmf.h"
#include "BitFIFO.h"
#include "AFSK.h"
#include "APRS.h"



/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */




/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Page 127 start address for STM32L432KC (256KB total flash)
#define FLASH_BASE_ADDR          0x08000000U
#define CONFIG_FLASH_PAGE        127
#define CONFIG_FLASH_ADDRESS     (FLASH_BASE_ADDR + (CONFIG_FLASH_PAGE * FLASH_PAGE_SIZE))		//0x0803F800U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

DAC_HandleTypeDef hdac1;
DMA_HandleTypeDef hdma_dac_ch1;

OPAMP_HandleTypeDef hopamp1;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

/* USER CODE BEGIN PV */
uint8_t flag = 0;
bool send_Pressed = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_DAC1_Init(void);
static void MX_OPAMP1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM7_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#include "stm32l4xx_hal.h"


#include "stm32l4xx_hal.h"
#include <stdio.h>


void aprs_led_pulse(void) {
    // 1. Turn LED ON
    HAL_GPIO_WritePin(APRS_VALID_GPIO_Port, APRS_VALID_Pin, GPIO_PIN_SET);

    // 2. Reset counter value
    __HAL_TIM_SET_COUNTER(&htim1, 0);

    // 3. Start Timer in Interrupt mode
    HAL_TIM_Base_Start_IT(&htim1);
}


HAL_StatusTypeDef Flash_WriteConfig(uint64_t data)
{
    // 1. Read actual flash size (in KB)
    uint32_t flash_size_kb = *((uint16_t*)FLASHSIZE_BASE);
    uint32_t flash_size_bytes = flash_size_kb * 1024;

    // 2. Compute page count and last valid page index
    uint32_t page_count = flash_size_bytes / 2048; // 2 KB per page
    uint32_t last_page_index = page_count - 1;
    uint32_t last_page_address = 0x08000000 + (last_page_index * 0x800);

    // 3. Unlock and clear flags
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    // 4. Erase last page
    FLASH_EraseInitTypeDef EraseInit;
    uint32_t PageError = 0;
    EraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInit.Page = last_page_index;
    EraseInit.NbPages = 1;
    EraseInit.Banks = FLASH_BANK_1;

    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&EraseInit, &PageError);
    if (status != HAL_OK) {
        HAL_FLASH_Lock();
        return status; // Erase failed
    }

    // 5. Program 64-bit data at start of last page
    status = HAL_FLASH_Program(
        FLASH_TYPEPROGRAM_DOUBLEWORD,
        last_page_address,
        data
    );

    // 6. Lock flash again
    HAL_FLASH_Lock();

    return status;
}

HAL_StatusTypeDef Flash_EraseLastPage(void)
{
    // 1. Read actual flash size
    uint32_t flash_size_kb = *((uint16_t*)FLASHSIZE_BASE);
    uint32_t flash_size_bytes = flash_size_kb * 1024;

    // 2. Compute last valid page
    uint32_t page_count = flash_size_bytes / 2048; // 2 KB per page
    uint32_t last_page_index = page_count - 1;

    // 3. Unlock and clear flags
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    // 4. Erase last page
    FLASH_EraseInitTypeDef EraseInit;
    uint32_t PageError = 0;
    EraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInit.Page = last_page_index;
    EraseInit.NbPages = 1;
    EraseInit.Banks = FLASH_BANK_1;

    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&EraseInit, &PageError);

    HAL_FLASH_Lock();
    return status;
}



HAL_StatusTypeDef Save_Config_To_Flash(const AppConfig_t *config)
{

    HAL_StatusTypeDef status;
    FLASH_EraseInitTypeDef erase_init = {0};
    uint32_t page_error = 0;

    // 1. Disable Caches (prevents CPU from serving stale Flash reads)
    __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();
    __HAL_FLASH_DATA_CACHE_DISABLE();

    // 2. Unlock Flash
    HAL_FLASH_Unlock();

    // 3. Clear flags (prevents false errors on STM32L4)
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);




    // 4. Configure page erase
    erase_init.TypeErase   = FLASH_TYPEERASE_PAGES;
    erase_init.Banks       = FLASH_BANK_1;
    erase_init.Page        = CONFIG_FLASH_PAGE;
    erase_init.NbPages     = 1;

    // 5. Erase Page with Interrupts Disabled
    __disable_irq();
    status = HAL_FLASHEx_Erase(&erase_init, &page_error);
    __enable_irq();

    if (status != HAL_OK) {
    	uint32_t error_code = HAL_FLASH_GetError(); // Inspect this in debugger
    	log_message(LOG_LEVEL_ERROR, "FLASH", "Error Code: %x\n\r", error_code);
        HAL_FLASH_Lock();
        return status;
    }

    // 6. Verify Page is completely 0xFF before writing
    volatile uint64_t *flash_ptr = (uint64_t *)CONFIG_FLASH_ADDRESS;
    if (*flash_ptr != 0xFFFFFFFFFFFFFFFFULL) {
      HAL_FLASH_Lock();
      return HAL_ERROR; // Memory is still dirty, do not attempt program
    }

    // 7. Write Data in 64-bit Double-Word Chunks
    uint64_t write_buffer = 0;
    uint8_t *src_ptr = (uint8_t *)config;
    uint32_t bytes_left = sizeof(AppConfig_t);
    uint32_t current_address = CONFIG_FLASH_ADDRESS;
    uint32_t num_double_words = (sizeof(AppConfig_t) + 7) / 8;

    for (uint32_t i = 0; i < num_double_words; i++) {
      write_buffer = 0xFFFFFFFFFFFFFFFFULL; // Default to erased state padding
      uint32_t chunk_size = (bytes_left >= 8) ? 8 : bytes_left;
      memcpy(&write_buffer, src_ptr, chunk_size);

      status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, current_address, write_buffer);
      if (status != HAL_OK) {
        break;
      }

    current_address += 8;
    src_ptr += chunk_size;
    bytes_left -= chunk_size;
  }



    // 8. Lock Flash & Clear Flags
    HAL_FLASH_Lock();



    return status;


}

void Load_Config_From_Flash(AppConfig_t *config)
{
    // Directly copy from Flash memory address into local memory structure
    memcpy(config, (const void *)CONFIG_FLASH_ADDRESS, sizeof(AppConfig_t));
}




void DisablePTT() {
	HAL_GPIO_WritePin(TX_PTT_GPIO_Port, TX_PTT_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(TX_PTT_LED_GPIO_Port, TX_PTT_LED_Pin, GPIO_PIN_RESET);
}

void EnablePTT() {
	HAL_GPIO_WritePin(TX_PTT_GPIO_Port, TX_PTT_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(TX_PTT_LED_GPIO_Port, TX_PTT_LED_Pin, GPIO_PIN_SET);
}


void StartAPRS() {
	HAL_StatusTypeDef halStatus;
	DAC_ChannelConfTypeDef sConfig = {0};

 	log_message(LOG_LEVEL_INFO,"APRS", "APRS Mode Started\n\r");
    //printf("APRS Mode 1 Selected\n\r");
 	log_message(LOG_LEVEL_INFO,"APRS", "Using Tx Frequency : %.4f\n\r", txFreq);
    //printf("\tUsing Tx Frequency : %.4f\n\r", txFreq);

	HAL_TIM_Base_Stop(&htim6);
	// 1. Specify the configuration parameters
	sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
	sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;		// <-- CHANGE TRIGGER SOURCE HERE
	sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;
	sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_ENABLE;
	sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;

    // 2. Re-initialize the DAC Channel with the new trigger source
	// (Change DAC_CHANNEL_1 to DAC_CHANNEL_2 if using the second channel)
//	if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
	halStatus =  HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1);
	if (halStatus != HAL_OK)
	{
//	  Error_Handler();
		printf("HAL_DAC_ConfigChannel() ERROR: %x\n\r", halStatus);
		HAL_Delay(1000);
		TRIGGER_ERROR(ERROR_DAC);
	}

	// Change timer frequency on the fly
	__HAL_TIM_SET_PRESCALER(&htim6, 0);      // Set PSC to 0 (divide clock by 80)
	__HAL_TIM_SET_AUTORELOAD(&htim6, 2082);  // Set ARR to 2082 (count 1000 steps)


	// 1. STAGING: Modify the configuration structures
	// CHANGE THESE TWO LINES TO DataWidth:
	// Note: PDATAALIGN for Peripheral, MDATAALIGN for Memory
	hdma_dac_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_dac_ch1.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;

	// 2. HARDWARE INITIALIZATION: Commit the configuration to physical registers
	// You MUST call this so the hardware applies the HALFWORD settings!
	HAL_DMA_Init(&hdma_dac_ch1);

	// 3. POINTER LINKING: Link the DAC and DMA structures together
    hdac1.DMA_Handle1 = &hdma_dac_ch1;
    hdma_dac_ch1.Parent = &hdac1;

    // 4. RESET STATE: Clear peripheral flags to avoid an immediate false interrupt
    __HAL_DMA_CLEAR_FLAG(&hdma_dac_ch1, __HAL_DMA_GET_TC_FLAG_INDEX(&hdma_dac_ch1));
    __HAL_DMA_CLEAR_FLAG(&hdma_dac_ch1, __HAL_DMA_GET_HT_FLAG_INDEX(&hdma_dac_ch1));

    // 5. PERIPHERAL STARTUP: Start the actual streaming
    // Only call this AFTER steps 1-4 are completely done.

    // Start DAC in DMA mode
    // This automatically links TIM6 trigger -> DAC hardware -> DMA requests
	HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_8B_R, DAC_REST);
    HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1,
    		                  (uint32_t*) send_buff,
							  sizeof(send_buff) / sizeof(send_buff[0]),    //AFSK_SEND_BUFF_SIZE,
							  DAC_ALIGN_8B_R);

    HAL_TIM_Base_Start_IT(&htim6);				// 26uS Timer


}



void StopAPRS() {
//	HAL_StatusTypeDef halStatus;

  log_message(LOG_LEVEL_INFO,"APRS", "APRS Mode Stopped\n\r");
  HAL_TIM_Base_Stop(&htim6);
  HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);

}

void StartDTMF() {
	HAL_StatusTypeDef halStatus;

	DAC_ChannelConfTypeDef sConfig = {0};

 	log_message(LOG_LEVEL_INFO,"DTMF", "DTMF Mode Started\n\r");
    //printf("APRS Mode 1 Selected\n\r");
 	log_message(LOG_LEVEL_INFO,"DTMF", "Using Tx Frequency : %.4f\n\r", txFreq);
    //printf("\tUsing Tx Frequency : %.4f\n\r", txFreq);

	HAL_TIM_Base_Stop(&htim6);
	// 1. Specify the configuration parameters
	sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
	sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;		// <-- CHANGE TRIGGER SOURCE HERE
	sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;
	sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_ENABLE;
	sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;

    // 2. Re-initialize the DAC Channel with the new trigger source
	// (Change DAC_CHANNEL_1 to DAC_CHANNEL_2 if using the second channel)
//	if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
	halStatus =  HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1);
	if (halStatus != HAL_OK)
	{
//	  Error_Handler();
		printf("HAL_DAC_ConfigChannel() ERROR: %x\n\r", halStatus);
		HAL_Delay(1000);
		TRIGGER_ERROR(ERROR_DAC);
	}

	// Change timer frequency on the fly
	__HAL_TIM_SET_PRESCALER(&htim6, 0);   // Set PSC to 79 (divide clock by 80)
	__HAL_TIM_SET_AUTORELOAD(&htim6, 1599);  // Set ARR to 999 (count 1000 steps)


	// 1. STAGING: Modify the configuration structures
	// CHANGE THESE TWO LINES TO DataWidth:
	// Note: PDATAALIGN for Peripheral, MDATAALIGN for Memory
	hdma_dac_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	hdma_dac_ch1.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;

	// 2. HARDWARE INITIALIZATION: Commit the configuration to physical registers
	// You MUST call this so the hardware applies the HALFWORD settings!
	HAL_DMA_Init(&hdma_dac_ch1);

	// 3. POINTER LINKING: Link the DAC and DMA structures together
	hdac1.DMA_Handle1 = &hdma_dac_ch1;
	hdma_dac_ch1.Parent = &hdac1;

	// 4. RESET STATE: Clear peripheral flags to avoid an immediate false interrupt
	__HAL_DMA_CLEAR_FLAG(&hdma_dac_ch1, __HAL_DMA_GET_TC_FLAG_INDEX(&hdma_dac_ch1));
	__HAL_DMA_CLEAR_FLAG(&hdma_dac_ch1, __HAL_DMA_GET_HT_FLAG_INDEX(&hdma_dac_ch1));

	// 5. PERIPHERAL STARTUP: Start the actual streaming
	// Only call this AFTER steps 1-4 are completely done.
    HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1,
    		                  (uint32_t*)dac_buffer,
							  sizeof(dac_buffer) / sizeof(dac_buffer[0]),
							  DAC_ALIGN_12B_R);

	HAL_TIM_Base_Start_IT(&htim6);

    // Start DAC in DMA mode
    // This automatically links TIM6 trigger -> DAC hardware -> DMA requests
//    HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)dac_buffer, BUFFER_SIZE, DAC_ALIGN_12B_R);

//    HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
//    HAL_TIM_Base_Start_IT(&htim6);				// 50uS Timer


}

void StopDTMF() {
//	HAL_StatusTypeDef halStatus;

	log_message(LOG_LEVEL_INFO,"DTMF", "DTMF Mode Stopped\n\r");
    // 200ms has elapsed! Stop the playback
    HAL_Delay(DTMF_DURATION);
    HAL_TIM_Base_Stop(&htim6);
    HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);
#if 0
    halStatus = HAL_OPAMP_Stop(&hopamp1);
	if (halStatus != HAL_OK)
	{
//	  Error_Handler();
		printf("HAL_OPAMP_Stop() ERROR: %x\n\r", halStatus);
		HAL_Delay(1000);
     	TRIGGER_ERROR(ERROR_OPAMP);
	}
#endif
#if 0
    if (HAL_OPAMP_Stop(&hopamp1) != HAL_OK) {
    	TRIGGER_ERROR(ERROR_OPAMP);
    }
#endif
}

void APRS_Send(APRSPacket *pack) {
   	  log_message(LOG_LEVEL_INFO,"APRS", "APRS Packet sent\n\r");
      //printf("APRS Mode 1 Selected\n\r");
   	  log_message(LOG_LEVEL_INFO,"APRS", "Using Tx Frequency : %.4f\n\r", txFreq);
      //printf("\tUsing Tx Frequency : %.4f\n\r", txFreq);
      if (aprsInit == 0) {
  	    BitFIFO_init(&tx_fifo, malloc(APRS_FIFO_BYTES), APRS_FIFO_BYTES);
  	    tx_pack.dest = malloc(APRS_SIGN_LEN);
  	    tx_pack.callsign = malloc(APRS_SIGN_LEN);
  	    tx_pack.digi = malloc(APRS_SIGN_LEN * APRS_MAX_DIGI);
  	    tx_pack.info = malloc(APRS_MAX_INFO + 1);

        HAL_OPAMP_Start(&hopamp1);
        AFSK_init();
        APRS_init();

        strcpy(tx_pack.dest, APRS_DEFAULT_PACKET.dest);
        strcpy(tx_pack.callsign, APRS_DEFAULT_PACKET.callsign);
        strcpy(tx_pack.digi, APRS_DEFAULT_PACKET.digi);
        strcpy(tx_pack.info, APRS_DEFAULT_PACKET.info);
  	    aprsInit = 1;
      }

      // N0CALL>APCSS,WIDE1-1:>MODE=?
      strcpy(tx_pack.callsign, pack->callsign);
      strcpy(tx_pack.dest, pack->dest);
      strcpy(tx_pack.digi, pack->digi);
      strcpy(tx_pack.info, pack->info);
      APRS_print(&tx_pack);

      EnablePTT();
      StartAPRS();
      APRS_encode(&tx_fifo, &tx_pack);
      AFSK_send(&tx_fifo);

}



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
////  uint32_t num = 0;
  GPIO_PinState pin_state;
  dtmf_dma_F = 0;
  dtmf_dma_H = 0;
  aprs_dma_F = 0;
  aprs_dma_H = 0;
  HAL_StatusTypeDef status;


  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_DAC1_Init();
  MX_OPAMP1_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  setbuf(stdin, NULL);
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);
  dma_printf_init(&huart2);
  dma_scanf_init(&huart2);

  log_set_level(LOG_LEVEL_INFO);
  // Clear the screen and home the cursor
  printf("\033[2J\033[H");

  printf("\n\rProject %s\n\r", PROJECT_NAME);
  printf("Build at %s %s\r\n", __DATE__, __TIME__);


  // set PTT High (Rx Mode)
  HAL_GPIO_WritePin(TX_PTT_GPIO_Port, TX_PTT_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(TX_PTT_LED_GPIO_Port, TX_PTT_LED_Pin, GPIO_PIN_RESET);

  for(int i=0; i<5; i++) {
    HAL_GPIO_WritePin(TX_PTT_LED_GPIO_Port, TX_PTT_LED_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(STATUS_GPIO_Port, STATUS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(APRS_VALID_GPIO_Port, APRS_VALID_Pin, GPIO_PIN_SET);
    HAL_Delay(250);
    HAL_GPIO_WritePin(TX_PTT_LED_GPIO_Port, TX_PTT_LED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(STATUS_GPIO_Port, STATUS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(APRS_VALID_GPIO_Port, APRS_VALID_Pin, GPIO_PIN_RESET);
    HAL_Delay(250);
  }



  ///////////////////////////////////////////////////////////////////
  // Load Config from Flash
  // Load saved settings
  // To clear FLASH->SR flags before new FLASH operations.

  log_message(LOG_LEVEL_DEBUG, "FLASH", "FLASH->SR: %x\n\r", FLASH->SR);
//  printf("FLASH->SR: %x\n\r", FLASH->SR);
  if (FLASH->SR != 0)
	  status = Flash_EraseLastPage();
//  printf("Status : %x\n\r", status);
  log_message(LOG_LEVEL_DEBUG, "FLASH", "Status : %x\n\r", status);
  log_message(LOG_LEVEL_DEBUG, "FLASH", "FLASH->SR: %x\n\r", FLASH->SR);
//  printf("FLASH->SR: %x\n\r", FLASH->SR);




  Load_Config_From_Flash(&sysConfig);
  log_message(LOG_LEVEL_DEBUG,"CONFIG", "Checksum: %x\n\r", sysConfig.checksum);
  // Check if configuration is unitialized (Flash reads 0xFFFFFFFF when erased)
  if (sysConfig.checksum != 0x5A5A5A5A) {
	  log_message(LOG_LEVEL_DEBUG,"CONFIG", "User Config Initializing\n\r");
      // Load default values if uninitialized
      sysConfig.version = 1;
      strcpy(sysConfig.device_name, "GroundStation-01");
      sysConfig.txFreq = TX_FREQ;
      sysConfig.rxFreq = RX_FREQ;

      sysConfig.sq_level = 0;
      sysConfig.af_level = 6;

	  txFreq = sysConfig.txFreq;
	  rxFreq = sysConfig.rxFreq;

      sysConfig.checksum = 0x5A5A5A5A;

      // Save default settings to flash
      status = Save_Config_To_Flash(&sysConfig);
      if (status != HAL_OK) {
    	  log_message(LOG_LEVEL_ERROR,"CONFIG", "Flash Save Error: %d\n\r", status);
      }
  } else {
	log_message(LOG_LEVEL_DEBUG,"CONFIG", "User Config initialized\n\r");
	log_message(LOG_LEVEL_DEBUG,"CONFIG", "Name: %s\n\r", sysConfig.device_name);
	log_message(LOG_LEVEL_DEBUG,"CONFIG", "Rx Freq: %f\n\r", sysConfig.rxFreq);
	log_message(LOG_LEVEL_DEBUG,"CONFIG", "Tx Freq: %f\n\r", sysConfig.txFreq);
	log_message(LOG_LEVEL_DEBUG,"CONFIG", "SQ Level: %d\n\r", sysConfig.sq_level);
	log_message(LOG_LEVEL_DEBUG,"CONFIG", "AF Level: %d\n\r", sysConfig.af_level);


	// Configure Frequencies for SR105U
	txFreq = sysConfig.txFreq;
	rxFreq = sysConfig.rxFreq;
  }



  Load_Config_From_Flash(&sysConfig);
  log_message(LOG_LEVEL_DEBUG,"CONFIG", "Checksum: %lx\n\r", sysConfig.checksum);

  ///////////////////////////////////////////////////////////////////
  HAL_TIM_Base_Start_IT(&htim7);				// 50uS Timer

  // Read from PA12 (MODE)
  pin_state = HAL_GPIO_ReadPin(MODE_GPIO_Port, MODE_Pin);
  if (pin_state == GPIO_PIN_SET) {
	ModeState = MODE_APRS;
	log_message(LOG_LEVEL_INFO,"MODE","APRS Mode \n\r");
  }	else {
	ModeState = MODE_DTMF;
	log_message(LOG_LEVEL_INFO,"MODE","DTMF Mode \n\r");
  }



  HAL_Delay(1000);




  ConfigSR105U_Freq(rxFreq, txFreq);
  HAL_Delay(500);
  ConfigSR105U_SQ(sysConfig.sq_level);
  HAL_Delay(500);

  ConfigSR105U_Vol(sysConfig.af_level);
  HAL_Delay(500);

  /* Start the OPAMP so it connects the internal DAC to the external pin */

   if (HAL_OPAMP_Start(&hopamp1) != HAL_OK) {
       Error_Handler();
   }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  if (aprsInit == 0) {
   	  BitFIFO_init(&tx_fifo, malloc(APRS_FIFO_BYTES), APRS_FIFO_BYTES);
   	  tx_pack.dest = malloc(APRS_SIGN_LEN);
   	  tx_pack.callsign = malloc(APRS_SIGN_LEN);
   	  tx_pack.digi = malloc(APRS_SIGN_LEN * APRS_MAX_DIGI);
   	  tx_pack.info = malloc(APRS_MAX_INFO + 1);

      HAL_OPAMP_Start(&hopamp1);
      AFSK_init();
      APRS_init();

      strcpy(tx_pack.dest, APRS_DEFAULT_PACKET.dest);
      strcpy(tx_pack.callsign, APRS_DEFAULT_PACKET.callsign);
      strcpy(tx_pack.digi, APRS_DEFAULT_PACKET.digi);
      strcpy(tx_pack.info, APRS_DEFAULT_PACKET.info);


   	  aprsInit = 1;

  }
  init_sine_lut();
  // Prime the entire buffer with baseline data (silence) before starting
  fill_buffer_half(0, BUFFER_SIZE);


//  run_main_menu(MENU_TIMEOUT); 		// Timeout of MENU_TIMEOUT (30000mS).
//  run_command_menu(MENU_TIMEOUT);


  printf("(M/m) for main menu. (C/c) for sending command\n\r");
  printf(" or Press BTN to send command \n\r");
  while(dma_getc_nonblocking() >= 0)
	  ;

  typedef enum {
      ACTIVE_MENU_NONE = 0,
      ACTIVE_MENU_MAIN,
      ACTIVE_MENU_COMMAND
  } ActiveMenu_t;

  ActiveMenu_t active_menu = ACTIVE_MENU_NONE;

  while (1) {
	// 1. Unblocked APRS Packet Handling
    if (APRS_decoded_packet_valid == 1) {
      log_message(LOG_LEVEL_INFO, "APRS", "VALID APRS %ld\r\n", HAL_GetTick());
	  log_message(LOG_LEVEL_INFO, "APRS", "dest  [%s]\r\n", APRS_decoded_packet.dest);
	  log_message(LOG_LEVEL_INFO, "APRS", "callsign  [%s]\r\n", APRS_decoded_packet.callsign);
	  log_message(LOG_LEVEL_INFO, "APRS", "digi = [%s]\r\n", APRS_decoded_packet.digi);
	  log_message(LOG_LEVEL_INFO, "APRS", "info = [%s]\r\n", APRS_decoded_packet.info);

	  APRS_decoded_packet_valid = 0;
    }

    if (APRS_send_done == true) {
    	APRS_send_done = false;
    	log_message(LOG_LEVEL_INFO,"APRS", "APRS Frame sends completely.\n\r");
    	 StopAPRS() ;
    }
    if (send_Pressed == 1) {
        send_Pressed = 0;
    	GPIO_PinState sel0, sel1, sel2;
    	uint8_t sel;
    	// User Pressed Send Button
    	log_message(LOG_LEVEL_INFO,"SEND","SEND Pressed\n\r");
        pin_state = HAL_GPIO_ReadPin(MODE_GPIO_Port, MODE_Pin);
        sel0 = HAL_GPIO_ReadPin(SELECT_0_GPIO_Port, SELECT_0_Pin);
        sel1 = HAL_GPIO_ReadPin(SELECT_1_GPIO_Port, SELECT_1_Pin);
        sel2 = HAL_GPIO_ReadPin(SELECT_2_GPIO_Port, SELECT_2_Pin);

        sel = ((sel2<<2) + (sel1<<1) + sel0);


        if (pin_state == GPIO_PIN_SET) {
          log_message(LOG_LEVEL_INFO,"MODE","APRS Mode, Sel: %d \n\r", sel);
       	  ModeState = MODE_APRS;

      	  switch(sel) {
      	  case 0:
        	  log_message(LOG_LEVEL_INFO,"APRS","APRS Mode 1\n\r");
		      strcpy(tx_pack.callsign, "NOCALL");
		      strcpy(tx_pack.dest, "APCSS");
		      strcpy(tx_pack.digi, "WIDE1-1");
		      strcpy(tx_pack.info, ">MODE=a");
         	  APRS_Send(&tx_pack);
      		  break;
          case 1:
        	  log_message(LOG_LEVEL_INFO,"APRS","APRS Mode 2\n\r");

		      strcpy(tx_pack.callsign, "NOCALL");
		      strcpy(tx_pack.dest, "APCSS");
		      strcpy(tx_pack.digi, "WIDE1-1");
		      strcpy(tx_pack.info, ">MODE=f");
         	  APRS_Send(&tx_pack);
		      break;
          case 2:
        	  log_message(LOG_LEVEL_INFO,"APRS","APRS Mode 3\n\r");

		      strcpy(tx_pack.callsign, "NOCALL");
		      strcpy(tx_pack.dest, "APCSS");
		      strcpy(tx_pack.digi, "WIDE1-1");
		      strcpy(tx_pack.info, ">MODE=b");
         	  APRS_Send(&tx_pack);

		      break;
          case 3:
        	  log_message(LOG_LEVEL_INFO,"APRS","APRS Mode 4\n\r");

		      strcpy(tx_pack.callsign, "NOCALL");
		      strcpy(tx_pack.dest, "APCSS");
		      strcpy(tx_pack.digi, "WIDE1-1");
		      strcpy(tx_pack.info, ">MODE=s");
         	  APRS_Send(&tx_pack);

		      break;
          case 4:
        	  log_message(LOG_LEVEL_INFO,"APRS","APRS Mode 5\n\r");

		      strcpy(tx_pack.callsign, "NOCALL");
		      strcpy(tx_pack.dest, "APCSS");
		      strcpy(tx_pack.digi, "WIDE1-1");
		      strcpy(tx_pack.info, ">MODE=m");
         	  APRS_Send(&tx_pack);

		      break;
          case 5:
        	  log_message(LOG_LEVEL_INFO,"APRS","APRS Mode 6\n\r");

		      strcpy(tx_pack.callsign, "NOCALL");
		      strcpy(tx_pack.dest, "APCSS");
		      strcpy(tx_pack.digi, "WIDE1-1");
		      strcpy(tx_pack.info, ">MODE=e");
         	  APRS_Send(&tx_pack);

		      break;
          case 6:
        	  log_message(LOG_LEVEL_INFO,"APRS","APRS Mode 7\n\r");

		      strcpy(tx_pack.callsign, "NOCALL");
		      strcpy(tx_pack.dest, "APCSS");
		      strcpy(tx_pack.digi, "WIDE1-1");
		      strcpy(tx_pack.info, ">MODE=j");
         	  APRS_Send(&tx_pack);

		      break;
          case 7:
        	  log_message(LOG_LEVEL_INFO,"APRS","APRS Mode 8\n\r");

		      strcpy(tx_pack.callsign, "NOCALL");
		      strcpy(tx_pack.dest, "APCSS");
		      strcpy(tx_pack.digi, "WIDE1-1");
		      strcpy(tx_pack.info, ">MODE=o");
         	  APRS_Send(&tx_pack);

		      break;


          default:
        	  log_message(LOG_LEVEL_INFO,"ERROR","Unknown Selected Mode: %d\n\r", sel);
      	  }

        }	else {
          log_message(LOG_LEVEL_INFO,"MODE","DTMF Mode, Sel: %d \n\r", sel);
       	  ModeState = MODE_DTMF;

      	  switch(sel) {
      	  case 0:
        	  log_message(LOG_LEVEL_INFO,"DTMF","DTMF Mode 1\n\r");

		      EnablePTT();
		      StartDTMF();
		      play_dtmf_tone_Delay('1', TONE_DURATION_MS);
		      play_dtmf_tone_Delay('#', TONE_DURATION_MS);
		      play_dtmf_tone_Delay(' ', TONE_DURATION_MS);
		      StopDTMF();
		      DisablePTT();
      		  break;
          case 1:
        	  log_message(LOG_LEVEL_INFO,"DTMF","DTMF Mode 2\n\r");

		      EnablePTT();
		      StartDTMF();
		      play_dtmf_tone_Delay('2', TONE_DURATION_MS);
		      play_dtmf_tone_Delay('#', TONE_DURATION_MS);
		      play_dtmf_tone_Delay(' ', TONE_DURATION_MS);
		      StopDTMF();
		      DisablePTT();

		      break;
          case 2:
        	  log_message(LOG_LEVEL_INFO,"DTMF","DTMF Mode 3\n\r");

		      EnablePTT();
		      StartDTMF();
		      play_dtmf_tone_Delay('3', TONE_DURATION_MS);
		      play_dtmf_tone_Delay('#', TONE_DURATION_MS);
		      play_dtmf_tone_Delay(' ', TONE_DURATION_MS);
		      StopDTMF();
		      DisablePTT();
		      break;
          case 3:
        	  log_message(LOG_LEVEL_INFO,"DTMF","DTMF Mode 4\n\r");

		      EnablePTT();
		      StartDTMF();
		      play_dtmf_tone_Delay('4', TONE_DURATION_MS);
		      play_dtmf_tone_Delay('#', TONE_DURATION_MS);
		      play_dtmf_tone_Delay(' ', TONE_DURATION_MS);
		      StopDTMF();
		      DisablePTT();
		      break;
          case 4:
        	  log_message(LOG_LEVEL_INFO,"DTMF","DTMF Mode 5\n\r");

		      EnablePTT();
		      StartDTMF();
		      play_dtmf_tone_Delay('5', TONE_DURATION_MS);
		      play_dtmf_tone_Delay('#', TONE_DURATION_MS);
		      play_dtmf_tone_Delay(' ', TONE_DURATION_MS);
		      StopDTMF();
		      DisablePTT();
		      break;
          case 5:
        	  log_message(LOG_LEVEL_INFO,"DTMF","DTMF Mode 6\n\r");

		      EnablePTT();
		      StartDTMF();
		      play_dtmf_tone_Delay('6', TONE_DURATION_MS);
		      play_dtmf_tone_Delay('#', TONE_DURATION_MS);
		      play_dtmf_tone_Delay(' ', TONE_DURATION_MS);
		      StopDTMF();
		      DisablePTT();
		      break;
          case 6:
        	  log_message(LOG_LEVEL_INFO,"DTMF","DTMF Mode 7\n\r");

		      EnablePTT();
		      StartDTMF();
		      play_dtmf_tone_Delay('7', TONE_DURATION_MS);
		      play_dtmf_tone_Delay('#', TONE_DURATION_MS);
		      play_dtmf_tone_Delay(' ', TONE_DURATION_MS);
		      StopDTMF();
		      DisablePTT();
		      break;
          case 7:
        	  log_message(LOG_LEVEL_INFO,"DTMF","DTMF Mode 8\n\r");

		      EnablePTT();
		      StartDTMF();
		      play_dtmf_tone_Delay('8', TONE_DURATION_MS);
		      play_dtmf_tone_Delay('#', TONE_DURATION_MS);
		      play_dtmf_tone_Delay(' ', TONE_DURATION_MS);
		      StopDTMF();
		      DisablePTT();
		      break;

          default:
        	  log_message(LOG_LEVEL_INFO,"ERROR","Unknown Selected Mode: %d\n\r", sel);
      	  }
        }
		printf("(M/m) for main menu. (C/c) for sending command\n\r");
		printf(" or Press BTN to send command \n\r");

    }
	// 2. Read input ONCE per loop cycle
	int key = dma_getc_nonblocking();

	// 3. Routing & State Management
	switch (active_menu) {
	  case ACTIVE_MENU_NONE:
	    // Example activation condition:
	    // Press 'm' to open Main Menu, 'c' to open Command Menu
	    if (key == 'm' || key == 'M') {
	      active_menu = ACTIVE_MENU_MAIN;
	      run_main_menu_nonblocking(-1, 1); // Trigger open
		  printf("(M/m) for main menu. (C/c) for sending command\n\r");
 		  printf(" or Press BTN to send command \n\r");
	    } else if (key == 'c' || key == 'C') {
	      active_menu = ACTIVE_MENU_COMMAND;
	      run_command_menu_nonblocking(-1, 1); // Trigger open
		  printf("(M/m) for main menu. (C/c) for sending command\n\r");
		  printf(" or Press BTN to send command \n\r");
	    }

	    break;

	  case ACTIVE_MENU_MAIN:
	      // Tick main menu. If it returns 0, it has timed out or exited.
	      if (!run_main_menu_nonblocking(key, 0)) {
	        active_menu = ACTIVE_MENU_NONE;
			  printf("(M/m) for main menu. (C/c) for sending command\n\r");
			  printf(" or Press BTN to send command \n\r");
	      }
	      break;

	  case ACTIVE_MENU_COMMAND:
	      // Tick command menu. If it returns 0, it has timed out or exited.
	      if (!run_command_menu_nonblocking(key, 0)) {
	        active_menu = ACTIVE_MENU_NONE;
			  printf("(M/m) for main menu. (C/c) for sending command\n\r");
			  printf(" or Press BTN to send command \n\r");
	      }

	      break;
	}

	HAL_Delay(10);

  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_8B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T2_TRGO;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC1_Init(void)
{

  /* USER CODE BEGIN DAC1_Init 0 */

  /* USER CODE END DAC1_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */

  /** DAC Initialization
  */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_T7_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_ENABLE;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC1_Init 2 */

  /* USER CODE END DAC1_Init 2 */

}

/**
  * @brief OPAMP1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_OPAMP1_Init(void)
{

  /* USER CODE BEGIN OPAMP1_Init 0 */

  /* USER CODE END OPAMP1_Init 0 */

  /* USER CODE BEGIN OPAMP1_Init 1 */

  /* USER CODE END OPAMP1_Init 1 */
  hopamp1.Instance = OPAMP1;
  hopamp1.Init.PowerSupplyRange = OPAMP_POWERSUPPLY_LOW;
  hopamp1.Init.Mode = OPAMP_FOLLOWER_MODE;
  hopamp1.Init.NonInvertingInput = OPAMP_NONINVERTINGINPUT_DAC_CH;
  hopamp1.Init.PowerMode = OPAMP_POWERMODE_NORMALPOWER;
  hopamp1.Init.UserTrimming = OPAMP_TRIMMING_FACTORY;
  if (HAL_OPAMP_Init(&hopamp1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN OPAMP1_Init 2 */

  /* USER CODE END OPAMP1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_4BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 15999;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 499;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OnePulse_Init(&htim1, TIM_OPMODE_SINGLE) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 8332;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 0;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 2082;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 80;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 19;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
  /* DMA1_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
  /* DMA1_Channel7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SPI_NSS_Pin|TX_PTT_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, APRS_VALID_Pin|STATUS_Pin|LD3_Pin|TX_PTT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : SELECT_1_Pin SELECT_2_Pin */
  GPIO_InitStruct.Pin = SELECT_1_Pin|SELECT_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI_NSS_Pin TX_PTT_LED_Pin */
  GPIO_InitStruct.Pin = SPI_NSS_Pin|TX_PTT_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : APRS_VALID_Pin STATUS_Pin LD3_Pin TX_PTT_Pin */
  GPIO_InitStruct.Pin = APRS_VALID_Pin|STATUS_Pin|LD3_Pin|TX_PTT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : MODE_Pin SELECT_0_Pin */
  GPIO_InitStruct.Pin = MODE_Pin|SELECT_0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : SEND_BTN_Pin */
  GPIO_InitStruct.Pin = SEND_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SEND_BTN_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */




#define PERIOD 10000
#define ON_TIME 100
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
  static uint32_t c;
  /* USER CODE END Callback 0 */
//  if (htim->Instance == TIM3) {
//    HAL_IncTick();
//  }
  /* USER CODE BEGIN Callback 1 */
  if (htim->Instance == TIM7) {
    c++;
    if ( c == PERIOD-ON_TIME) {
	  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
    } else if (c == PERIOD) {
      HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
      flag = 1;
      c = 0;
    }
  } //else if (htim->Instance == TIM6) {
//	  //HAL_GPIO_TogglePin(PIO_PULSE_GPIO_Port, PIO_PULSE_Pin);
//  }

  if (htim->Instance == TIM1) {
      // 1. Turn LED OFF
      HAL_GPIO_WritePin(APRS_VALID_GPIO_Port, APRS_VALID_Pin, GPIO_PIN_RESET);

      // 2. Stop Timer Interrupt
      HAL_TIM_Base_Stop_IT(htim);
  }
  /* USER CODE END Callback 1 */
}






int __io_putchar(int ch)
{
  dma_printf_putc(ch&0xFF);
  return ch;
}

int __io_getchar(void)
{
  return dma_scanf_getc_blocking();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
  // Check if the interrupt came from USART1
  if (huart->Instance == USART2) {
     // Handle USART2 specific TX completion
  }
  // Check if it came from USART2
  else if (huart->Instance == USART1) {
    // Do something else for USART1, or do nothing
	rxBuffer[rxIndex++] = rxChar[0];

	// Example: Stop reading if we hit a newline or fill the buffer
	if (rxChar[0] == 0X0A || rxIndex >= 49) {
	  rxBuffer[rxIndex] = '\0'; // Null terminate string
	  rxCompleteFlag = 1;       // Signal main loop we are done
	} else {
	  // Re-arm interrupt for the next byte
	  HAL_UART_Receive_IT(&huart1, rxChar, 1);
	}
  }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
  if (huart->Instance == USART2) {
    dma_printf_send_it(huart);
  }
  if (huart->Instance == USART1) {
    txCompleteFlag = 1;
  }
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	static uint32_t last_interrupt_time = 0;
	uint32_t current_time = HAL_GetTick();

    if (GPIO_Pin == GPIO_PIN_5)
    {
      // 200 ms debounce window
      if(current_time - last_interrupt_time > 200) {
        // Execute action
      	// Add your interrupt code here
        //HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
//      	printf("SEND Pressed\n\r");
      	send_Pressed = 1;
//////////////////////////////////
#if 0
        if (ModeState == MODE_APRS) {
        	 APRS_Send(">MODE=a");

       	  log_message(LOG_LEVEL_INFO,"BTNSEND", "APRS Mode 1 Selected\n\r");
          //printf("APRS Mode 1 Selected\n\r");
       	  log_message(LOG_LEVEL_INFO,"BTNSEND", "Using Tx Frequency : %.4f\n\r", txFreq);
          //printf("\tUsing Tx Frequency : %.4f\n\r", txFreq);
          if (aprsInit == 0) {
      	    BitFIFO_init(&tx_fifo, malloc(APRS_FIFO_BYTES), APRS_FIFO_BYTES);
      	    tx_pack.dest = malloc(APRS_SIGN_LEN);
      	    tx_pack.callsign = malloc(APRS_SIGN_LEN);
      	    tx_pack.digi = malloc(APRS_SIGN_LEN * APRS_MAX_DIGI);
      	    tx_pack.info = malloc(APRS_MAX_INFO + 1);
      	    aprsInit = 1;
          }

          HAL_OPAMP_Start(&hopamp1);
          AFSK_init();
          APRS_init();

          strcpy(tx_pack.dest, APRS_DEFAULT_PACKET.dest);
          strcpy(tx_pack.callsign, APRS_DEFAULT_PACKET.callsign);
          strcpy(tx_pack.digi, APRS_DEFAULT_PACKET.digi);
          strcpy(tx_pack.info, APRS_DEFAULT_PACKET.info);

          EnablePTT();

          // N0CALL>APCSS,WIDE1-1:>MODE=a
          strcpy(tx_pack.callsign, "NOCALL");
          strcpy(tx_pack.dest, "APCSS");
          strcpy(tx_pack.digi, "WIDE1-1");
          strcpy(tx_pack.info, ">MODE=a");

          APRS_print(&tx_pack);

          StartAPRS();
          APRS_encode(&tx_fifo, &tx_pack);
          AFSK_send(&tx_fifo);


        } else  if (ModeState == MODE_DTMF) {
        	log_message(LOG_LEVEL_INFO,"BTNSEND", "DTMF Mode, no signal is generated.\n\r");
            //printf("DTMF Mode, no signal is generated.\n\r");
        }
//////////////////////////////////
#endif
        last_interrupt_time = current_time;
      }

    }
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  uint8_t error_code;
  __disable_irq();


  // Default to 1 blink if no specific code was set

  switch(global_error_code) {
    case NO_ERROR:
	  error_code = 1;
	  break;
    case ERROR_TIMER:
	  error_code = 2;
	  break;
    case ERROR_DAC:
	  error_code = 3;
	  break;
    case ERROR_UART:
	  error_code = 4;
	  break;
    case ERROR_OPAMP:
	  error_code = 5;
	  break;

  }

  volatile uint32_t count;

  while (1)
  {
      for (uint8_t i = 0; i < error_code; i++)
      {
          HAL_GPIO_WritePin(STATUS_GPIO_Port, STATUS_Pin, GPIO_PIN_SET);
          count = 14000000; // ~250ms at 170MHz
          __asm__ volatile ("1: subs %0, %0, #1 \n\t bne 1b" : "+r" (count) :: "cc");

          HAL_GPIO_WritePin(STATUS_GPIO_Port, STATUS_Pin, GPIO_PIN_RESET);
          count = 14000000;
          __asm__ volatile ("1: subs %0, %0, #1 \n\t bne 1b" : "+r" (count) :: "cc");
      }

      // Long pause between sequences
      count = 110000000;
      __asm__ volatile ("1: subs %0, %0, #1 \n\t bne 1b" : "+r" (count) :: "cc");
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
