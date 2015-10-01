/*****************************************************
 
 *****************************************************/
#include <ESP.h>
#include "RawFlashDriver.h"

extern "C" {
#include "c_types.h"
#include "spi_flash.h"
}

extern "C" uint32_t _SPIFFS_start;
extern "C" uint32_t _SPIFFS_end;


RawFlashDriverClass::RawFlashDriverClass(uint32_t startAddress, uint32_t flashSize, uint32_t flashSecSize) 
: _startAddress(startAddress)
, _flashSize(flashSize)
, _flashSecSize(flashSecSize)
{  
}


bool RawFlashDriverClass::read(uint32_t address, uint8_t data[], uint32_t size) {
  noInterrupts();
  bool result = (spi_flash_read(address, reinterpret_cast<uint32_t*>(data), size) == SPI_FLASH_RESULT_OK);
  interrupts();
  return result;
}


bool RawFlashDriverClass::write(uint32_t address, uint8_t data[], uint32_t size) {
  noInterrupts();
  bool result = (spi_flash_write(address, reinterpret_cast<uint32_t*>(data), size) == SPI_FLASH_RESULT_OK);
  interrupts();
  return result;  
}


bool RawFlashDriverClass::eraseSector(uint32_t address) {
  noInterrupts();
  bool result = (spi_flash_erase_sector(address/SPI_FLASH_SEC_SIZE) == SPI_FLASH_RESULT_OK);
  interrupts();
  return result; 
}


bool RawFlashDriverClass::eraseAll() {
  bool result = true;
  for (uint32_t i = _startAddress; i < _startAddress + _flashSize && result; i = i + _flashSecSize) {
    result = eraseSector(i);
  }
  return result;
}


void RawFlashDriverClass::showFlashInfo() {
  Serial.println(F("flash info:"));
  Serial.print(F("  start address: 0x")); Serial.println(_startAddress, HEX);
  Serial.print(F("  end address: 0x")); Serial.println((uint32_t)&_SPIFFS_end - 0x40200000, HEX);
  Serial.print(F("  flash size: 0x")); Serial.println(_flashSize, HEX);
  Serial.print(F("  flash sector size: 0x")); Serial.println(_flashSecSize, HEX);
}


void RawFlashDriverClass::showData(uint8_t data[], uint32_t size) {
  for (int i = 0; i < size; i++) {
    if (i % 16 == 0) {
      Serial.print(F("R "));
      Serial.print(i);
      Serial.print(F(": "));
    }
    Serial.print(F("0x"));
    Serial.print(data[i], HEX);
    Serial.print(F(" "));
    Serial.print((char)data[i]);
    if (i % 16 == 15) {
      Serial.println();
    } else {
      Serial.print(F(" - "));
    }
  }  
  Serial.println();
}


RawFlashDriverClass RawFlashDriver((uint32_t)&_SPIFFS_start - 0x40200000, (uint32_t)&_SPIFFS_end - (uint32_t)&_SPIFFS_start, SPI_FLASH_SEC_SIZE);
