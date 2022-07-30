
#include <math.h>
#include <string.h>
#include "main.h"
#include "sai.h"
#include "audio_io.h"
#include "arm_math.h"

sample_stereo_t sai_rx_buf[SAI_RX_BUF_FRAME];
uint8_t sai_tx_buf[SAI_TX_BUF_SIZE];
uint8_t sai_tx_buf_cur = 0;

uint8_t I2S_Callback_Flag = 0;
uint8_t I2S_Half_Callback_Flag = 0;


sample_stereo_t playerBuf[PLAYER_BUF_LEN]__attribute__((section(".ARM.__at_0xC00062FC")));
//sample_stereo_t playerBuf[PLAYER_BUF_LEN]__attribute__((section(".sdram")));
int32_t playerBufIndex = 0;

__IO uint8_t locked = 0;

void AudioIO_Init(){
	
	HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t*)sai_tx_buf, SAI_TX_BUF_SIZE >> 1);
	HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t*)sai_rx_buf, SAI_RX_BUF_FRAME * NUM_AUDIO_CH);
	
}

void AudioIO_Write_Tone(float freq, float during, float amplitude) {
	while(locked);
	int duringSample = during * SAMPLE_RATE;
	int curIndex = playerBufIndex;
	if (duringSample <= 1000) return;
	for (int i = 0; i < duringSample; ++i) {
		int value = playerBuf[curIndex].L + amplitude * arm_sin_f32(2 * PI * freq * i / SAMPLE_RATE) * expf(-7.0f * i / duringSample);
		if (value >= 32768){
			value = 32767;
		}
		if (value < -32768) value = -32768;
		playerBuf[curIndex].L = playerBuf[curIndex].R = (int16_t)value;
		if (++curIndex >= PLAYER_BUF_LEN) {
			curIndex = 0;
		}
	}
}



__weak void AudioIO_Input_Callback(void* pInput, uint32_t frameCount) {
	
	/* We have not woken a task at the start of the ISR. */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	audio_input_message_t msg = {pInput, frameCount};

	if (audioInputMessageHandle) {
		xQueueSendFromISR( audioInputMessageHandle, &msg, &xHigherPriorityTaskWoken );
	}

	/* Now the buffer is empty we can switch context if necessary. */
	/* Actual macro used here is port specific. */
	portYIELD_FROM_ISR (xHigherPriorityTaskWoken);
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
  
	I2S_Half_Callback_Flag = 1;
	AudioIO_Input_Callback(sai_rx_buf, INPUT_CB_FRAME_CNT);
}

volatile uint32_t sai_tx_ir_cnt = 0;

void HAL_SAI_RxCpltCallback (SAI_HandleTypeDef *hsai) {
  
	++sai_tx_ir_cnt;
	
	I2S_Callback_Flag = 1;
	AudioIO_Input_Callback(sai_rx_buf + INPUT_CB_FRAME_CNT, INPUT_CB_FRAME_CNT);
	
	// HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t*)sai_rx_buf, SAI_RX_BUF_SIZE>>1);
	// printf("HAL_SAI_Receive_DMA: %d\r\n", res);
}



__weak void AudioIO_Output_Callback(void* pOutput, uint32_t frameCount) {
	locked = 1;
	int size1 = PLAYER_BUF_LEN - playerBufIndex;
	if (size1 >= frameCount) {
		memcpy(pOutput, &playerBuf[playerBufIndex], frameCount * BLOCK_ALIGN);
		memset(&playerBuf[playerBufIndex], 0, frameCount * BLOCK_ALIGN);
		playerBufIndex += frameCount;
		if (playerBufIndex >= PLAYER_BUF_LEN ) {
			playerBufIndex = 0;
		}
	}
	else {
		memcpy(pOutput, &playerBuf[playerBufIndex], size1 * BLOCK_ALIGN);
		memset(&playerBuf[playerBufIndex], 0, size1 * BLOCK_ALIGN);
		int size2 = frameCount - size1;
		memcpy((sample_stereo_t*)pOutput + size1, playerBuf, size2 * BLOCK_ALIGN);
		memset(playerBuf, 0, size2 * BLOCK_ALIGN);
		playerBufIndex = size2;
	}
	locked = 0;
}


void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai){
	
	AudioIO_Output_Callback((void*)sai_tx_buf, OUTPUT_CB_FRAME_CNT);
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai){
	
	AudioIO_Output_Callback((void*)(sai_tx_buf + OUTPUT_CB_FRAME_CNT * BLOCK_ALIGN), OUTPUT_CB_FRAME_CNT);
	
	//HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t*)sai_tx_buf[sai_tx_buf_cur], 480 * blockAlign >> 1);
	//HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t*)sai_tx_buf, SAI_RX_BUF_SIZE>>1);
}

