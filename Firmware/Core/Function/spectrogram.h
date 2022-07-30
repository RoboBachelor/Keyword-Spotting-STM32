#ifndef SPECTROGRAM_H
#define SPECTROGRAM_H

#include <stdio.h>
#include "main.h"
#include "feature_extraction.h"
#include "audio_io.h"


// #define SAMPLE_RATE  16000U /* Input signal sampling rate */

#define FFT_LEN       1024U /* Number of FFT points. Must be greater or equal to FRAME_LEN */
#define FRAME_LEN   FFT_LEN /* Window length and then padded with zeros to match FFT_LEN */
// #define HOP_LEN        512U /* Number of overlapping samples between successive frames */
#define HOP_LEN INPUT_CB_FRAME_CNT

#define NUM_MELS        40U /* Number of mel bands */
#define NUM_MEL_COEFS  981U /* Number of mel filter weights. Returned by MelFilterbank_Init */

#define SPEC_T 30
#define SPEC_F NUM_MELS

extern float specQueue[SPEC_F][SPEC_T];
extern uint8_t specQueueIndex;
extern osMutexId specQueueMutexHandle;

void Spectrogram_Init(void);
void Get_Spectrogram(float retSpec[][SPEC_T]);

#endif
