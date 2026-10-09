/*
 * sr105u.c
 *
 *  Created on: 2026年7月14日
 *      Author: Tsai
 */

#include "global.h"
#include "main.h"

#include "SR105U.h"




// extern ERROR_Code global_error_code;

// Config SR105U Tx and Rx frequency
void ConfigSR105U_Freq(float rx_freq, float tx_freq) {
  char buffer[80];
  uint32_t startTime;

  rxChar[0] = 0;
  rxIndex = 0;

  for (int i=0; i<50; i++) {
	rxBuffer[i] = 0;
  }

  log_message(LOG_LEVEL_INFO,"SR105U", "Config SR105U Freq: RX: %.4f, TX: %.4f\n\r", rxFreq, txFreq);
//  printf("Config SR105U Tx and Rx Frequencies\n\r");
//  printf("   Current Frequency Settings:\n\r");
//  printf("      RX Freq: %.4f\n\r", rxFreq);
//  printf("      TX Freq: %.4f\n\r", txFreq);



  // 1. Reset flags
  txCompleteFlag = 0;
  rxCompleteFlag = 0;
  rxIndex = 0;

  // 2. Start the non-blocking transmission


  sprintf(buffer, SR105_CONFIG_FREQ, rx_freq, tx_freq);
//  printf("Sending %s\n\r", buffer);
  log_message(LOG_LEVEL_DEBUG,"SR105U", "Sending %s\n\r", buffer);

  HAL_UART_Transmit_IT(&huart1, (uint8_t*) buffer, strlen(buffer));
  // 3. Wait for transmission to actually finish (usually microseconds)

  startTime = HAL_GetTick();
  while (txCompleteFlag == 0) {
    // Optional: add a tiny safety timeout here so you don't hang forever
    // if hardware fails.
    if ((HAL_GetTick() - startTime) >= SR105U_TRANSMIT_TIMEOUT) {
        HAL_UART_AbortReceive(&huart1); // Stop listening
        break; // Exit loop due to timeout
    }
  }

  // 3.1. Evaluate the outcome
  if (txCompleteFlag == 1) {
    // Success! Response received within 1 second.
    log_message(LOG_LEVEL_DEBUG,"SR105U", "CMD Transmitted\n\r");
//	printf("CMD Transmitted\n\r");
  } else {
    // Timeout error handling.
	log_message(LOG_LEVEL_ERROR,"SR105U", "CMD Transmit Timeout\n\r");
////	printf("CMD Transmit Timeout\n\r");
	// Call Error_Handler() to indicate Error of Sending CMD to SR105U
	TRIGGER_ERROR(ERROR_UART);
  }


  // 4. Start listening for the response immediately
  //    Should received: +DMOSETGROUP=0<CR><LF>
  HAL_UART_Receive_IT(&huart1, rxChar, 1);

  // 5. Start 10-second countdown clock
  startTime = HAL_GetTick();

  while (rxCompleteFlag == 0) {
    // Check if 1000ms has elapsed
    if ((HAL_GetTick() - startTime) >= SR105U_RESPONSE_TIMEOUT) {
        HAL_UART_AbortReceive(&huart1); // Stop listening
        break; // Exit loop due to timeout
    }
  }

  // 6. Evaluate the outcome
  if (rxCompleteFlag == 1) {
    // Success! Response received within 1 second.
	log_message(LOG_LEVEL_INFO, "SR105U",   "Response Received : %s\n\r", rxBuffer);
//	printf("Response Received : %s\n\r", rxBuffer);

	// clean up 4 trail NULLs
	HAL_UART_Receive(&huart1, rxChar, 1, 1000);
	HAL_UART_Receive(&huart1, rxChar, 1, 1000);
	HAL_UART_Receive(&huart1, rxChar, 1, 1000);
	HAL_UART_Receive(&huart1, rxChar, 1, 1000);
  } else {
    // Timeout error handling.
	log_message(LOG_LEVEL_ERROR,"SR105U", "CMD Response Timeout\n\r");
	// Call Error_Handler() to indicate Error of Sending CMD to SR105U
  }

}

