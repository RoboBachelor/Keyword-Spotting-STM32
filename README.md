# Keyword Spotting on STM32 Microcontroller

This is our experimental code for *Keyword Spotting System and Evaluation of Pruning and Quantization Methods on Low-Power Edge Microcontrollers* submitted 
to [DCASE 2022 Workshop](https://dcase.community/workshop2022/index). You may refer to [paper manuscript](https://site.mr-digital.cn/file/DCASE2022_ID49_V1_1.pdf) for more information.

## Firmware
Please refer to this repository temporarily: [https://github.com/RoboBachelor/Audio-Classification-STM32F7](https://github.com/RoboBachelor/Audio-Classification-STM32F7)


#### Development Board: [Alientek Apollo STM32F767 (正点原子阿波罗F767开发板)](http://www.openedv.com/docs/boards/stm32/zdyz_stm32f767_apollo.html)
#### Usage:   
- Press and hold **KEY2** to receive live audio and run audio feature generation, otherwise not run audio feature generation and a fixed sample speatrogram is used for inference.  
- Press and hold **KEY0** to skip inference (used to measure the consumed power when CPU is relative free).

## Training
The model is training using Pytorch. The code is to be updated.

## Contact
Any problems and suggestions are welcome. Please mail to Jingyi.Wang1903@student.xjtlu.edu.cn
