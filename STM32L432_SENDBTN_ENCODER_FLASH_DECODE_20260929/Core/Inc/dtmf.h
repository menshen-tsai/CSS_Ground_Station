/*
 * dtmf.h
 *
 *  Created on: 2026年6月30日
 *      Author: USER
 */

#ifndef INC_DTMF_H_
#define INC_DTMF_H_

#include "math.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

typedef enum {
    STATE_IDLE,
    STATE_TONE,
    STATE_GAP
} DTMF_State;


#define LUT_SIZE 256
#define SAMPLING_RATE 50000.0f
// 12-bit DAC middle point is 2048. Max peak-to-peak amplitude for mixed signal should be < 2048 to prevent clipping.
#define AMPLITUDE 800
#define OFFSET    2048
// We use a 100-sample buffer.
// Half-buffer size = 50 samples. Every 1ms (50 / 50kHz), the CPU wakes up briefly to fill 50 samples.
#define BUFFER_SIZE 400


#define TONE_DURATION_MS   200   // tone length
#define GAP_DURATION_MS    100   // silence between tones

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif



// --- Flags set by ISR ---



void init_sine_lut(void);
void set_dtmf_key(char key);
void fill_buffer_half(uint32_t start_idx, uint32_t end_idx);

void play_dtmf_tone(char key, uint32_t duration_ms);
void play_dtmf_tone_Delay(char key, uint32_t duration_ms);
void play_dtmf_sequence_Delay(const char *sequence, uint32_t tone_ms, uint32_t gap_ms) ;

void start_next_tone(void) ;
void play_dtmf_sequence(const char *sequence);

void schedule_gap(void) ;
void process_dtmf_player(void) ;



#endif /* INC_DTMF_H_ */
