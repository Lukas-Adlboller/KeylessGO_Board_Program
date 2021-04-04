#include "HR2046.h"
#include <cstdint>

HR2046::HR2046(PinName mosi, PinName miso, PinName clk, PinName cs) : spi(mosi, miso, clk), chipSelect(cs)
{
}

void HR2046::initialize(void)
{
  spi.format(8, 3);
  spi.frequency(10000000);
  chipSelect = 1;
}

bool HR2046::wasTouched(void)
{
  update();
  return zraw >= HR2046_Z_THRESHOLD;
}

void HR2046::readData(uint16_t *x, uint16_t *y, uint16_t *z)
{
  update();
  *z = zraw;

  /*
    Calculation of useable coordinates by using measured borders of one
    specific Touch Display. The measurements are not precise but useable. 

    X and Y values in the following table are the xraw and yraw values recieved
    from the display measured at the corners in each rotation:
    
    Rotation 0:   dY = 3585   dX = 3615
      - Top Left:       X: 95   Y: 190
      - Top Right:      X: 3710 Y: 190
      - Bottom Left:    X: 95   Y: 3775
      - Bottom Right:   X: 3710 Y: 3775

    Rotation 1:   dY = 3615   dX = 3615
      - Top Left:       X: 160  Y: 4000
      - Top Right:      X: 160  Y: 385
      - Bottom Left:    X: 3775 Y: 4000
      - Bottom Right:   X: 3775 Y: 385

    Rotation 2:   dY = 3640   dX = 3615
      - Top Left:       X: 4000  Y: 3945
      - Top Right:      X: 385   Y: 3945
      - Bottom Left:    X: 4000  Y: 305
      - Bottom Right:   X: 385   Y: 305

    Rotation 3:   dY = 3624   dX = 3604
      - Top Left:       X: 3955  Y: 191
      - Top Right:      X: 3955  Y: 3815
      - Bottom Left:    X: 351   Y: 191
      - Bottom Right:   X: 351   Y: 3815
  */
  switch(rotation)
  {
    case 0:
      *x = (xraw - 95) / (3615 / 240);
      *y = (yraw - 190) / (3585 / 320);
      break;
    case 1:
      *x = (xraw - 160) / (3615 / 320);
      *y = (yraw - 385) / (3615 / 240);
      break;
    case 2:
      *x = (xraw - 385) / (3615 / 240);
      *y = (yraw - 305) / (3640 / 320);
      break;
    case 3:
      *x = (xraw - 351) / (3604 / 320);
      *y = (yraw - 191) / (3624 / 240);
      break;
    default:
      *x = xraw;
      *y = yraw;
      break;
  }

  // printf("[Info] Raw X: %d Y: %d | Point: X: %d Y: %d\n", xraw, yraw, *x, *y);
}

void HR2046::setRotation(uint8_t rot)
{
  rotation = rot % 4;
}

int16_t HR2046::bestTwoAvg(int16_t x, int16_t y, int16_t z)
{
  int16_t da, db, dc;
  int16_t reta = 0;

  if ( x > y ) da = x - y; else da = y - x;
  if ( x > z ) db = x - z; else db = z - x;
  if ( z > y ) dc = z - y; else dc = y - z;

  if(da <= db && da <= dc)
  {
    reta = (x + y) >> 1;
  }
  else if(db <= da && db <= dc)
  {
    reta = (x + z) >> 1;
  }
  else
  {
    reta = (y + z) >> 1;
  }

  return reta;
}

uint16_t HR2046::transfer16(uint8_t cmd)
{
  spi.format(16, 3);
  int16_t data = spi.write(cmd);
  spi.format(8, 3);

  return data;
}

void HR2046::update(void)
{
  uint16_t data[6];
  int z;

  chipSelect = 0;

  spi.write(0xB1);
  int16_t z1 = transfer16(0xC1) >> 3;
  z = z1 + 4095;
  int16_t z2 = transfer16(0x91) >> 3;
  z -= z2;

  if(z >= HR2046_Z_THRESHOLD)
  {
    transfer16(0x91); // Dummy X measure (1st is always noisy)
    data[0] = transfer16(0xD1) >> 3;
    data[1] = transfer16(0x91) >> 3;
    data[2] = transfer16(0xD1) >> 3;
    data[3] = transfer16(0x91) >> 3;
  }
  
  data[4] = transfer16(0xD0) >> 3;
  data[5] = transfer16(0x00) >> 3;

  chipSelect = 1;

  if(z < 0)
  {
    z = 0;
  }

  if(z < HR2046_Z_THRESHOLD)
  {
    zraw = 0;
    return;
  }

  zraw = z;

  // Average pair with least distance between each measured x then y
  int16_t x = bestTwoAvg(data[0], data[2], data[4]);
  int16_t y = bestTwoAvg(data[1], data[3], data[5]);

  if(z >= HR2046_Z_THRESHOLD)
  {
    switch(rotation)
    {
      case 0:
        xraw = 4095 - y;
        yraw = x;
        break;
      case 1:
        xraw = x;
        yraw = y;
        break;
      case 2:
        xraw = y;
        yraw = 4095 - x;
        break;
      case 3:
        xraw = 4095 - x;
        yraw = 4095 - y;
        break;
      default:
      break;
    }
  }
}