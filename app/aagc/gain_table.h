#include <stdint.h>

#define EXPECT_REG_VAL 9000.0f         // adc input power -10dBm
#define REG_ADDRESS    0x43c20124   // bit14 - bit0

#define HIGH_THRESHOLD 3
#define LOW_THRESHOLD  1.5f

#define T_SIZE 31

// control 12 bit  --- high 8 bit 
uint16_t t_reg_0x17[T_SIZE] = { 0x0 };

// control 12 bit --- low 4 bit in reg high 4 bit
uint16_t t_reg_0x16[T_SIZE] = { 0x0 };

float t_if_gain[T_SIZE] = { 0.0f };

float t_attenuation[T_SIZE] = { 0.0f };
