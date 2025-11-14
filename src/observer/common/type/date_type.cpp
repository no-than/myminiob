/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "common/type/date_type.h"
#include "common/lang/comparator.h"
#include "common/lang/sstream.h"
#include "common/log/log.h"
#include "common/value.h"
#include "common/lang/limits.h"
#include "storage/common/column.h"
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <string>

const DateType DateType::instance_;

const DateType *DateType::instance()
{
  return &instance_;
}

RC DateType::str_to_date(const string &str, int &date_val) const
{
  // 去除首尾空格
  string trimmed_data = str;
  trimmed_data.erase(trimmed_data.begin(), std::find_if(trimmed_data.begin(), trimmed_data.end(), [](unsigned char ch) {
    return !std::isspace(ch);
  }));
  trimmed_data.erase(std::find_if(trimmed_data.rbegin(), trimmed_data.rend(), [](unsigned char ch) {
    return !std::isspace(ch);
  }).base(), trimmed_data.end());
  
  // 处理带时间的日期格式 YYYY-MM-DD HH:MM:SS
  if (trimmed_data.length() == 19 && trimmed_data[10] == ' ') {
    trimmed_data = trimmed_data.substr(0, 10); // 只取日期部分
  }
  
  // 验证日期格式 YYYY-MM-DD
  if (trimmed_data.length() != 10) {
    LOG_WARN("Invalid date format length: %s", trimmed_data.c_str());
    return RC::INVALID_ARGUMENT;
  }
  
  if (trimmed_data[4] != '-' || trimmed_data[7] != '-') {
    LOG_WARN("Invalid date format separators: %s", trimmed_data.c_str());
    return RC::INVALID_ARGUMENT;
  }
  
  // 验证每个字符是否为数字
  for (size_t i = 0; i < trimmed_data.length(); ++i) {
    if (i == 4 || i == 7) continue;  // 跳过分隔符
    if (!std::isdigit(trimmed_data[i])) {
      LOG_WARN("Invalid date format, non-digit character: %s", trimmed_data.c_str());
      return RC::INVALID_ARGUMENT;
    }
  }
  
  // 解析年月日
  int year, month, day;
  try {
    string year_str = trimmed_data.substr(0, 4);
    string month_str = trimmed_data.substr(5, 2);
    string day_str = trimmed_data.substr(8, 2);
    
    year = std::stoi(year_str);
    month = std::stoi(month_str);
    day = std::stoi(day_str);
  } catch (const std::exception &e) {
    LOG_WARN("Failed to parse date components: %s", trimmed_data.c_str());
    return RC::INVALID_ARGUMENT;
  }
  
  // 验证日期有效性
  if (!is_valid_date(year, month, day)) {
    LOG_WARN("Invalid date: %04d-%02d-%02d", year, month, day);
    return RC::INVALID_ARGUMENT;
  }
  
  // 将日期转换为整数存储 (YYYYMMDD 格式)
  date_val = year * 10000 + month * 100 + day;
  
  return RC::SUCCESS;
}

int DateType::compare(const Value &left, const Value &right) const
{
  ASSERT(left.attr_type() == AttrType::DATES, "left value should be date");
  ASSERT(right.attr_type() == AttrType::DATES, "right value should be date");
  
  int left_date = left.get_int();  // 使用 get_int() 而不是 get_date()
  int right_date = right.get_int();
  
  if (left_date < right_date) {
    return -1;
  }
  if (left_date > right_date) {
    return 1;
  }
  return 0;
}

