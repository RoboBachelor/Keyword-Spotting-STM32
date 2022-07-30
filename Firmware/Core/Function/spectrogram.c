/**
 ******************************************************************************
 * @file    melspectrogram_example.c
 * @author  MCD Application Team
 * @brief   Melspectrogram computation example
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Software License Agreement
 * SLA0055, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *        www.st.com/resource/en/license_agreement/dm00251784.pdf
 *
 ******************************************************************************
 */
#include "feature_extraction.h"
#include "spectrogram.h"
#include <stdio.h>
#include "usart.h"
#include "tim.h"

/*
 * y = librosa.load('bus.wav', sr=None, duration=1)[0] # Keep native 16kHz sampling rate
 * librosa.feature.melspectrogram(y, sr=16000, n_mels=30, n_fft=1024, hop_length=512, center=False)
 */


arm_rfft_fast_instance_f32 S_Rfft;
MelFilterTypeDef           S_MelFilter;
SpectrogramTypeDef         S_Spectr;
MelSpectrogramTypeDef      S_MelSpectr;
LogMelSpectrogramTypeDef   S_LogMelSpectr;

float32_t pInFrame[FRAME_LEN];
float32_t pOutColBuffer[NUM_MELS];
float32_t pWindowFuncBuffer[FRAME_LEN]__attribute__((section(".ARM.__at_0xC00043A8")));
float32_t pSpectrScratchBuffer[FFT_LEN];
float32_t pMelFilterCoefs[NUM_MEL_COEFS]__attribute__((section(".ARM.__at_0xC00053A8")));
uint32_t pMelFilterStartIndices[NUM_MELS];
uint32_t pMelFilterStopIndices[NUM_MELS];

float normedQueueBuffer[FRAME_LEN - HOP_LEN];

float specQueue[SPEC_F][SPEC_T]__attribute__((section(".ARM.__at_0xC00012C0")));
uint8_t specQueueIndex = 0;

	
void Get_Spectrogram(float retSpec[][SPEC_T]){
	
	xSemaphoreTake(specQueueMutexHandle, portMAX_DELAY);
	
	uint16_t queueIndex = specQueueIndex;
	for (uint16_t t = 0; t < SPEC_T; ++t) {
		
		for (uint16_t f = 0; f < SPEC_F; ++f) {
			retSpec[f][t] = specQueue[f][queueIndex]; 
		}
		if (++queueIndex >= SPEC_T) {
			queueIndex = 0;
		}
	}
	xSemaphoreGive(specQueueMutexHandle);
}	


//void AudioIO_Input_Callback(void* pInput, uint32_t frameCount) {
void Spectrogram_Task(void const * argument) {
	
	audio_input_message_t msg;
	
	while ( xQueueReceive(audioInputMessageHandle, &msg, portMAX_DELAY) == pdTRUE ) {
		
		// printf("CNT=%d | Spec_Task: pInput = 0x%p, frameCnt = %d, queue wait: %d\r\n", spec_task_cnt++, msg.pInput, msg.frameCnt, osMessageWaiting(audioInputMessageHandle));
		if (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin)) {
			continue;
		}
		
		
		// Defined HOP_LEN should be equal to defined INPUT_CB_FRAME_CNT in audio_io.h!
		assert_param(msg.frameCnt == HOP_LEN);	
		
		// Int16 to normed float32
		for (uint16_t i = 0; i < HOP_LEN; ++i){
			pInFrame[i + FRAME_LEN - HOP_LEN] = (float) ((sample_stereo_t*)msg.pInput)[i].L / (1 << 15);
		}
		
		memcpy(pInFrame, normedQueueBuffer, (FRAME_LEN - HOP_LEN) * sizeof(float));
		memcpy(normedQueueBuffer, pInFrame + HOP_LEN, (FRAME_LEN - HOP_LEN) * sizeof(float));
		
		// Calculate log-mel-spectrogram.
		LogMelSpectrogramColumn(&S_LogMelSpectr, pInFrame, pOutColBuffer);
		
		xSemaphoreTake(specQueueMutexHandle, portMAX_DELAY);
		for (uint8_t i = 0; i < SPEC_F; ++i) {
			specQueue[i][specQueueIndex] = pOutColBuffer[i];
		}
		
		if (++specQueueIndex >= SPEC_T) {
			specQueueIndex = 0;
		}
		xSemaphoreGive(specQueueMutexHandle);
		
		// Print the latest Mel column to plot in PC.
	//	printf("spec");
	//	for (uint8_t i = 0; i < SPEC_F; ++i){
	//		printf("%.1f|", pOutColBuffer[i]);
	//	}
	//	printf("\r\n");
		
	}
	
}

void Spectrogram_Init(void)
{
  /* Init window function */
  if (Window_Init(pWindowFuncBuffer, FRAME_LEN, WINDOW_HANN) != 0)
  {
    while(1);
  }

  /* Init RFFT */
  arm_rfft_fast_init_f32(&S_Rfft, FFT_LEN);

  /* Init Spectrogram */
  S_Spectr.pRfft    = &S_Rfft;
  S_Spectr.Type     = SPECTRUM_TYPE_POWER;
  S_Spectr.pWindow  = pWindowFuncBuffer;
  S_Spectr.SampRate = SAMPLE_RATE;
  S_Spectr.FrameLen = FRAME_LEN;
  S_Spectr.FFTLen   = FFT_LEN;
  S_Spectr.pScratch = pSpectrScratchBuffer;

  /* Init Mel filter */
  S_MelFilter.pStartIndices = pMelFilterStartIndices;
  S_MelFilter.pStopIndices  = pMelFilterStopIndices;
  S_MelFilter.pCoefficients = pMelFilterCoefs;
  S_MelFilter.NumMels   = NUM_MELS;
  S_MelFilter.FFTLen    = FFT_LEN;
  S_MelFilter.SampRate  = SAMPLE_RATE;
  S_MelFilter.FMin      = 0.0;
  S_MelFilter.FMax      = S_MelFilter.SampRate / 2.0;
  S_MelFilter.Formula   = MEL_SLANEY;
  S_MelFilter.Normalize = 1;
  S_MelFilter.Mel2F     = 1;
  MelFilterbank_Init(&S_MelFilter);

  /* Init MelSpectrogram */
  S_MelSpectr.SpectrogramConf = &S_Spectr;
  S_MelSpectr.MelFilter       = &S_MelFilter;
  
  /* Init LogMelSpectrogram */
  S_LogMelSpectr.MelSpectrogramConf = &S_MelSpectr;
  S_LogMelSpectr.LogFormula = LOGMELSPECTROGRAM_SCALE_DB;
  S_LogMelSpectr.Ref = 1.0f;
  S_LogMelSpectr.TopdB = 80.0f;
}

// Example Code. Not used.
void AudioPreprocessing_Run(int16_t *pInSignal, float32_t *pOut, uint32_t signal_len)
{
  const uint32_t num_frames = 1 + (signal_len - FRAME_LEN) / HOP_LEN;

  for (uint32_t frame_index = 0; frame_index < num_frames; frame_index++)
  {
    buf_to_float_normed(&pInSignal[HOP_LEN * frame_index], pInFrame, FRAME_LEN);
    MelSpectrogramColumn(&S_MelSpectr, pInFrame, pOutColBuffer);
    /* Reshape column into pOut */
    for (uint32_t i = 0; i < NUM_MELS; i++)
    {
      pOut[i * num_frames + frame_index] = pOutColBuffer[i];
    }
  }
}
