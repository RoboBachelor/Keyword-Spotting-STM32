cmake_minimum_required(VERSION 3.21)
project(C_test C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(C_test
        cnn_one_fstride4_normed.c
        model.c
        arm_stat_fcn.c
        )
