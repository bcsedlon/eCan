################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
INO_SRCS += \
..\WiFiManager\examples\AutoConnect\AutoConnect.ino 

INO_DEPS += \
.\WiFiManager\examples\AutoConnect\AutoConnect.ino.d 


# Each subdirectory must supply rules for building sources it contributes
WiFiManager\examples\AutoConnect\AutoConnect.o: ..\WiFiManager\examples\AutoConnect\AutoConnect.ino
	@echo 'Building file: $<'
	@echo 'Starting C++ compile'
	"D:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/xtensa-esp32-elf/bin/xtensa-esp32-elf-g++" -DESP_PLATFORM "-DMBEDTLS_CONFIG_FILE=\"mbedtls/esp_config.h\"" -DHAVE_CONFIG_H "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/config" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/bluedroid" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/app_trace" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/app_update" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/bootloader_support" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/bt" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/driver" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/esp32" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/esp_adc_cal" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/ethernet" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/fatfs" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/freertos" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/heap" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/jsmn" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/log" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/mdns" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/mbedtls" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/mbedtls_port" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/newlib" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/nvs_flash" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/openssl" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/spi_flash" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/sdmmc" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/spiffs" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/tcpip_adapter" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/ulp" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/vfs" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/wear_levelling" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/xtensa-debug-module" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/coap" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/console" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/expat" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/json" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/lwip" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/newlib" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/nghttp" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/soc" "-ID:\App\sloeber\/arduinoPlugin/packages/arduino-esp32/tools/sdk/include/wpa_supplicant" -std=gnu++11 -fno-exceptions -Os -g3 -Wpointer-arith -fexceptions -fstack-protector -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -mlongcalls -nostdlib -Wall -Werror=all -Wextra -Wno-error=unused-function -Wno-error=unused-but-set-variable -Wno-error=unused-variable -Wno-error=deprecated-declarations -Wno-unused-parameter -Wno-sign-compare -fno-rtti -c -DF_CPU=240000000L -DARDUINO=10802 -DARDUINO_LOLIN32 -DARDUINO_ARCH_ARDUINO-ESP32 "-DARDUINO_BOARD=\"LOLIN32\"" "-DARDUINO_VARIANT=\"lolin32\"" -DESP32 -DCORE_DEBUG_LEVEL=0  -I"D:\App\sloeber\arduinoPlugin\packages\arduino-esp32\cores\esp32" -I"D:\App\sloeber\arduinoPlugin\packages\arduino-esp32\variants\lolin32" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -D__IN_ECLIPSE__=1 -x c++ "$<"  -o  "$@"
	@echo 'Finished building: $<'
	@echo ' '