void ConfigSR105U_SQ(uint32_t sq_level) {
  char buffer[80];
  uint32_t startTime;

  rxChar[0] = 0;
  rxIndex = 0;
  rxCompleteFlag = 0;
  for (int i=0; i<50; i++) {
	rxBuffer[i] = 0;
  }

  log_message(LOG_LEVEL_INFO,"SR105U", "Config SR105U SQ Level: %ld\n\r", sq_level);
//  printf("Config SR105U SQ Level\n\r");
//  printf("   Current SQ Level Setting:\n\r");
//  printf("      SQ: %ld\n\r", sq_level);

  /////////////////////////////////////////////
  // Config SQ level
  // AT+DMOFUN=SQ,MICLVL,TOT,SCRAMLVL,COMP

  sprintf(buffer, SR105_CONFIG_FUN, sq_level);
  //printf("Sending %s\n\r", buffer);
  log_message(LOG_LEVEL_DEBUG,"SR105U", "Sending %s\n\r", buffer);

  HAL_UART_Transmit_IT(&huart1, (uint8_t*) buffer, strlen(buffer));
  // 3. Wait for transmission to actually finish (usually microseconds)

  startTime = HAL_GetTick();
  while (txCompleteFlag == 0) {
    // Optional: add a tiny safety timeout here so you don't hang forever
    // if hardware fails.
    if ((HAL_GetTick() - startTime) >= SR105U_TRANSMIT_TIMEOUT) {
        HAL_UART_AbortReceive(&huart1); // Stop listening
        break; // Exit loop due to timeout
    }
  }

  // 3.1. Evaluate the outcome
  if (txCompleteFlag == 1) {
    // Success! Response received within 1 second.
//	printf("CMD Transmitted\n\r");
    log_message(LOG_LEVEL_DEBUG,"SR105U", "CMD Transmitted\n\r");
  } else {
    // Timeout error handling.
//	printf("CMD Transmit Timeout\n\r");
	log_message(LOG_LEVEL_ERROR,"SR105U", "CMD Transmit Timeout\n\r");
	// Call Error_Handler() to indicate Error of Sending CMD to SR105U
	TRIGGER_ERROR(ERROR_UART);
  }


  // 4. Start listening for the response immediately
  //    Should received: +DMOFUN=0<CR><LF>
  HAL_UART_Receive_IT(&huart1, rxChar, 1);

  // 5. Start 10-second countdown clock
  startTime = HAL_GetTick();

  while (rxCompleteFlag == 0) {
    // Check if 1000ms has elapsed
    if ((HAL_GetTick() - startTime) >= SR105U_RESPONSE_TIMEOUT) {
        HAL_UART_AbortReceive(&huart1); // Stop listening
        break; // Exit loop due to timeout
    }
  }

  // 6. Evaluate the outcome
  if (rxCompleteFlag == 1) {
    // Success! Response received within 1 second.
//	printf("Response Received : %s\n\r", rxBuffer);
	log_message(LOG_LEVEL_INFO, "SR105U",   "Response Received : %s\n\r", rxBuffer);
	// clean up 4 trail NULLs
	HAL_UART_Receive_IT(&huart1, rxChar, 1);
	HAL_UART_Receive_IT(&huart1, rxChar, 1);
	HAL_UART_Receive_IT(&huart1, rxChar, 1);
	HAL_UART_Receive_IT(&huart1, rxChar, 1);
  } else {
    // Timeout error handling.
//	printf("Response Timeout\n\r");
	log_message(LOG_LEVEL_ERROR,"SR105U", "CMD Response Timeout\n\r");
	// Call Error_Handler() to indicate Error of Sending CMD to SR105U
  }
}

void ConfigSR105U_Vol(uint32_t afout_level) {
  char buffer[80];
  uint32_t startTime;


  rxChar[0] = 0;
  rxIndex = 0;
  rxCompleteFlag = 0;

  for (int i=0; i<50; i++) {
	rxBuffer[i] = 0;
  }

  /////////////////////////////////////////////
  // Config AF_OUT level
  // AT+DMOVOL=X


  log_message(LOG_LEVEL_INFO,"SR105U", "Config SR105U Audio Level: %ld\n\r", afout_level);

  sprintf(buffer, SR105_CONFIG_VOL, afout_level);
  //printf("Sending %s\n\r", buffer);
  log_message(LOG_LEVEL_DEBUG,"SR105U", "Sending %s\n\r", buffer);


  HAL_UART_Transmit_IT(&huart1, (uint8_t*) buffer, strlen(buffer));
  // 3. Wait for transmission to actually finish (usually microseconds)

  startTime = HAL_GetTick();
  while (txCompleteFlag == 0) {
    // Optional: add a tiny safety timeout here so you don't hang forever
    // if hardware fails.
    if ((HAL_GetTick() - startTime) >= SR105U_TRANSMIT_TIMEOUT) {
        HAL_UART_AbortReceive(&huart1); // Stop listening
        break; // Exit loop due to timeout
    }
  }

  // 3.1. Evaluate the outcome
  if (txCompleteFlag == 1) {
    // Success! Response received within 1 second.
//	printf("CMD Transmitted\n\r");
    log_message(LOG_LEVEL_DEBUG,"SR105U", "CMD Transmitted\n\r");
  } else {
    // Timeout error handling.
//	printf("CMD Transmit Timeout\n\r");
	log_message(LOG_LEVEL_ERROR,"SR105U", "CMD Transmit Timeout\n\r");
	// Call Error_Handler() to indicate Error of Sending CMD to SR105U
	TRIGGER_ERROR(ERROR_UART);
  }


  // 4. Start listening for the response immediately
  //    Should received: +DMOSETGROUP=0<CR><LF>
  HAL_UART_Receive_IT(&huart1, rxChar, 1);

  // 5. Start 10-second countdown clock
  startTime = HAL_GetTick();

  while (rxCompleteFlag == 0) {
    // Check if 1000ms has elapsed
    if ((HAL_GetTick() - startTime) >= SR105U_RESPONSE_TIMEOUT) {
        HAL_UART_AbortReceive(&huart1); // Stop listening
        break; // Exit loop due to timeout
    }
  }

  // 6. Evaluate the outcome
  if (rxCompleteFlag == 1) {
    // Success! Response received within 1 second.
//	printf("Response Received : %s\n\r", rxBuffer);
	log_message(LOG_LEVEL_INFO, "SR105U",   "Response Received : %s\n\r", rxBuffer);

	// clean up 4 trail NULLs
	HAL_UART_Receive_IT(&huart1, rxChar, 1);
	HAL_UART_Receive_IT(&huart1, rxChar, 1);
	HAL_UART_Receive_IT(&huart1, rxChar, 1);
	HAL_UART_Receive_IT(&huart1, rxChar, 1);
  } else {
    // Timeout error handling.
//	printf("Response Timeout\n\r");
	log_message(LOG_LEVEL_ERROR,"SR105U", "CMD Response Timeout\n\r");
	// Call Error_Handler() to indicate Error of Sending CMD to SR105U
  }






}
