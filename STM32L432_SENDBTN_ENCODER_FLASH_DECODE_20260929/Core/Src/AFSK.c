/*
 * AFSK.c
 *
 *  Created on: Jul 14, 2020
 *      Author: matthewtran
 */

#include "AFSK.h"

const uint8_t AFSK_SINE_LOOKUP[256] = {
	128, 131, 134, 137, 140, 143, 146, 149, 152, 156, 159, 162, 165, 168, 171, 174,
	176, 179, 182, 185, 188, 191, 193, 196, 199, 201, 204, 206, 209, 211, 213, 216,
	218, 220, 222, 224, 226, 228, 230, 232, 234, 236, 237, 239, 240, 242, 243, 245,
	246, 247, 248, 249, 250, 251, 252, 252, 253, 254, 254, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 254, 254, 253, 252, 252, 251, 250, 249, 248, 247,
	246, 245, 243, 242, 240, 239, 237, 236, 234, 232, 230, 228, 226, 224, 222, 220,
	218, 216, 213, 211, 209, 206, 204, 201, 199, 196, 193, 191, 188, 185, 182, 179,
	176, 174, 171, 168, 165, 162, 159, 156, 152, 149, 146, 143, 140, 137, 134, 131,
	128, 124, 121, 118, 115, 112, 109, 106, 103,  99,  96,  93,  90,  87,  84,  81,
	79,  76,  73,  70,  67,  64,  62,  59,  56,  54,  51,  49,  46,  44,  42,  39,
	37,  35,  33,  31,  29,  27,  25,  23,  21,  19,  18,  16,  15,  13,  12,  10,
	9,   8,   7,   6,   5,   4,   3,   3,   2,   1,   1,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   1,   1,   2,   3,   3,   4,   5,   6,   7,   8,
	9,  10,  12,  13,  15,  16,  18,  19,  21,  23,  25,  27,  29,  31,  33,  35,
	37,  39,  42,  44,  46,  49,  51,  54,  56,  59,  62,  64,  67,  70,  73,  76,
	79,  81,  84,  87,  90,  93,  96,  99, 103, 106, 109, 112, 115, 118, 121, 124
};

#define DAC_REST 127 // based on AFSK_SINE_LOOKUP

// REQUIREMENTS
// Timers used for DAC and ADC are set to overflow at FS
// DMA request for DMA set in circular mode

// constants
#define SEND_FREQ_SHIFT    5 // samples_per_bit = 1 << FREQ_SHIFT
#define RECV_FREQ_SHIFT    3
#define BAUD            1200 // bps
#define FREQ_MARK       1200 // Hz, 1
#define FREQ_SPACE      2200 // Hz, 0
#define RECV_DELAY         4 // k in sin(n)sin(n-k) decoding
#define ADC_REST         127 // center value of ADC

// derived constants
#define SEND_FS   (BAUD << SEND_FREQ_SHIFT) // Hz, BAUD << FREQ_SHIFT
#define RECV_FS   (BAUD << RECV_FREQ_SHIFT)
#define DPH_MARK                  134217728 // FREQ_MARK  / FS * (1 << 32) (using 2*pi rad = 2^32)
#define DPH_SPACE                 246065835 // FREQ_SPACE / FS * (1 << 32) (using 2*pi rad = 2^32)

// variables
extern uint8_t send_buff[2048]; // DAC, needs to be multiple of 1 << (FREQ_MUL+1)
extern uint32_t phase;
extern BitFIFO *send_fifo;

//uint8_t recv_buff[2048]; // ADC, 0.11s latency, too large and packet might be overwritten
uint8_t recv_buff[2048]; // ADC, 0.11s latency, too large and packet might be overwritten
static int32_t prev_pll, pll;
static bool prev_bit, curr_nrzi;
#define SIG_FIFO_SIZE sizeof(int32_t) * (RECV_DELAY + 1) // bytes, 1 int32_t wasted
static IntFIFO sig_fifo;

static int32_t x_prev, x_curr; // x[n-1], x[n]
static int32_t y_prev, y_curr; // y[n-1], y[n]

bool   APRS_send_done = false;

extern bool AFSK_sending;
extern DMA_HandleTypeDef hdma_dac_ch1;
extern MODE_State ModeState;


// functions
void AFSK_init() {



	// Change Timer, DMA, DAC configurations for APRS
#if 0
	DAC_ChannelConfTypeDef sConfig = {0};

	HAL_TIM_Base_Stop(&htim6);
	// 1. Specify the configuration parameters
	sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
	sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;		// <-- CHANGE TRIGGER SOURCE HERE
	sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;
	sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_ENABLE;
	sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;

    // 2. Re-initialize the DAC Channel with the new trigger source
	// (Change DAC_CHANNEL_1 to DAC_CHANNEL_2 if using the second channel)
	if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
	{
	  Error_Handler();
	}



	// Change timer frequency on the fly
	__HAL_TIM_SET_PRESCALER(&htim6, 0);   // Set PSC to 79 (divide clock by 80)
	__HAL_TIM_SET_AUTORELOAD(&htim6, 2082);  // Set ARR to 999 (count 1000 steps)
	// Reconfig DMA
	// CHANGE THESE TWO LINES TO DataWidth:
	// Note: PDATAALIGN for Peripheral, MDATAALIGN for Memory
	hdma_dac_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_dac_ch1.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;


	// Start sending
	HAL_TIM_Base_Start_IT(&htim6);
	HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_8B_R, DAC_REST);
	HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
