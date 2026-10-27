param(
    [string]$Port = "COM5"
)

Write-Host "====================================================================" -ForegroundColor Cyan
Write-Host "           PhantomBLE - ESP32 DevKit 38-Pin Flasher" -ForegroundColor Yellow
Write-Host "====================================================================" -ForegroundColor Cyan
Write-Host "Target Serial Port: $Port" -ForegroundColor Green
Write-Host "To specify another port: .\flash.ps1 -Port COMx`n"

Write-Host "NOTE: If connection fails or gives 'Wrong boot mode', HOLD down the BOOT button" -ForegroundColor Yellow
Write-Host "      on your ESP32 board, tap EN / RST, and then run this script again.`n" -ForegroundColor Yellow

$bootloader = "firmware/esp32_devkit_38pin/bootloader.bin"
$partitions = "firmware/esp32_devkit_38pin/partitions.bin"
$firmware = "firmware/esp32_devkit_38pin/firmware.bin"

if (-not (Test-Path $firmware)) {
    Write-Host "[ERROR] Firmware binary not found at $firmware" -ForegroundColor Red
    exit 1
}

Write-Host "Starting flash procedure via esptool..." -ForegroundColor Cyan
python -m esptool --chip esp32 -p $Port -b 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 $bootloader 0x8000 $partitions 0x10000 $firmware

if ($LASTEXITCODE -eq 0) {
    Write-Host "`n====================================================================" -ForegroundColor Green
    Write-Host "[SUCCESS] PhantomBLE has been flashed successfully to your ESP32!" -ForegroundColor Green
    Write-Host "====================================================================" -ForegroundColor Green
    Write-Host "1. Press the EN / RST button on your ESP32 to restart."
    Write-Host "2. Connect your phone or PC to Wi-Fi SoftAP: PhantomBLE-XXXX"
    Write-Host "3. Navigate to: http://192.168.4.1 in your browser."
    Write-Host "4. Or open Serial Monitor at 115200 baud to use the CLI menu."
} else {
    Write-Host "`n[ERROR] Flashing failed with exit code $LASTEXITCODE." -ForegroundColor Red
}
