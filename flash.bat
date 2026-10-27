@echo off
setlocal enabledelayedexpansion

echo ====================================================================
echo             PhantomBLE - ESP32 DevKit 38-Pin Flasher
echo ====================================================================
echo.

set PORT=%1
if "%PORT%"=="" set PORT=COM5

echo Target Port: %PORT%
echo (If your board is on a different port, run: flash.bat COMx)
echo.
echo NOTE: If connection fails, HOLD the 'BOOT' button on your ESP32,
echo       press and release the 'EN/RST' button, then release 'BOOT'.
echo.
pause

echo.
echo Flashing PhantomBLE firmware to %PORT%...
python -m esptool --chip esp32 -p %PORT% -b 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 firmware/esp32_devkit_38pin/bootloader.bin 0x8000 firmware/esp32_devkit_38pin/partitions.bin 0x10000 firmware/esp32_devkit_38pin/firmware.bin

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ====================================================================
    echo [SUCCESS] PhantomBLE successfully flashed to your ESP32 DevKit!
    echo ====================================================================
    echo Next Steps:
    echo  1. Press the EN / RST button on your ESP32.
    echo  2. Connect to Wi-Fi SoftAP: 'PhantomBLE-XXXX' (no password by default)
    echo  3. Open browser: http://192.168.4.1
    echo  4. Or open Serial Monitor at 115200 baud.
    echo ====================================================================
) else (
    echo.
    echo [ERROR] Flashing failed. Please check COM port and BOOT mode.
)

pause
