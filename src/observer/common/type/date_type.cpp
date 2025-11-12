#include "common/type/date_type.h"
#include "common/value.h"
#include "common/lang/comparator.h"
#include "common/log/log.h"
#include "common/lang/string.h"

DateType *DateType::instance()
{
  static DateType instance;
  return &instance;
}

DateType::DateType() : DataType(AttrType::DATES) {}

int DateType::compare(const Value &left, const Value &right) const
{
  ASSERT(left.attr_type() == AttrType::DATES && right.attr_type() == AttrType::DATES, 
         "invalid type");
  
  int left_date = left.get_date();
  int right_date = right.get_date();
  
  if (left_date < right_date) {
    return -1;
  } else if (left_date > right_date) {
    return 1;
  } else {
    return 0;
  }
}

RC DateType::to_string(const Value &val, string &result) const
{
  int32_t date = val.get_date();
  int year = date / 10000;
  int month = (date / 100) % 100;
  int day = date % 100;
  
  char buf[16];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d", year, month, day);
  result = buf;
  return RC::SUCCESS;
}

RC DateType::set_value_from_str(Value &val, const std::string &data) const
{
  int32_t date_val;
  RC rc = str_to_date(data.c_str(), date_val);
  if (rc != RC::SUCCESS) {
    // 如果日期无效，返回 RC::FAILURE 并记录日志
    LOG_WARN("Failed to convert string to date: '%s', returning FAILURE.", data.c_str());
    return RC::FAILURE;  // 返回自定义的 FAILURE 错误码
  }
  val.set_date(date_val);
  return RC::SUCCESS;
}

RC DateType::str_to_date(const char* s, int32_t& date) const
{
  if (s == nullptr) {
    LOG_WARN("Date string is null.");
    return RC::INVALID_ARGUMENT; // 处理空指针错误
  }

  int year, month, day;
  
  // 使用 sscanf 解析日期字符串
  if (sscanf(s, "%4d-%2d-%2d", &year, &month, &day) != 3) {
    LOG_WARN("Failed to parse date from string: %s", s);
    return RC::INVALID_ARGUMENT;  // 如果解析失败，返回无效参数错误
  }

  // 检查日期的有效性
  if (month < 1 || month > 12 || day < 1 || day > 31) {
    LOG_WARN("Invalid month or day in date string: %s", s);
    return RC::INVALID_ARGUMENT;  // 无效的月份或日期
  }

  // 对于2月，检查闰年和日期有效性
  if (month == 2) {
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
      // 闰年，2月有29天
      if (day > 29) {
        LOG_WARN("Invalid day in February for leap year: %s", s);
        return RC::INVALID_ARGUMENT;
      }
    } else {
      // 非闰年，2月只有28天
      if (day > 28) {
        LOG_WARN("Invalid day in February for non-leap year: %s", s);
        return RC::INVALID_ARGUMENT;
      }
    }
  }

  // 对于4月、6月、9月、11月，检查日期有效性（最大31天）
  if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30) {
    LOG_WARN("Invalid day in month with 30 days: %s", s);
    return RC::INVALID_ARGUMENT;
  }

  // 如果日期有效，则返回成功，并将日期存储为整数格式 (YYYYMMDD)
  date = year * 10000 + month * 100 + day;
  return RC::SUCCESS;
}
