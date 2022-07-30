#ifndef AUDIO_IO_H
#define AUDIO_IO_H

#include "main.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "cmsis_os.h"
#include "portmacro.h"

typedef struct {
    int16_t L;
    int16_t R;
} sample_stereo_t;

typedef struct {
	void* pInput;
	uint32_t frameCnt;
} audio_input_message_t;


extern osMessageQId audioInputMessageHandle;

#define BLOCK_ALIGN sizeof(sample_stereo_t)
#define NUM_AUDIO_CH 2

#define OUTPUT_CB_FRAME_CNT 480
#define SAI_TX_BUF_SIZE (OUTPUT_CB_FRAME_CNT * BLOCK_ALIGN * 2)

#define INPUT_CB_FRAME_CNT 512
#define SAI_RX_BUF_FRAME (INPUT_CB_FRAME_CNT * 2)

#define SAMPLE_RATE 16000
#define PLAYER_BUF_LEN (SAMPLE_RATE * 2) // 2 for stereo

#ifndef PI
	#define PI 3.14159265358979323846f
#endif

void AudioIO_Init(void);
void AudioIO_Write_Tone(float freq, float during, float amplitude);

#endif
