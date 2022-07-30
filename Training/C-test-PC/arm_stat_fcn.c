
#include <math.h>
#include "arm_stat_fcn.h"

/**
 * @brief Mean value of a floating-point vector.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       blockSize length of the input vector
 * @param[out]      *pResult mean value returned here
 * @return none.
 */

void arm_mean_f32(
        float32_t * pSrc,
        uint32_t blockSize,
        float32_t * pResult)
{
    float32_t sum = 0.0f;                          /* Temporary result storage */
    uint32_t blkCnt;                               /* loop counter */

#if defined (ARM_MATH_DSP)
    /* Run the below code for Cortex-M4 and Cortex-M3 */

  float32_t in1, in2, in3, in4;

  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C = (A[0] + A[1] + A[2] + ... + A[blockSize-1]) */
    in1 = *pSrc++;
    in2 = *pSrc++;
    in3 = *pSrc++;
    in4 = *pSrc++;

    sum += in1;
    sum += in2;
    sum += in3;
    sum += in4;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;

#else
    /* Run the below code for Cortex-M0 */

    /* Loop over blockSize number of values */
    blkCnt = blockSize;

#endif /* #if defined (ARM_MATH_DSP) */

    while (blkCnt > 0U)
    {
        /* C = (A[0] + A[1] + A[2] + ... + A[blockSize-1]) */
        sum += *pSrc++;

        /* Decrement the loop counter */
        blkCnt--;
    }

    /* C = (A[0] + A[1] + A[2] + ... + A[blockSize-1]) / blockSize  */
    /* Store the result to the destination */
    *pResult = sum / (float32_t) blockSize;
}

/**
 * @brief Standard deviation of the elements of a floating-point vector.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       blockSize length of the input vector
 * @param[out]      *pResult standard deviation value returned here
 * @return none.
 */

void arm_std_f32(
        float32_t * pSrc,
        uint32_t blockSize,
        float32_t * pResult)
{
    float32_t sum = 0.0f;                          /* Temporary result storage */
    float32_t sumOfSquares = 0.0f;                 /* Sum of squares */
    float32_t in;                                  /* input value */
    uint32_t blkCnt;                               /* loop counter */
#if defined (ARM_MATH_DSP)
    float32_t meanOfSquares, mean, squareOfMean;   /* Temporary variables */
#else
    float32_t squareOfSum;                         /* Square of Sum */
    float32_t var;                                 /* Temporary varaince storage */
#endif

    if (blockSize == 1U)
    {
        *pResult = 0;
        return;
    }

#if defined (ARM_MATH_DSP)
    /* Run the below code for Cortex-M4 and Cortex-M3 */

  /*loop Unrolling */
  blkCnt = blockSize >> 2U;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
   ** a second loop below computes the remaining 1 to 3 samples. */
  while (blkCnt > 0U)
  {
    /* C = (A[0] * A[0] + A[1] * A[1] + ... + A[blockSize-1] * A[blockSize-1])  */
    /* Compute Sum of squares of the input samples
     * and then store the result in a temporary variable, sum. */
    in = *pSrc++;
    sum += in;
    sumOfSquares += in * in;
    in = *pSrc++;
    sum += in;
    sumOfSquares += in * in;
    in = *pSrc++;
    sum += in;
    sumOfSquares += in * in;
    in = *pSrc++;
    sum += in;
    sumOfSquares += in * in;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4U;

  while (blkCnt > 0U)
  {
    /* C = (A[0] * A[0] + A[1] * A[1] + ... + A[blockSize-1] * A[blockSize-1]) */
    /* Compute Sum of squares of the input samples
     * and then store the result in a temporary variable, sum. */
    in = *pSrc++;
    sum += in;
    sumOfSquares += in * in;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Compute Mean of squares of the input samples
   * and then store the result in a temporary variable, meanOfSquares. */
  meanOfSquares = sumOfSquares / ((float32_t) blockSize - 1.0f);

  /* Compute mean of all input values */
  mean = sum / (float32_t) blockSize;

  /* Compute square of mean */
  squareOfMean = (mean * mean) * (((float32_t) blockSize) /
                                  ((float32_t) blockSize - 1.0f));

  /* Compute standard deviation and then store the result to the destination */
  arm_sqrt_f32((meanOfSquares - squareOfMean), pResult);

#else
    /* Run the below code for Cortex-M0 */

    /* Loop over blockSize number of values */
    blkCnt = blockSize;

    while (blkCnt > 0U)
    {
        /* C = (A[0] * A[0] + A[1] * A[1] + ... + A[blockSize-1] * A[blockSize-1]) */
        /* Compute Sum of squares of the input samples
         * and then store the result in a temporary variable, sumOfSquares. */
        in = *pSrc++;
        sumOfSquares += in * in;

        /* C = (A[0] + A[1] + ... + A[blockSize-1]) */
        /* Compute Sum of the input samples
         * and then store the result in a temporary variable, sum. */
        sum += in;

        /* Decrement the loop counter */
        blkCnt--;
    }

    /* Compute the square of sum */
    squareOfSum = ((sum * sum) / (float32_t) blockSize);

    /* Compute the variance */
    var = ((sumOfSquares - squareOfSum) / (float32_t) (blockSize - 1.0f));

    /* Compute standard deviation and then store the result to the destination */
    // arm_sqrt_f32(var, pResult);
    *pResult = sqrtf(var);

#endif /* #if defined (ARM_MATH_DSP) */
}