RC DateType::add(const Value &left, const Value &right, Value &result) const
{
  // 日期 + 整数天数
  if (left.attr_type() == AttrType::DATES && right.attr_type() == AttrType::INTS) {
    int date_val = left.get_int();  // 使用 get_int() 而不是 get_date()
    int days = right.get_int();
    
    // 简单实现：直接在 YYYYMMDD 格式上加天数
    int year = date_val / 10000;
    int month = (date_val % 10000) / 100;
    int day = date_val % 100;
    
    day += days;
    
    // 处理日期溢出（简化版本）
    while (day > get_days_in_month(year, month)) {
      day -= get_days_in_month(year, month);
      month++;
      if (month > 12) {
        month = 1;
        year++;
      }
    }
    
    while (day <= 0) {
      month--;
      if (month <= 0) {
        month = 12;
        year--;
      }
      day += get_days_in_month(year, month);
    }
    
    int result_date = year * 10000 + month * 100 + day;
    result.set_int(result_date);  // 使用 set_int() 而不是 set_date()
    return RC::SUCCESS;
  }
  
  // 整数天数 + 日期
  if (left.attr_type() == AttrType::INTS && right.attr_type() == AttrType::DATES) {
    return add(right, left, result);
  }
  
  return RC::UNSUPPORTED;
}

RC DateType::subtract(const Value &left, const Value &right, Value &result) const
{
  // 日期 - 日期，返回天数差
  if (left.attr_type() == AttrType::DATES && right.attr_type() == AttrType::DATES) {
    int left_date = left.get_int();   // 使用 get_int() 而不是 get_date()
    int right_date = right.get_int();
    
    // 简化实现：直接计算 YYYYMMDD 格式的差值
    int diff = left_date - right_date;
    result.set_int(diff);
    return RC::SUCCESS;
  }
  
  // 日期 - 整数天数，返回新日期
  if (left.attr_type() == AttrType::DATES && right.attr_type() == AttrType::INTS) {
    Value neg_right;
    neg_right.set_int(-right.get_int());
    return add(left, neg_right, result);
  }
  
  return RC::UNSUPPORTED;
}

RC DateType::multiply(const Value &left, const Value &right, Value &result) const
{
  return RC::UNSUPPORTED;
}

RC DateType::divide(const Value &left, const Value &right, Value &result) const
{
  return RC::UNSUPPORTED;
}

RC DateType::cast_to(const Value &val, AttrType type, Value &result) const
{
  switch (type) {
    case AttrType::CHARS: {
      string str_val;
      RC rc = to_string(val, str_val);
      if (rc != RC::SUCCESS) {
        return rc;
      }
      result.set_string(str_val.c_str());
      break;
    }
    case AttrType::DATES: {
      result = val;
      break;
    }
    default: {
      return RC::UNSUPPORTED;
    }
  }
  return RC::SUCCESS;
}

RC DateType::set_value_from_str(Value &val, const string &data) const
{
  int date_val;
  RC rc = str_to_date(data, date_val);
  if (rc != RC::SUCCESS) {
    return rc;
  }
  
  val.set_int(date_val);  // 使用 set_int() 而不是 set_date()
  val.set_type(AttrType::DATES);  // 明确设置类型
  return RC::SUCCESS;
}

RC DateType::to_string(const Value &val, string &result) const
{
  int date_value = val.get_int();  // 使用 get_int() 而不是 get_date()
  
  int year = date_value / 10000;
  int month = (date_value % 10000) / 100;
  int day = date_value % 100;
  
  std::ostringstream oss;
  oss << std::setfill('0') << std::setw(4) << year << "-"
      << std::setw(2) << month << "-"
      << std::setw(2) << day;
  
  result = oss.str();
  return RC::SUCCESS;
}

bool DateType::is_valid_date(int year, int month, int day) const
{
  // 验证年份范围
  if (year < 1000 || year > 9999) {
    return false;
  }
  
  // 验证月份范围
  if (month < 1 || month > 12) {
    return false;
  }
  
  // 验证日期范围
  if (day < 1 || day > get_days_in_month(year, month)) {
    return false;
  }
  
  return true;
}

bool DateType::is_leap_year(int year) const
{
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int DateType::get_days_in_month(int year, int month) const
{
  static const int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  
  if (month < 1 || month > 12) {
    return 0;
  }
  
  if (month == 2 && is_leap_year(year)) {
    return 29;
  }
  
  return days_in_month[month];
}