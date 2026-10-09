/*
 * AFSK.h
 *
 *  Created on: Jul 13, 2020
 *      Author: matthewtran
 */

#ifndef INC_AFSK_H_
#define INC_AFSK_H_

#include "main.h"
#include <stdbool.h>
#include <malloc.h>

#include "BitFIFO.h"
#include "IntFIFO.h"
#include "APRS.h"


// Private Defines
#define DAC_REST           127 // based on AFSK_SINE_LOOKUP

#define SEND_FREQ_SHIFT      5 // samples_per_bit = 1 << FREQ_SHIFT
#define RECV_FREQ_SHIFT      3
#define BAUD              1200 // bps
#define FREQ_MARK         1200 // Hz, 1
#define FREQ_SPACE        2200 // Hz, 0
#define RECV_DELAY           4 // k in sin(n)sin(n-k) decoding


// derived constants
#define SEND_FS   (BAUD << SEND_FREQ_SHIFT) // Hz, BAUD << FREQ_SHIFT
#define RECV_FS   (BAUD << RECV_FREQ_SHIFT)
#define DPH_MARK                  134217728 // FREQ_MARK  / FS * (1 << 32) (using 2*pi rad = 2^32)
#define DPH_SPACE                 246065835 // FREQ_SPACE / FS * (1 << 32) (using 2*pi rad = 2^32)

#define AFSK_SEND_BUFF_SIZE			2048

void AFSK_init();
bool AFSK_send(BitFIFO *bfifo);

// private helpers
void AFSK_send_fillbuff(uint8_t *buff, uint32_t samples);
void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac);
void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac);

void AFSK_decode_sig(uint8_t *sig_buff, uint32_t len);

#endif /* INC_AFSK_H_ */
