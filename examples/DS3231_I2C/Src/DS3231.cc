/*
 * DS3231.cc
 *
 *  Created on: Sep 20, 2019
 *      Author: lamar
 */

/*
 * ds3231.cc
 *
 *  Created on: Sep 13, 2019
 *      Author: lamar
 */

#include "DS3231.h"
#include "bcd.h"

#include "main.h"
#include "stm32f7xx_hal_uart.h"
#include <cstdio>

#define LOG(fmt, r...) printf("[Debug] " fmt "\n", r)

namespace bcd4_4 {
inline uint8_t Encode(uint8_t num) noexcept {
	uint8_t t = (num / 10U) & 0b1111U;
	uint8_t u = num % 10U;  // definitely not exceeding 4 bits
	return (t << 4U) | u;
}
;

inline uint8_t Decode(uint8_t bcd) noexcept {
	return (((bcd >> 4U) & 0b1111U) * 10U) + (bcd & 0b1111U);
}

}  // namespace bcd4_4
namespace bcd3_4 {
inline uint8_t Encode(uint8_t num) noexcept {
	uint8_t t = (num / 10U) & 0b111U;
	uint8_t u = num % 10U;  // definitely not exceeding 4 bits
	return (t << 4U) | u;
}
;

inline uint8_t Decode(uint8_t bcd) noexcept {
	return (((bcd >> 4U) & 0b111U) * 10U) + (bcd & 0b1111U);
}
}  // namespace bcd3_4

// slave address: 1101000 << 1
// TODO(lamarrr): remove all func suffix
// TODO(lamarrr): Add checking
// status only
#define CHECK_ERROR_S(x) \
  if ((x) != Status::Ok) return x
// result
#define CHECK_ERROR_R(x, r) \
  if ((x) != Status::Ok) return std::make_pair(r, x)
#define CHECK_ERROR(v) \
  if ((v.second) != Status::Ok) return v

