/*
 * global.c
 *
 *  Created on: 2026年7月14日
 *      Author: Tsai
 */

//#include "main.h"
//#include "SR105U.h"
#include "global.h"

ERROR_Code global_error_code;

LogLevel currentLogLevel = LOG_LEVEL_DEBUG; // default

float txFreq, rxFreq;

///uint16_t dac_buffer[BUFFER_SIZE];
uint32_t dac_buffer[BUFFER_SIZE];

MODE_State ModeState = MODE_DTMF;


// Variables for SR105U
volatile uint8_t txCompleteFlag = 0;
volatile uint8_t rxCompleteFlag = 0;
////volatile uint8_t rxFlag = 0; // Set to 1 when response is complete
uint8_t rxChar[1];
uint8_t rxBuffer[50];
uint16_t rxIndex = 0;

// Variables for APRS
BitFIFO tx_fifo;
APRSPacket tx_pack;
bool AFSK_sending;
uint8_t send_buff[2048]; // DAC, needs to be multiple of 1 << (FREQ_MUL+1)
uint32_t phase;
BitFIFO *send_fifo;
uint8_t aprsInit=0;

// Debug variables
DEBUG_Info debugInfo = DEBUG_NONE;
uint32_t dtmf_dma_F;
uint32_t dtmf_dma_H;
uint32_t aprs_dma_F;
uint32_t aprs_dma_H;
struct DacValue dacValue;

AppConfig_t sysConfig;

void log_set_level(LogLevel level) {
    currentLogLevel = level;
}

void log_message(LogLevel level, const char *tag, const char *fmt, ...)
{
    if (level < currentLogLevel) return;

    const char *levelStr;
    switch (level) {
        case LOG_LEVEL_DEBUG:   levelStr = "DEBUG"; break;
        case LOG_LEVEL_INFO:    levelStr = "INFO"; break;
        case LOG_LEVEL_WARNING: levelStr = "WARN"; break;
        case LOG_LEVEL_ERROR:   levelStr = "ERROR"; break;
        default:                levelStr = ""; break;
    }

    char buffer[256];  // adjust size as needed
    for(int i=0; i< sizeof(buffer); i++)
    	buffer[i] = 0;
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    // 2. Determine raw string length
        size_t raw_len = strlen(buffer);

        // 3. Convert non-printable bytes (preserving the last 2 bytes)
        char print_buffer[1537];
        char *out = print_buffer;
        const unsigned char *in = (const unsigned char *)buffer;

        for (size_t idx = 0; idx < raw_len && (out - print_buffer) < (sizeof(print_buffer) - 7); idx++) {
            unsigned char c = in[idx];

            // Keep as-is if printable OR if it is within the last 2 bytes of the buffer
            if (isprint(c) || idx >= (raw_len > 2 ? raw_len - 2 : 0)) {
                *out++ = c;
            } else {
                out += sprintf(out, "<0x%02X>", c);
            }
        }
        *out = '\0';
    printf("[%s] %s: %s", levelStr, tag, print_buffer);
}
