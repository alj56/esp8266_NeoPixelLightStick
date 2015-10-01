/*****************************************************
 
 *****************************************************/

#ifndef RAW_FLASH_DRIVER_H
#define RAW_FLASH_DRIVER_H

#include <stddef.h>
#include <stdint.h>


class RawFlashDriverClass {
  public:
    uint32_t _startAddress;
    uint32_t _flashSize;
    uint32_t _flashSecSize;

  public:
    RawFlashDriverClass(uint32_t startAddress, uint32_t flashSize, uint32_t flashSecSize);

    bool read(uint32_t address, uint8_t data[], uint32_t size);
    bool write(uint32_t address, uint8_t data[], uint32_t size);
    bool eraseSector(uint32_t address);
    bool eraseAll();

    void showFlashInfo();
    void showData(uint8_t data[], uint32_t size);
};

extern RawFlashDriverClass RawFlashDriver;

#endif // RAW_FLASH_DRIVER_H
