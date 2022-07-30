//
// Created by Jingyi on 2022/4/18.
//

#ifndef C_TEST_ARM_STAT_FCN_H
#define C_TEST_ARM_STAT_FCN_H

#include <stdint.h>

/**
 * @brief 32-bit floating-point type definition.
 */
typedef float float32_t;

/**
 * @brief 64-bit floating-point type definition.
 */
typedef double float64_t;

void arm_mean_f32(
        float32_t * pSrc,
        uint32_t blockSize,
        float32_t * pResult);

void arm_std_f32(
        float32_t * pSrc,
        uint32_t blockSize,
        float32_t * pResult);


#endif //C_TEST_ARM_STAT_FCN_H
