#include "mbed.h"
#include <cstdint>

#define HR2046_Z_THRESHOLD 400

#ifndef HR2046_H
#define HR2046_H
class HR2046
{
  public:
    HR2046(PinName mosi, PinName miso, PinName clk, PinName cs);
    void initialize(void);
    void setRotation(uint8_t rot);
    void readData(uint16_t* x, uint16_t* y, uint16_t* z);
    bool wasTouched(void);

  private:
    SPI spi;
    DigitalOut chipSelect;
    uint8_t rotation = 1;
    uint16_t xraw = 0;
    uint16_t yraw = 0;
    uint16_t zraw = 0;

    void update(void);
    uint16_t transfer16(uint8_t cmd);
    int16_t bestTwoAvg(int16_t x, int16_t y, int16_t z);
};
#endif