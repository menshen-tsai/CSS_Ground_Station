/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "global.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void DisablePTT();
void EnablePTT();
void StartAPRS(void);
void StopAPRS(void);
void StartDTMF(void);
void StopDTMF(void);

HAL_StatusTypeDef Save_Config_To_Flash(const AppConfig_t *config);
void Load_Config_From_Flash(AppConfig_t *config);

void APRS_Send(APRSPacket *pack) ;
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SELECT_1_Pin GPIO_PIN_0
#define SELECT_1_GPIO_Port GPIOA
#define VCP_TX_Pin GPIO_PIN_2
#define VCP_TX_GPIO_Port GPIOA
#define SPI_NSS_Pin GPIO_PIN_4
#define SPI_NSS_GPIO_Port GPIOA
#define APRS_VALID_Pin GPIO_PIN_0
#define APRS_VALID_GPIO_Port GPIOB
#define STATUS_Pin GPIO_PIN_1
#define STATUS_GPIO_Port GPIOB
#define TX_PTT_LED_Pin GPIO_PIN_8
#define TX_PTT_LED_GPIO_Port GPIOA
#define SELECT_2_Pin GPIO_PIN_12
#define SELECT_2_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define VCP_RX_Pin GPIO_PIN_15
#define VCP_RX_GPIO_Port GPIOA
#define LD3_Pin GPIO_PIN_3
#define LD3_GPIO_Port GPIOB
#define MODE_Pin GPIO_PIN_4
#define MODE_GPIO_Port GPIOB
#define SEND_BTN_Pin GPIO_PIN_5
#define SEND_BTN_GPIO_Port GPIOB
#define SEND_BTN_EXTI_IRQn EXTI9_5_IRQn
#define TX_PTT_Pin GPIO_PIN_6
#define TX_PTT_GPIO_Port GPIOB
#define SELECT_0_Pin GPIO_PIN_7
#define SELECT_0_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

#define DTMF_DURATION  200			// 200mS of Tone

#define TRIGGER_ERROR(code) do { \
    global_error_code = (code);  \
    Error_Handler();             \
} while(0)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
