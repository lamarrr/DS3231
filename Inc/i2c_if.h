/*
 * i2c_if.h
 *
 *  Created on: Sep 14, 2019
 *      Author: lamar
 */

#ifndef DRIVERS_DS3231_I2C_INCLUDE_I2C_IF_H_
#define DRIVERS_DS3231_I2C_INCLUDE_I2C_IF_H_

#include <functional>

namespace ds3231 {

enum class Status : uint8_t { Ok = 0, Error = 1, Busy = 2, Timeout = 3 };

using duration_type = std::chrono::duration<uint32_t, std::milli>;

// std::bind
typedef Status(I2cTxFunctionT)(const uint8_t* buffer, uint16_t size,
                               duration_type timeout);
typedef Status(I2cRxFunctionT)(uint8_t* buffer, uint16_t size,
                               duration_type timeout);

struct I2cInterface {
  std::function<I2cTxFunctionT> tx;
  std::function<I2cRxFunctionT> rx;
};
};      // namespace ds3231
#endif  // DRIVERS_DS3231_I2C_INCLUDE_I2C_IF_H_