#endif

	AFSK_sending = false;
	// DSP variables
	prev_pll = 0;
	pll = 0;
	prev_bit = false;
	curr_nrzi = true;
	IntFIFO_init(&sig_fifo, malloc(SIG_FIFO_SIZE), SIG_FIFO_SIZE);
	for (uint32_t i = 0; i < RECV_DELAY; i++) {
		IntFIFO_push(&sig_fifo, 0);
	}
	// Start receiving
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) recv_buff, sizeof(recv_buff));
	HAL_TIM_Base_Start(&htim2);

}

bool AFSK_send(BitFIFO *bfifo) {
	if (AFSK_sending) {
		return false;
	}

	send_fifo = bfifo;
	phase = 0;
	AFSK_sending = 1;
	AFSK_send_fillbuff(send_buff, sizeof(send_buff));

//    EnablePTT();
//	HAL_GPIO_WritePin(TX_PTT_GPIO_Port, TX_PTT_Pin, 0);
//	HAL_GPIO_WritePin(TX_PTT_LED_GPIO_Port, TX_PTT_LED_Pin, 1);

	HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*) send_buff, AFSK_SEND_BUFF_SIZE, DAC_ALIGN_8B_R);

	return true;
}

// private helpers
void AFSK_send_fillbuff(uint8_t *buff, uint32_t samples) {
	uint32_t sig_index = 0;
	for (uint32_t i = 0; i < (samples >> SEND_FREQ_SHIFT); i++) {
		if (BitFIFO_is_empty(send_fifo)) {
			break;
		}

		uint32_t dph = BitFIFO_pop(send_fifo) ? DPH_MARK : DPH_SPACE;
		for (sig_index = i << SEND_FREQ_SHIFT; sig_index < ((i + 1) << SEND_FREQ_SHIFT); sig_index++) {
			phase += dph;
			buff[sig_index] = AFSK_SINE_LOOKUP[phase >> 24]; // 8-bit sine lookup
		}
	}

	for (; sig_index < samples; sig_index++) {
		buff[sig_index] = DAC_REST; // at rest level if no more bits to send
	}
}

// Static filter state variables (keep across buffer calls)
static int32_t bpf_x1 = 0, bpf_x2 = 0, bpf_y1 = 0, bpf_y2 = 0; // Pre-BPF
static int32_t lpf_x1 = 0, lpf_x2 = 0, lpf_y1 = 0, lpf_y2 = 0; // Post-LPF
static int32_t dc_level = 0;                                 // Adaptive Slicer

void AFSK_decode_sig(uint8_t *sig_buff, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        // 1. Remove DC Offset
        int32_t raw_sig = (int32_t)sig_buff[i] - ADC_REST;

        // 2. Pre-Bandpass Filter (Center ~1700Hz, fs=9600Hz)
        // Strips out-of-band noise before quadratic multiplication
        int32_t sig = raw_sig - bpf_x2 + ((bpf_y1 * 187) >> 8) - ((bpf_y2 * 150) >> 8);
        bpf_x2 = bpf_x1;
        bpf_x1 = raw_sig;
        bpf_y2 = bpf_y1;
        bpf_y1 = sig;

        // 3. Delay-Product Demodulation
        x_prev = x_curr;
        x_curr = (IntFIFO_pop(&sig_fifo) * sig) >> 2;
        IntFIFO_push(&sig_fifo, sig);

        // 4. 2nd-Order Lowpass Filter (~1200Hz Cutoff, fs=9600Hz)
        // Stronger attenuation than original 1st-order filter
        int32_t lpf_in = x_curr;
        y_curr = lpf_in + (lpf_x1 << 1) + lpf_x2 + ((lpf_y1 * 133) >> 8) - ((lpf_y2 * 83) >> 8);
        lpf_x2 = lpf_x1;
        lpf_x1 = lpf_in;
        lpf_y2 = lpf_y1;
        lpf_y1 = y_curr;

        // 5. Adaptive DC Baseline Tracking & Dynamic Slicing
        // Slowly tracks low-frequency noise offset instead of assuming rigid '0'
        dc_level += (y_curr - dc_level) >> 9;
        bool bit = y_curr > dc_level;

        // 6. PLL, NRZI2NRZ, Sample (Unchanged)
        if (bit != prev_bit) {
        	pll -= (pll >> 3); // 12.5% correction step instead of 50%
//            pll >>= 1; // divide by 2 to nudge to 0
        }
        prev_pll = pll;
        pll += 1 << (32 - RECV_FREQ_SHIFT);
        if (pll < 0 && prev_pll > 0) { // overflow, so SAMPLE!
            APRS_decode_update(curr_nrzi == bit);
            curr_nrzi = bit;
        }
        prev_bit = bit;
    }
}

