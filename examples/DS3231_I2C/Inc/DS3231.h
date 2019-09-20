/*
 * ds3232.cc
 *
 *  Created on: Sep 13, 2019
 *      Author: lamar
 */

#ifndef DRIVERS_DS3231_I2C_INCLUDE_DS3231_H_
#define DRIVERS_DS3231_I2C_INCLUDE_DS3231_H_

#include <cinttypes>
#include <ctime>

#include <chrono>  // NOLINT
#include <tuple>
#include <utility>

#include "i2c_if.h"  // NOLINT

namespace ds3231 {
constexpr uint16_t EPOCH = 2019;
using register_type = uint8_t;

namespace register_map {
constexpr register_type kSeconds = 0x00U;
constexpr register_type kMinutes = 0x01U;
constexpr register_type kHours = 0x02U;
constexpr register_type kDay = 0x03U;
constexpr register_type kDate = 0x04U;
constexpr register_type kMonthCentury = 0x05U;
constexpr register_type kYear = 0x06U;
constexpr register_type kAlarm1_Seconds = 0x07U;
constexpr register_type kAlarm1_Minutes = 0x08U;
constexpr register_type kAlarm1_Hours = 0x09U;
constexpr register_type kAlarm1_Day_Date = 0x0AU;
constexpr register_type kAlarm2_Minutes = 0x0BU;
constexpr register_type kAlarm2_Hours = 0x0CU;
constexpr register_type kAlarm2_Day_Date = 0x0DU;
constexpr register_type kControl = 0x0EU;
constexpr register_type kControlStatus = 0x0FU;
constexpr register_type kAgingOffset = 0x10U;
constexpr register_type kTempMsb = 0x11U;
constexpr register_type kTempLsb = 0x12U;
};  // namespace register_map
// namespace register_map

// no need for bit packing, mostly different registers

// Alarm1, Alarm1Rate
namespace alarm1 {

// bool once_per_second;

enum class Rate : uint8_t {
  OncePerSecond = 0U,
  Sec = 1U,
  MinSec = 2U,
  HrMinSec = 3U,
  DatHrMinSec = 4U,
  DayDatHrMinSec = 5U
};
// 12 hour timepoint
struct Timepoint {
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hour;
  bool use_week_day;
  uint8_t day;
};

};  // namespace alarm1

namespace alarm2 {
enum class Rate : uint8_t {
  OncePerMinute = 0U,
  Min = 1U,
  HrMin = 2U,
  DatHrMin = 3U,
  DayDatHrMin = 4U,
};

// 12 hour timepoint
struct Timepoint {
  uint8_t minutes;
  uint8_t hour;
  bool use_week_day;
  uint8_t day;
};
};  // namespace alarm2

template <typename R>
using Result = std::pair<R, Status>;

template <typename... R>
using MultiResult = std::tuple<R..., Status>;

/*
tm_sec [0, 60] (since C++11)
int tm_min  [0, 59]
int tm_hour – [0, 23]
int tm_mday  [1, 31]
int tm_mon [0, 11]
int tm_year years since 1900
int tm_wday 0,6
int tm_yday since 199
*/

struct Timepoint {
  uint8_t second;
  // TODO(lamarrr): Make sure we take care of 20 Hour format in case of already
  // set registers
  uint8_t hour;
  // day-of-week register increments at midnight.
  uint8_t week_day;
  // (i.e., if 1 equals Sunday, then 2 equals Monday, and so on).
  uint8_t month_day;
  uint8_t month;
  bool century_state;
  uint16_t year;
  // to ensure usability for strftime
  tm ToIso(void);
  bool IsValid(void);
};

class DS3231 {
 public:
  explicit DS3231(const I2cInterface&);
  DS3231(const DS3231&) = delete;
  DS3231(DS3231&&) = default;
  DS3231& operator=(const DS3231&) = delete;
  DS3231& operator=(DS3231&&) = default;
  ~DS3231() = default;



  Status Reset(duration_type timeout);


  // Get and Set semantics used especially due to user
  Result<Timepoint> GetTime(duration_type timeout) noexcept;
  Status SetTime(const Timepoint* time, duration_type timeout) noexcept;

  // Use if you're familiar with it's datasheet
  Result<uint8_t> SecondsFunc(duration_type timeout) noexcept;
  Status SecondsFunc(uint8_t data, duration_type timeout) noexcept;

