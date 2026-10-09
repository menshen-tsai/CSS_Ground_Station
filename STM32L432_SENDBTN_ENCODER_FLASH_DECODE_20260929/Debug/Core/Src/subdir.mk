################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/AFSK.c \
../Core/Src/APRS.c \
../Core/Src/BitFIFO.c \
../Core/Src/IntFIFO.c \
../Core/Src/dma_printf.c \
../Core/Src/dma_ring.c \
../Core/Src/dma_scanf.c \
../Core/Src/dtmf.c \
../Core/Src/global.c \
../Core/Src/log.c \
../Core/Src/main.c \
../Core/Src/menu.c \
../Core/Src/sr105u.c \
../Core/Src/stm32l4xx_hal_msp.c \
../Core/Src/stm32l4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32l4xx.c 

OBJS += \
./Core/Src/AFSK.o \
./Core/Src/APRS.o \
./Core/Src/BitFIFO.o \
./Core/Src/IntFIFO.o \
./Core/Src/dma_printf.o \
./Core/Src/dma_ring.o \
./Core/Src/dma_scanf.o \
./Core/Src/dtmf.o \
./Core/Src/global.o \
./Core/Src/log.o \
./Core/Src/main.o \
./Core/Src/menu.o \
./Core/Src/sr105u.o \
./Core/Src/stm32l4xx_hal_msp.o \
./Core/Src/stm32l4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32l4xx.o 

C_DEPS += \
./Core/Src/AFSK.d \
./Core/Src/APRS.d \
./Core/Src/BitFIFO.d \
./Core/Src/IntFIFO.d \
./Core/Src/dma_printf.d \
./Core/Src/dma_ring.d \
./Core/Src/dma_scanf.d \
./Core/Src/dtmf.d \
./Core/Src/global.d \
./Core/Src/log.d \
./Core/Src/main.d \
./Core/Src/menu.d \
./Core/Src/sr105u.d \
./Core/Src/stm32l4xx_hal_msp.d \
./Core/Src/stm32l4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32l4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L432xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/AFSK.cyclo ./Core/Src/AFSK.d ./Core/Src/AFSK.o ./Core/Src/AFSK.su ./Core/Src/APRS.cyclo ./Core/Src/APRS.d ./Core/Src/APRS.o ./Core/Src/APRS.su ./Core/Src/BitFIFO.cyclo ./Core/Src/BitFIFO.d ./Core/Src/BitFIFO.o ./Core/Src/BitFIFO.su ./Core/Src/IntFIFO.cyclo ./Core/Src/IntFIFO.d ./Core/Src/IntFIFO.o ./Core/Src/IntFIFO.su ./Core/Src/dma_printf.cyclo ./Core/Src/dma_printf.d ./Core/Src/dma_printf.o ./Core/Src/dma_printf.su ./Core/Src/dma_ring.cyclo ./Core/Src/dma_ring.d ./Core/Src/dma_ring.o ./Core/Src/dma_ring.su ./Core/Src/dma_scanf.cyclo ./Core/Src/dma_scanf.d ./Core/Src/dma_scanf.o ./Core/Src/dma_scanf.su ./Core/Src/dtmf.cyclo ./Core/Src/dtmf.d ./Core/Src/dtmf.o ./Core/Src/dtmf.su ./Core/Src/global.cyclo ./Core/Src/global.d ./Core/Src/global.o ./Core/Src/global.su ./Core/Src/log.cyclo ./Core/Src/log.d ./Core/Src/log.o ./Core/Src/log.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/menu.cyclo ./Core/Src/menu.d ./Core/Src/menu.o ./Core/Src/menu.su ./Core/Src/sr105u.cyclo ./Core/Src/sr105u.d ./Core/Src/sr105u.o ./Core/Src/sr105u.su ./Core/Src/stm32l4xx_hal_msp.cyclo ./Core/Src/stm32l4xx_hal_msp.d ./Core/Src/stm32l4xx_hal_msp.o ./Core/Src/stm32l4xx_hal_msp.su ./Core/Src/stm32l4xx_it.cyclo ./Core/Src/stm32l4xx_it.d ./Core/Src/stm32l4xx_it.o ./Core/Src/stm32l4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32l4xx.cyclo ./Core/Src/system_stm32l4xx.d ./Core/Src/system_stm32l4xx.o ./Core/Src/system_stm32l4xx.su

.PHONY: clean-Core-2f-Src

