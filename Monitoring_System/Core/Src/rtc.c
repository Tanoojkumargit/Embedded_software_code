#include "rtc.h"
#include "i2c.h"


#define RTC_ADDRESS (0x68 << 1)  // Replace with your RTC module's I2C address

void RTC_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds) {
  uint8_t data[3];
  data[0] = (seconds / 10 << 4) | (seconds % 10);  // Convert to BCD
  data[1] = (minutes / 10 << 4) | (minutes % 10);  // Convert to BCD
  data[2] = (hours / 10 << 4) | (hours % 10);      // Convert to BCD
  HAL_I2C_Mem_Write(&hi2c1, RTC_ADDRESS, 0x00, 1, data, 3, HAL_MAX_DELAY);
}

void RTC_SetDate(uint8_t day, uint8_t month, uint16_t year) {
  uint8_t data[3];
  data[0] = (day / 10 << 4) | (day % 10);          // Convert to BCD
  data[1] = (month / 10 << 4) | (month % 10);      // Convert to BCD
  data[2] = ((year % 100) / 10 << 4) | (year % 10);// Convert to BCD
  HAL_I2C_Mem_Write(&hi2c1, RTC_ADDRESS, 0x04, 1, data, 3, HAL_MAX_DELAY);
}

void RTC_GetTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds) {
  uint8_t data[3];
  HAL_I2C_Mem_Read(&hi2c1, RTC_ADDRESS, 0x00, 1, data, 3, HAL_MAX_DELAY);
  *seconds = (data[0] >> 4) * 10 + (data[0] & 0x0F);
  *minutes = (data[1] >> 4) * 10 + (data[1] & 0x0F);
  *hours   = (data[2] >> 4) * 10 + (data[2] & 0x0F);
}

void RTC_GetDate(uint8_t *day, uint8_t *month, uint16_t *year) {
  uint8_t data[3];
  HAL_I2C_Mem_Read(&hi2c1, RTC_ADDRESS, 0x04, 1, data, 3, HAL_MAX_DELAY);
  *day     = (data[0] >> 4) * 10 + (data[0] & 0x0F);
  *month   = (data[1] >> 4) * 10 + (data[1] & 0x0F);
  *year    = 2000 + (data[2] >> 4) * 10 + (data[2] & 0x0F);
}