namespace ds3231 {
uint8_t To24_Hours(uint8_t enc) {
	bool is_12_hour = static_cast<bool>(enc & (1 << 6));
	if (is_12_hour) {
		bool is_pm = static_cast<bool>(enc & (1 << 5));
		auto dec = bcd3_4::Decode(enc & 0b00011111);
		return is_pm ? dec + 12 : dec;

	} else {
		return bcd3_4::Decode(enc & 0b00111111);
	}
}

uint8_t Encode24Hours(uint8_t hr24) {
	return 0b00111111 & bcd3_4::Encode(hr24);
}

tm Timepoint::ToIso(void) {
}
bool Timepoint::IsValid(void) {
}

// TODO(lamarrr): mark all noexcept

DS3231::DS3231(const I2cInterface& interface) :
		interface_ { interface } {
}

Status DS3231::WriteByte_(uint8_t reg, uint8_t byte, duration_type timeout)
		noexcept {
	uint8_t data[] = { reg, byte };
	LOG("Sending: %u", byte);
	interface_.tx(data, sizeof(data), timeout);
}
Status DS3231::SetBit_(uint8_t reg, uint8_t bit_index, duration_type timeout)
		noexcept {
	auto res = ReadByte_(reg, timeout);
	CHECK_ERROR_S(res.second);
	res.first = res.first | (0b01 << bit_index);
	return WriteByte_(reg, res.first, timeout);
}
Status DS3231::UnsetBit_(uint8_t reg, uint8_t bit_index, duration_type timeout)
		noexcept {
	auto res = ReadByte_(reg, timeout);
	CHECK_ERROR_S(res.second);
	res.first = res.first & (0b11111111 ^ bit_index);
	return WriteByte_(reg, res.first, timeout);
}
Result<uint8_t> DS3231::ReadByte_(uint8_t reg, duration_type timeout) noexcept {
	uint8_t byte { };
	CHECK_ERROR_R(interface_.tx(&reg, 1, timeout), byte);
	auto status = interface_.rx(&byte, 1, timeout);
	return std::make_pair(byte, status);
}
Result<uint8_t> DS3231::ReadBit_(uint8_t reg, uint8_t bit_index,
		duration_type timeout) noexcept {
	auto res = ReadByte_(reg, timeout);
	res.first = (res.first & (0b01 << bit_index)) >> bit_index;
	return res;
}

Status DS3231::Reset(duration_type timeout) {
	return WriteByte_(register_map::kControl, (0b1 << 4) | (0b1 << 3), timeout);
}

Result<Timepoint> DS3231::GetTime(duration_type timeout) noexcept {
}
Status DS3231::SetTime(const Timepoint* time, duration_type timeout) noexcept {
}

// Use if you're familiar with it's datasheet
Result<uint8_t> DS3231::SecondsFunc(duration_type timeout) noexcept {
	auto res = ReadByte_(register_map::kSeconds, timeout);
	res.first = bcd3_4::Decode(res.first & 0b1111111);
	return res;
}
Status DS3231::SecondsFunc(uint8_t seconds, duration_type timeout) noexcept {
	return WriteByte_(register_map::kSeconds,
			bcd3_4::Encode(seconds) & 0b1111111, timeout);
}

Result<uint8_t> DS3231::MinutesFunc(duration_type timeout) noexcept {
	auto res = ReadByte_(register_map::kMinutes, timeout);
	res.first = bcd3_4::Decode(res.first & 0b1111111);
	return res;
}
Status DS3231::MinutesFunc(uint8_t minutes, duration_type timeout) noexcept {
	return WriteByte_(register_map::kMinutes,
			bcd3_4::Encode(minutes) & 0b1111111, timeout);
}

// return 24-hour based
Result<uint8_t> DS3231::HoursFunc(duration_type timeout) noexcept {
	auto res = ReadByte_(register_map::kHours, timeout);
	return std::make_pair(To24_Hours(res.first), res.second);
}
Status DS3231::HoursFunc(uint8_t hours, duration_type timeout) noexcept {
	return WriteByte_(register_map::kHours, Encode24Hours(hours), timeout);
}

// use iso
Result<uint8_t> DS3231::WeekDaysFunc(duration_type timeout) noexcept {
	auto res = ReadByte_(register_map::kDay, timeout);
	// iso
	res.first = (res.first & 0b111) - 1;
	return res;
}
Status DS3231::WeekDaysFunc(uint8_t wkday, duration_type timeout) noexcept {
	// iso
	return WriteByte_(register_map::kDay, (wkday + 1) & 0b111, timeout);
}

Result<uint8_t> DS3231::DateFunc(duration_type timeout) noexcept {
	auto res = ReadByte_(register_map::kDate, timeout);
	res.first = bcd3_4::Decode(res.first & 0b00111111);
	return res;
}
Status DS3231::DateFunc(uint8_t date, duration_type timeout) noexcept {
	return WriteByte_(register_map::kDate, bcd3_4::Encode(date) & 0b00111111,
			timeout);
}

MultiResult<uint8_t, bool> DS3231::MonthCenturyFunc(duration_type timeout)
		noexcept {
	auto res = ReadByte_(register_map::kMonthCentury, timeout);

	res.first = bcd3_4::Decode(res.first & 0b00011111) - 1;
	bool century_state = static_cast<bool>(res.first & (0b1 << 7));
	return std::make_tuple(res.first, century_state, res.second);
}
Status DS3231::MonthCenturyFunc(std::pair<uint8_t, bool> month_century,
		duration_type timeout) noexcept {
	uint8_t byte = static_cast<uint8_t>(month_century.second) << 7;
	byte |= (bcd3_4::Encode(month_century.first + 1) & 0b00011111);

	return WriteByte_(register_map::kMonthCentury, byte, timeout);
}

// change this code after a century, Haha
Result<uint16_t> DS3231::YearFunc(duration_type timeout) noexcept {
	// years since 2019, 0 for none
	uint16_t year = EPOCH;
	auto res_h = MonthCenturyFunc(timeout);
	CHECK_ERROR_R(std::get<2>(res_h), year);
	auto res_t = ReadByte_(register_map::kYear, timeout);
	bool century_flip = static_cast<uint8_t>(std::get<1>(res_h));
	year = (EPOCH + static_cast<uint16_t>(century_flip) * 100);
	year += bcd4_4::Decode(res_t.first);

	return std::make_pair(year, res_t.second);
}
Status DS3231::YearFunc(uint16_t year, duration_type timeout) noexcept {
	auto year_epoched = year - 2019;
	auto mc = MonthCenturyFunc(timeout);
	CHECK_ERROR_S(std::get<2>(mc));
	std::get<1>(mc) = year_epoched > 100;
	MonthCenturyFunc(std::make_pair(std::get<0>(mc), std::get<1>(mc)), timeout);

	return WriteByte_(register_map::kYear, bcd4_4::Encode(year_epoched),
			timeout);
}

MultiResult<uint8_t, bool> DS3231::Alarm1_SecondsFunc(duration_type timeout)
		noexcept {
	auto al = ReadByte_(register_map::kAlarm1_Seconds, timeout);
	bool alarm = static_cast<bool>(al.first >> 7);
	uint8_t seconds = bcd3_4::Decode(al.first & 0b01111111);

	return std::make_tuple(seconds, alarm, al.second);
}
Status DS3231::Alarm1_SecondsFunc(std::pair<uint8_t, bool> data,
		duration_type timeout) noexcept {
	uint8_t byte = (static_cast<uint8_t>(data.second) << 7)
			| bcd3_4::Encode(data.first);
	return WriteByte_(register_map::kAlarm1_Seconds, byte, timeout);
}

MultiResult<uint8_t, bool> DS3231::Alarm1_MinutesFunc(duration_type timeout)
		noexcept {
	auto al = ReadByte_(register_map::kAlarm1_Minutes, timeout);

	uint8_t mini = bcd3_4::Decode(0b01111111 & al.first);
	bool alarm = static_cast<bool>(al.first >> 7);
	return std::make_tuple(mini, alarm, al.second);
}
Status DS3231::Alarm1_MinutesFunc(std::pair<uint8_t, bool> data,
		duration_type timeout) noexcept {
	uint8_t byte = (static_cast<uint8_t>(data.second) << 7)
			| bcd3_4::Encode(data.first);
	return WriteByte_(register_map::kAlarm1_Minutes, byte, timeout);
}

MultiResult<uint8_t, bool> DS3231::Alarm1_HoursFunc(duration_type timeout)
		noexcept {
	auto res = ReadByte_(register_map::kAlarm1_Hours, timeout);

	return std::make_tuple(To24_Hours(res.first & 0b01111111),
			static_cast<bool>(res.first & (1 << 7)), res.second);
}
Status DS3231::Alarm1_HoursFunc(std::pair<uint8_t, bool> data,
		duration_type timeout) noexcept {
	return WriteByte_(register_map::kAlarm1_Hours,
			Encode24Hours(data.first)
					| static_cast<bool>(data.first & (1 << 7)), timeout);
}

MultiResult<uint16_t, bool, bool> DS3231::Alarm1_DayDateFunc(
		duration_type timeout) noexcept {
	auto res = ReadByte_(register_map::kAlarm1_Day_Date, timeout);
	bool is_day = static_cast<bool>(res.first & (1 << 6));
	return std::make_tuple(
			(is_day ?
					bcd3_4::Decode(res.first & 0b00001111) - 1 :
					bcd3_4::Decode(res.first & 0b00111111)), is_day,
			static_cast<bool>(res.first & 0b10000000), res.second);
}
Status DS3231::Alarm1_DayDateFunc(std::tuple<uint16_t, bool, bool> data,
		duration_type timeout) noexcept {
	uint16_t day_date = std::get<0>(data);
	bool is_day = std::get<1>(data);
	bool alarm = std::get<2>(data);
	uint8_t byte = (static_cast<uint8_t>(is_day) << 6)
			| (static_cast<uint8_t>(alarm) << 7);
	if (is_day) {
		byte |= (bcd3_4::Encode(day_date + 1) & 0b01111);
	} else {
		byte |= (bcd3_4::Encode(day_date) & 0b0111111);
	}

	return WriteByte_(register_map::kAlarm1_Day_Date, byte, timeout);
}

MultiResult<uint8_t, bool> DS3231::Alarm2_MinutesFunc(duration_type timeout)
		noexcept {
}
Status DS3231::Alarm2_MinutesFunc(std::pair<uint8_t, bool>,
		duration_type timeout) noexcept {
}

MultiResult<uint8_t, bool> DS3231::Alarm2_HoursFunc(duration_type timeout)
		noexcept {
}
Status DS3231::Alarm2_HoursFunc(std::pair<uint8_t, bool>, duration_type timeout)
		noexcept {
}

MultiResult<uint8_t, bool, bool> DS3231::Alarm2_DayDateFunc(
		duration_type timeout) noexcept {
}
Status DS3231::Alarm2_DayDateFunc(std::pair<uint8_t, bool>,
		duration_type timeout) noexcept {
}

Result<uint8_t> DS3231::ControlFunc(duration_type timeout) noexcept {
}
Status DS3231::ControlFunc(uint8_t, duration_type timeout) noexcept {
}

Result<uint8_t> DS3231::ControlStatusFunc(duration_type timeout) noexcept {
}
Status DS3231::ControlStatusFunc(uint8_t, duration_type timeout) noexcept {
}

Result<uint8_t> DS3231::AgingOffsetFunc(duration_type timeout) noexcept {
}
Status DS3231::AgingOffsetFunc(uint8_t, duration_type timeout) noexcept {
}

Result<uint8_t> DS3231::TempMsbFunc(duration_type timeout) noexcept {
}
Status DS3231::TempLsbFunc(duration_type timeout) noexcept {
}

}
;
// namespace ds3231
