/*
 * bcd3_4.h
 *
 *  Created on: 14 Sep 2019
 *      Author: lamar
 */

#ifndef DRIVERS_DS3231_I2C_INCLUDE_BCD3_4_H_
#define DRIVERS_DS3231_I2C_INCLUDE_BCD3_4_H_

#include <cinttypes>

namespace bcd4_4 {
inline uint8_t Encode(uint8_t num) noexcept;

inline uint8_t Decode(uint8_t bcd) noexcept;
}
namespace bcd3_4 {
inline uint8_t Encode(uint8_t num) noexcept;

inline uint8_t Decode(uint8_t bcd) noexcept;
}  // namespace bcd3_4

#endif  // DRIVERS_DS3231_I2C_INCLUDE_BCD3_4_H_
