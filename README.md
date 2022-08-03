# Keyword Spotting on STM32 Microcontroller

This is our experimental code for *Keyword Spotting System and Evaluation of Pruning and Quantization Methods on Low-Power Edge Microcontrollers* submitted 
to [DCASE 2022 Workshop](https://dcase.community/workshop2022/index). You may refer to [paper manuscript](https://site.mr-digital.cn/file/DCASE2022_ID49_V1_1.pdf) for more information.

## Firmware

#### Development Board: [Alientek Apollo STM32F767 (正点原子阿波罗F767开发板)](http://www.openedv.com/docs/boards/stm32/zdyz_stm32f767_apollo.html)
#### Environment:   Keil MDK v5.37, ARM Compiler v6.18, HAL Driver
#### Usage:   
- Press and hold **KEY2** to receive live audio and run audio feature generation, otherwise not run audio feature generation and a fixed sample speatrogram is used for inference.  
- Press and hold **KEY0** to skip inference (used to measure the consumed power when CPU is relative free).

## Training

#### Environment:   Pytorch 10.1
#### Dataset:  
[Speech Command Dataset v0.01](https://ai.googleblog.com/2017/08/launching-speech-commands-dataset.html). You don't have to download the full dataset since the preprocessed audio spectrograms are included in this repository. Put the dataset to `data/` and remove `prep_train/` and `prep_test/` folders to re-generate audio spectrograms when you modify the pre-processing functions.

## Contact
Any problems and suggestions are welcome. Please mail to Jingyi.Wang1903@student.xjtlu.edu.cn

## Acknowledgment
Thanks to Chengsen Dong (Master student in University of Glasgow since 2022). We discuss and get inspiration from each other. His github: [xddcode](https://github.com/xddcore).