void AFSK_decode_sig1(uint8_t *sig_buff, uint32_t len) {
	for (uint32_t i = 0; i < len; i++) {
		// simplified demodulation and 1200Hz IIR filter, designed for 9600Hz sample rate
		int32_t sig = sig_buff[i] - ADC_REST;
		x_prev = x_curr;
		x_curr = (IntFIFO_pop(&sig_fifo) * sig) >> 2; // x[0]*0.29289322, precalculating
		y_prev = y_curr;
		y_curr = x_prev + x_curr + (y_prev >> 1); // x[0]*0.29289322 + x[-1]*0.29289322 + y[-1]*0.41421356
		bool bit = y_curr > 0;
		IntFIFO_push(&sig_fifo, sig);

		// PLL, NRZI2NRZ, sample
		if (bit != prev_bit) {
			pll >>= 1; // divide by 2 to nudge to 0
		}
		prev_pll = pll;
		pll += 1 << (32 - RECV_FREQ_SHIFT);
		if (pll < 0 && prev_pll > 0) { // overflow, so SAMPLE!
			APRS_decode_update(curr_nrzi == bit); // also do NRZI2NRZ while you at it
			curr_nrzi = bit;
		}
		prev_bit = bit;
	}
}


///////////////////////////////////
// Called when the first half of the buffer (indices 0 to 49) has finished transmitting
void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* hdac) {
//    fill_buffer_half(0, BUFFER_SIZE / 2);
	  uint32_t ms_ticks;

	  if (hdac->Instance == DAC1) {
		ms_ticks = HAL_GetTick();
	    if (ModeState == MODE_APRS) {
		  if (!AFSK_sending) {
		    HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);
		    HAL_DAC_Start(&hdac1, DAC_CHANNEL_1); // stopping DMA stops DAC too
		    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_8B_R, DAC_REST);
		    DisablePTT();
		    APRS_send_done = true;
		    //		    HAL_GPIO_WritePin(TX_PTT_GPIO_Port, TX_PTT_Pin, 1);
		    //		    HAL_GPIO_WritePin(TX_PTT_LED_GPIO_Port, TX_PTT_LED_Pin, 0);
		  }

	  	  AFSK_send_fillbuff(send_buff, sizeof(send_buff) >> 1);
	      aprs_dma_H++;
		  if (BitFIFO_is_empty(send_fifo)) {
		    AFSK_sending = false;
		  }
	    } else if(ModeState == MODE_DTMF) {
		  fill_buffer_half(0, BUFFER_SIZE / 2);

	      dacValue.value[0] = dac_buffer[0];
	      dacValue.value[1] = dac_buffer[1];
	      dacValue.ts = ms_ticks;
	      dtmf_dma_H++;
	    }
	  }

}

// Called when the second half of the buffer (indices 50 to 99) has finished transmitting
void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac) {
//    fill_buffer_half(BUFFER_SIZE / 2, BUFFER_SIZE);
	  uint32_t ms_ticks;
	  ms_ticks = HAL_GetTick();
	  if (hdac->Instance == DAC1) {
	    if(ModeState == MODE_APRS) {
		  if (!AFSK_sending) {
			HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);
			HAL_DAC_Start(&hdac1, DAC_CHANNEL_1); // stopping DMA stops DAC too
			HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_8B_R, DAC_REST);
		    DisablePTT();
		    APRS_send_done = true;
//			HAL_GPIO_WritePin(TX_PTT_GPIO_Port, TX_PTT_Pin, 1);
//			HAL_GPIO_WritePin(TX_PTT_LED_GPIO_Port, TX_PTT_LED_Pin, 0);
		  }

	  	  AFSK_send_fillbuff(send_buff + (sizeof(send_buff) >> 1), sizeof(send_buff) >> 1);
	      aprs_dma_F++;
		  if (BitFIFO_is_empty(send_fifo)) {
			AFSK_sending = false;
		  }
	    } else if(ModeState == MODE_DTMF) {
		  fill_buffer_half(BUFFER_SIZE / 2, BUFFER_SIZE);

	      dacValue.value[0] = dac_buffer[0];
	      dacValue.value[1] = dac_buffer[1];
	      dacValue.ts = ms_ticks;
	      dtmf_dma_F++;
	    }
	  }
}

// The correct HAL callback for DAC2 Channel 1 Half-Transfer
void HAL_DACEx_ConvHalfCpltCallbackCh2(DAC_HandleTypeDef* hdac) {
    if (hdac->Instance == DAC1) {
        fill_buffer_half(0, BUFFER_SIZE / 2);
    }
}

// The correct HAL callback for DAC2 Channel 1 Full-Transfer
void HAL_DACEx_ConvCpltCallbackCh2(DAC_HandleTypeDef* hdac) {
    if (hdac->Instance == DAC1) {
        fill_buffer_half(BUFFER_SIZE / 2, BUFFER_SIZE);
    }
}


///////////////////////////////////
// receiving interrupts
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
	AFSK_decode_sig(recv_buff, sizeof(recv_buff) >> 1);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	AFSK_decode_sig(recv_buff + (sizeof(recv_buff) >> 1), sizeof(recv_buff) >> 1);
}


// receiving interrupts
