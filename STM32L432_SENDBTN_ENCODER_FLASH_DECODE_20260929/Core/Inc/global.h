/*
 * global.h
 *
 *  Created on: 2026年7月14日
 *      Author: Tsai
 */

#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include <stdarg.h>

#include "dtmf.h"
#include "BitFIFO.h"
#include "APRS.h"



typedef struct  {
    uint32_t version;
//    uint32_t device_id;
//    float    calib_factor;
    char     device_name[32];
    float 	 txFreq;
    float	 rxFreq;
    uint32_t sq_level;
    uint32_t af_level;
    uint32_t checksum; // Simple validation flag/checksum
} AppConfig_t;




#include "AFSK.h"


// DECLARATION ONLY: No memory is allocated here.
// This is safe to include in 100+ source files.

typedef enum {
    MODE_DTMF,
    MODE_APRS,
} MODE_State;

typedef enum {
    NO_ERROR,
    ERROR_TIMER,
	ERROR_DAC,
	ERROR_UART,
	ERROR_OPAMP,
} ERROR_Code;

typedef enum {
    DEBUG_NONE,
    DEBUG_INFO,
	DEBUG_WARNING,
	DEBUG_ERROR
} DEBUG_Info;


typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_NONE   // disable all logs
} LogLevel;

struct DacValue {
	uint32_t value[2];
	uint32_t ts;
};



extern LogLevel currentLogLevel;


extern struct DacValue dacValue;


extern ADC_HandleTypeDef hadc1;
extern DAC_HandleTypeDef hdac1;
extern DMA_HandleTypeDef hdma_dac_ch1;

extern OPAMP_HandleTypeDef hopamp1;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;




extern ERROR_Code global_error_code;
extern float txFreq, rxFreq;

///uint16_t dac_buffer[BUFFER_SIZE];
extern uint32_t dac_buffer[BUFFER_SIZE];

extern MODE_State ModeState;

// Configuration
extern AppConfig_t sysConfig;

// Variables for SR105U
extern volatile uint8_t txCompleteFlag;
extern volatile uint8_t rxCompleteFlag;
extern uint8_t rxChar[1];
extern uint8_t rxBuffer[50];
extern uint16_t rxIndex;

// Variables for APRS
extern BitFIFO      tx_fifo;
extern APRSPacket   tx_pack;
extern bool         AFSK_sending;
extern uint8_t      send_buff[2048]; // DAC, needs to be multiple of 1 << (FREQ_MUL+1)
extern uint32_t     phase;
extern BitFIFO      *send_fifo;
extern uint8_t      aprsInit;
extern bool         APRS_send_done;
extern uint8_t      recv_buff[2048];
// Debug Variables
extern DEBUG_Info debugInfo;
extern uint32_t dtmf_dma_F;
extern uint32_t dtmf_dma_H;
extern uint32_t aprs_dma_F;
extern uint32_t aprs_dma_H;


void log_set_level(LogLevel level);
void log_message(LogLevel level, const char *tag, const char *fmt, ...);
void aprs_led_pulse(void) ;

#endif /* INC_GLOBAL_H_ */