  Result<uint8_t> MinutesFunc(duration_type timeout) noexcept;
  Status MinutesFunc(uint8_t data, duration_type timeout) noexcept;

  Result<uint8_t> HoursFunc(duration_type timeout) noexcept;
  Status HoursFunc(uint8_t data, duration_type timeout) noexcept;

  Result<uint8_t> WeekDaysFunc(duration_type timeout) noexcept;
  Status WeekDaysFunc(uint8_t data, duration_type timeout) noexcept;

  Result<uint8_t> DateFunc(duration_type timeout) noexcept;
  Status DateFunc(uint8_t data, duration_type timeout) noexcept;

  MultiResult<uint8_t, bool> MonthCenturyFunc(duration_type timeout) noexcept;
  Status MonthCenturyFunc(std::pair<uint8_t, bool> data,
                          duration_type timeout) noexcept;

  Result<uint16_t> YearFunc(duration_type timeout) noexcept;
  Status YearFunc(uint16_t data, duration_type timeout) noexcept;

  MultiResult<uint8_t, bool> Alarm1_SecondsFunc(duration_type timeout) noexcept;
  Status Alarm1_SecondsFunc(std::pair<uint8_t, bool> data,
                            duration_type timeout) noexcept;

  MultiResult<uint8_t, bool> Alarm1_MinutesFunc(duration_type timeout) noexcept;
  Status Alarm1_MinutesFunc(std::pair<uint8_t, bool> data,
                            duration_type timeout) noexcept;

  MultiResult<uint8_t, bool> Alarm1_HoursFunc(duration_type timeout) noexcept;
  Status Alarm1_HoursFunc(std::pair<uint8_t, bool> data,
                          duration_type timeout) noexcept;

  MultiResult<uint16_t, bool, bool> Alarm1_DayDateFunc(
      duration_type timeout) noexcept;
  Status Alarm1_DayDateFunc(std::tuple<uint16_t, bool, bool> data,
                            duration_type timeout) noexcept;

  MultiResult<uint8_t, bool> Alarm2_MinutesFunc(duration_type timeout) noexcept;
  Status Alarm2_MinutesFunc(std::pair<uint8_t, bool>,
                            duration_type timeout) noexcept;

  MultiResult<uint8_t, bool> Alarm2_HoursFunc(duration_type timeout) noexcept;
  Status Alarm2_HoursFunc(std::pair<uint8_t, bool>,
                          duration_type timeout) noexcept;

  MultiResult<uint8_t, bool, bool> Alarm2_DayDateFunc(
      duration_type timeout) noexcept;
  Status Alarm2_DayDateFunc(std::pair<uint8_t, bool>,
                            duration_type timeout) noexcept;

  Result<uint8_t> ControlFunc(duration_type timeout) noexcept;
  Status ControlFunc(uint8_t, duration_type timeout) noexcept;

  Result<uint8_t> ControlStatusFunc(duration_type timeout) noexcept;
  Status ControlStatusFunc(uint8_t, duration_type timeout) noexcept;

  Result<uint8_t> AgingOffsetFunc(duration_type timeout) noexcept;
  Status AgingOffsetFunc(uint8_t, duration_type timeout) noexcept;

  Result<uint8_t> TempMsbFunc(duration_type timeout) noexcept;
  Status TempLsbFunc(duration_type timeout) noexcept;

 private:
  I2cInterface interface_;

  Status WriteByte_(uint8_t reg, uint8_t byte, duration_type timeout) noexcept;
  Status BurstWriteBytes_(uint8_t reg, uint8_t* data, uint16_t size,
                          duration_type timeout) noexcept;
  Status SetBit_(uint8_t reg, uint8_t bit_index,
                 duration_type timeout) noexcept;
  Status UnsetBit_(uint8_t reg, uint8_t bit_index,
                   duration_type timeout) noexcept;
  Result<uint8_t> ReadByte_(uint8_t reg, duration_type timeout) noexcept;
  Result<uint8_t> BurstReadBytes_(uint8_t reg, uint8_t* data, uint16_t size,
                                  duration_type timeout) noexcept;
  Result<uint8_t> ReadBit_(uint8_t reg, uint8_t bit_index,
                           duration_type timeout) noexcept;
};
}  // namespace ds3231
#endif  // DRIVERS_DS3231_I2C_INCLUDE_DS3231_H_
