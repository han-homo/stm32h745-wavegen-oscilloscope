################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../dsp1/arm_bitreversal2.c \
../dsp1/arm_cfft_f32.c \
../dsp1/arm_cfft_init_f32.c \
../dsp1/arm_cfft_radix8_f32.c \
../dsp1/arm_cmplx_mag_f32.c \
../dsp1/arm_common_tables.c \
../dsp1/arm_const_structs.c \
../dsp1/arm_cos_f32.c \
../dsp1/arm_max_f32.c \
../dsp1/arm_mean_f32.c \
../dsp1/arm_mult_f32.c \
../dsp1/arm_offset_f32.c \
../dsp1/arm_rfft_fast_f32.c \
../dsp1/arm_rfft_fast_init_f32.c 

OBJS += \
./dsp1/arm_bitreversal2.o \
./dsp1/arm_cfft_f32.o \
./dsp1/arm_cfft_init_f32.o \
./dsp1/arm_cfft_radix8_f32.o \
./dsp1/arm_cmplx_mag_f32.o \
./dsp1/arm_common_tables.o \
./dsp1/arm_const_structs.o \
./dsp1/arm_cos_f32.o \
./dsp1/arm_max_f32.o \
./dsp1/arm_mean_f32.o \
./dsp1/arm_mult_f32.o \
./dsp1/arm_offset_f32.o \
./dsp1/arm_rfft_fast_f32.o \
./dsp1/arm_rfft_fast_init_f32.o 

C_DEPS += \
./dsp1/arm_bitreversal2.d \
./dsp1/arm_cfft_f32.d \
./dsp1/arm_cfft_init_f32.d \
./dsp1/arm_cfft_radix8_f32.d \
./dsp1/arm_cmplx_mag_f32.d \
./dsp1/arm_common_tables.d \
./dsp1/arm_const_structs.d \
./dsp1/arm_cos_f32.d \
./dsp1/arm_max_f32.d \
./dsp1/arm_mean_f32.d \
./dsp1/arm_mult_f32.d \
./dsp1/arm_offset_f32.d \
./dsp1/arm_rfft_fast_f32.d \
./dsp1/arm_rfft_fast_init_f32.d 


# Each subdirectory must supply rules for building sources it contributes
dsp1/%.o dsp1/%.su dsp1/%.cyclo: ../dsp1/%.c dsp1/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DARM_MATH_CM7 -DCORE_CM7 -DUSE_HAL_DRIVER -DSTM32H745xx -DUSE_PWR_DIRECT_SMPS_SUPPLY -c -I../Core/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../Drivers/CMSIS/Include -I"C:/STM32/CMSIS-DSP-main/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-dsp1

clean-dsp1:
	-$(RM) ./dsp1/arm_bitreversal2.cyclo ./dsp1/arm_bitreversal2.d ./dsp1/arm_bitreversal2.o ./dsp1/arm_bitreversal2.su ./dsp1/arm_cfft_f32.cyclo ./dsp1/arm_cfft_f32.d ./dsp1/arm_cfft_f32.o ./dsp1/arm_cfft_f32.su ./dsp1/arm_cfft_init_f32.cyclo ./dsp1/arm_cfft_init_f32.d ./dsp1/arm_cfft_init_f32.o ./dsp1/arm_cfft_init_f32.su ./dsp1/arm_cfft_radix8_f32.cyclo ./dsp1/arm_cfft_radix8_f32.d ./dsp1/arm_cfft_radix8_f32.o ./dsp1/arm_cfft_radix8_f32.su ./dsp1/arm_cmplx_mag_f32.cyclo ./dsp1/arm_cmplx_mag_f32.d ./dsp1/arm_cmplx_mag_f32.o ./dsp1/arm_cmplx_mag_f32.su ./dsp1/arm_common_tables.cyclo ./dsp1/arm_common_tables.d ./dsp1/arm_common_tables.o ./dsp1/arm_common_tables.su ./dsp1/arm_const_structs.cyclo ./dsp1/arm_const_structs.d ./dsp1/arm_const_structs.o ./dsp1/arm_const_structs.su ./dsp1/arm_cos_f32.cyclo ./dsp1/arm_cos_f32.d ./dsp1/arm_cos_f32.o ./dsp1/arm_cos_f32.su ./dsp1/arm_max_f32.cyclo ./dsp1/arm_max_f32.d ./dsp1/arm_max_f32.o ./dsp1/arm_max_f32.su ./dsp1/arm_mean_f32.cyclo ./dsp1/arm_mean_f32.d ./dsp1/arm_mean_f32.o ./dsp1/arm_mean_f32.su ./dsp1/arm_mult_f32.cyclo ./dsp1/arm_mult_f32.d ./dsp1/arm_mult_f32.o ./dsp1/arm_mult_f32.su ./dsp1/arm_offset_f32.cyclo ./dsp1/arm_offset_f32.d ./dsp1/arm_offset_f32.o ./dsp1/arm_offset_f32.su ./dsp1/arm_rfft_fast_f32.cyclo ./dsp1/arm_rfft_fast_f32.d ./dsp1/arm_rfft_fast_f32.o ./dsp1/arm_rfft_fast_f32.su ./dsp1/arm_rfft_fast_init_f32.cyclo ./dsp1/arm_rfft_fast_init_f32.d ./dsp1/arm_rfft_fast_init_f32.o ./dsp1/arm_rfft_fast_init_f32.su

.PHONY: clean-dsp1

