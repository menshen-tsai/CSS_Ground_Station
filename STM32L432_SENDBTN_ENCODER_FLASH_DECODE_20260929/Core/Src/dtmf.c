/*
 * dtmf.c
 *
 *  Created on: 2026年6月30日
 *      Author: USER
 */


#include "main.h"
#include "dtmf.h"


extern TIM_HandleTypeDef htim4;

uint32_t sine_lut[LUT_SIZE];

// DTMF Frequencies
const float row_freqs[4] = {697.0f, 770.0f, 852.0f, 941.0f};
const float col_freqs[4] = {1209.0f, 1336.0f, 1477.0f, 1633.0f};



volatile uint32_t phase_acc_row = 0;
volatile uint32_t phase_acc_col = 0;
volatile uint32_t phase_inc_row = 0;
volatile uint32_t phase_inc_col = 0;



// --- Sequence state ---
static const char *dtmf_sequence = NULL;
static uint32_t current_index = 0;
static bool playing = false;

// --- Flags set by ISR ---
volatile bool tone_done_flag = false;
volatile bool gap_done_flag  = false;

// --- Debug timer flag ---
//volatile uint32_t num = 0;

DTMF_State dtmf_state = STATE_IDLE;

///uint16_t dac_buffer[BUFFER_SIZE];
extern uint32_t dac_buffer[BUFFER_SIZE];

// Initialize the Sine Lookup Table
void init_sine_lut(void) {
    for (int i = 0; i < LUT_SIZE; i++) {
        // Generate sine wave scaled for 12-bit DAC
        sine_lut[i] = (uint32_t)((sinf((2.0f * M_PI * i) / LUT_SIZE) + 1.0f) * AMPLITUDE + OFFSET);
    }
}

// Call this function whenever you want to change the playing key
void set_dtmf_key(char key) {
    int row = -1, col = -1;

    // Find coordinates in matrix
    switch(key) {
        case '1': row = 0; col = 0; break;
        case '2': row = 0; col = 1; break;
        case '3': row = 0; col = 2; break;
        case 'A': row = 0; col = 3; break;
        case '4': row = 1; col = 0; break;
        case '5': row = 1; col = 1; break;
        case '6': row = 1; col = 2; break;
        case 'B': row = 1; col = 3; break;
        case '7': row = 2; col = 0; break;
        case '8': row = 2; col = 1; break;
        case '9': row = 2; col = 2; break;
        case 'C': row = 2; col = 3; break;
        case '*': row = 3; col = 0; break;
        case '0': row = 3; col = 1; break;
        case '#': row = 3; col = 2; break;
        case 'D': row = 3; col = 3; break;
        default: // Mute
            phase_inc_row = 0;
            phase_inc_col = 0;
//        	row = 0;
//        	col = 0;
            return;
    }

    // Calculate Phase Increments
    // Formula: Phase_Inc = (Freq * LUT_SIZE * 2^16) / Sampling_Rate
    // We scale by 2^16 (shift left 16) for fixed-point precision accumulator
    phase_inc_row = (uint32_t)((row_freqs[row] * LUT_SIZE * 65536.0f) / SAMPLING_RATE);
    phase_inc_col = (uint32_t)((col_freqs[col] * LUT_SIZE * 65536.0f) / SAMPLING_RATE);
}


void fill_buffer_half(uint32_t start_idx, uint32_t end_idx) {
    for (uint32_t i = start_idx; i < end_idx; i++) {
        if (phase_inc_row == 0 && phase_inc_col == 0) {
            // Muted state: Fill buffer with mid-rail silent voltage
            dac_buffer[i] = 2048;
        } else {
            // Advance phase accumulators
            phase_acc_row += phase_inc_row;
            phase_acc_col += phase_inc_col;

            uint16_t idx_row = (phase_acc_row >> 16) % LUT_SIZE;
            uint16_t idx_col = (phase_acc_col >> 16) % LUT_SIZE;

            // Mix signals
            dac_buffer[i] = sine_lut[idx_row] + sine_lut[idx_col];
        }
    }
}

void play_dtmf_tone(char key, uint32_t duration_ms) {
    set_dtmf_key(key);        // start tone
    dtmf_state = STATE_TONE;

    __HAL_TIM_SET_COUNTER(&htim4, 0);
    __HAL_TIM_SET_AUTORELOAD(&htim4, duration_ms*10-1);
    HAL_TIM_Base_Start_IT(&htim4);  // ISR will set tone_done_flag
}


void play_dtmf_tone_Delay(char key, uint32_t duration_ms) {
    // 1. Set the frequencies for the desired key
    set_dtmf_key(key);

    // 2. Wait for the specified duration while DMA plays the tone in the background
    HAL_Delay(duration_ms);

    // 3. Mute the tone by changing the key to a space/invalid character
    set_dtmf_key(' ');
}


void play_dtmf_sequence_Delay(const char *sequence, uint32_t tone_ms, uint32_t gap_ms) {
    for (size_t i = 0; i < strlen(sequence); i++) {
        char key = sequence[i];

        // Play the tone for the requested duration
        play_dtmf_tone_Delay(key, tone_ms);

        // Insert a gap (silence) between digits
        HAL_Delay(gap_ms);
    }
}

// --- Internal helpers ---
void start_next_tone(void) {
    if (!playing || dtmf_sequence == NULL) return;

    if (current_index < strlen(dtmf_sequence)) {
        char key = dtmf_sequence[current_index++];
        //set_dtmf_key(key);
        play_dtmf_tone(key, TONE_DURATION_MS);

    } else {
        // sequence complete
    	dtmf_state = STATE_IDLE;
        playing = false;
        set_dtmf_key(' '); // mute
    }
}


void play_dtmf_sequence(const char *sequence) {
    dtmf_sequence = sequence;
    current_index = 0;
    playing = true;
    start_next_tone();
}





void schedule_gap(void) {
    dtmf_state = STATE_GAP;

    __HAL_TIM_SET_COUNTER(&htim4, 0);
    __HAL_TIM_SET_AUTORELOAD(&htim4, GAP_DURATION_MS*10-1);
    HAL_TIM_Base_Start_IT(&htim4);
}


void process_dtmf_player(void) {
    if (tone_done_flag == true) {
        tone_done_flag = false;
        set_dtmf_key(' ');      // mute safely outside ISR
        dtmf_state = STATE_GAP;
        schedule_gap();
    }

    if (gap_done_flag == true) {
        gap_done_flag = false;
        start_next_tone();      // safe outside ISR
    }
}
