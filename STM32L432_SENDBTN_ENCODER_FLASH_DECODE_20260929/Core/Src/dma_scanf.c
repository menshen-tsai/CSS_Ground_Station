//
// Created by naoki on 19/03/09.
//

#include "main.h"
#include "dma_ring.h"
#include "dma_scanf.h"
#include "dma_printf.h"

struct dma_scanf_info dsi;

void dma_scanf_init(UART_HandleTypeDef *scanf_huart){
    dsi.huart = scanf_huart;
    dma_ring_init(&dsi.rx_ring);
    HAL_UART_Receive_DMA(dsi.huart, dsi.rx_ring.buf, dsi.rx_ring.buf_size);
}

// U1RxBufferPtrIN = U1RxBufSize - __HAL_DMA_GET_COUNTER(huart7.hdmarx);

int dma_scanf_getc_blocking(){
//	uint16_t countTR = __HAL_DMA_GET_COUNTER(dsi.huart->hdmarx);
    while(dma_ring_available(&dsi.rx_ring) == 0){
        dma_ring_set_w_ptr(&dsi.rx_ring, (uint16_t)((dsi.rx_ring.buf_size - dsi.huart->hdmarx->Instance->CNDTR)&0xFFFF));
////        dma_ring_set_w_ptr(&dsi.rx_ring, (uint16_t)((dsi.rx_ring.buf_size - countTR)&0xFFFF));
    }
    //char c;
    uint8_t c;
    dma_ring_getc(&dsi.rx_ring, &c);
    dma_printf_putc(c);
    return c;
}




/**
 * @brief Non-blocking character fetch from DMA Ring Buffer
 * @return Character (0-255) if available, -1 if buffer is empty
 */
int dma_getc_nonblocking(void) {
    // 1. Sync the hardware state to our software w_ptr
    // NDTR is a down-counter (Size -> 0)
//    uint16_t current_ndtr = (uint16_t)__HAL_DMA_GET_COUNTER(dsi.huart->hdmarx);
    dma_ring_set_w_ptr(&dsi.rx_ring, (uint16_t)((dsi.rx_ring.buf_size - dsi.huart->hdmarx->Instance->CNDTR)&0xFFFF));

////    dsi.rx_ring.w_ptr = (uint16_t)(dsi.rx_ring.buf_size - current_ndtr);

    // 2. Check if there's new data to read
    if (dsi.rx_ring.r_ptr == dsi.rx_ring.w_ptr) {
        return -1; // Buffer is empty
    }

    // 3. F7 Cache Maintenance (Mandatory for STM32F767/753)
    // Ensure the CPU reads from RAM, not the stale Cache
////    SCB_InvalidateDCache_by_Addr((uint32_t *)&dsi.rx_ring.buf[dsi.rx_ring.r_ptr], 1);

    // 4. Extract the character
    uint8_t c = dsi.rx_ring.buf[dsi.rx_ring.r_ptr];

    // 5. Update the Read Pointer with wrap-around
    dsi.rx_ring.r_ptr = (uint16_t)((dsi.rx_ring.r_ptr + 1) % dsi.rx_ring.buf_size);

    return (int)c;
}

/**
 * @brief Non-blocking character fetch from DMA Ring Buffer
 * @return Character (0-255) if available, -1 if buffer is empty
 */
int dma_getc_blocking(void) {
    // 1. Sync the hardware state to our software w_ptr
    // NDTR is a down-counter (Size -> 0)
//    uint16_t current_ndtr = (uint16_t)__HAL_DMA_GET_COUNTER(dsi.huart->hdmarx);
    dma_ring_set_w_ptr(&dsi.rx_ring, (uint16_t)((dsi.rx_ring.buf_size - dsi.huart->hdmarx->Instance->CNDTR)&0xFFFF));

////    dsi.rx_ring.w_ptr = (uint16_t)(dsi.rx_ring.buf_size - current_ndtr);

    // 2. Check if there's new data to read
    while (dsi.rx_ring.r_ptr == dsi.rx_ring.w_ptr) {
        dma_ring_set_w_ptr(&dsi.rx_ring, (uint16_t)((dsi.rx_ring.buf_size - dsi.huart->hdmarx->Instance->CNDTR)&0xFFFF));
        // Buffer is empty, wait
    }

    // 3. F7 Cache Maintenance (Mandatory for STM32F767/753)
    // Ensure the CPU reads from RAM, not the stale Cache
////    SCB_InvalidateDCache_by_Addr((uint32_t *)&dsi.rx_ring.buf[dsi.rx_ring.r_ptr], 1);

    // 4. Extract the character
    uint8_t c = dsi.rx_ring.buf[dsi.rx_ring.r_ptr];

    // 5. Update the Read Pointer with wrap-around
    dsi.rx_ring.r_ptr = (uint16_t)((dsi.rx_ring.r_ptr + 1) % dsi.rx_ring.buf_size);

    return (int)c;
}

int get_char_with_timeout(uint32_t timeout_ms) {
	int c;
	if (timeout_ms == 0) {
		c = dma_getc_blocking() ;
		return c;
	}

    uint32_t start_tick = HAL_GetTick();

    while ((HAL_GetTick() - start_tick) < timeout_ms) {
        c = dma_getc_nonblocking(); // Your non-blocking function

        if (c != -1) {
            return c; // Key pressed! Return it immediately
        }

        // Optional: Feed a watchdog here if you have one enabled
    }

    return -2; // Custom code indicating a timeout occurred
}
