/* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
miniob is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
         http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once

#include "common/type/data_type.h"

/**
 * @brief 日期类型
 * @ingroup DataType
 */
class DateType : public DataType
{
public:
  DateType() : DataType(AttrType::DATES) {}

  virtual ~DateType() = default;

  int compare(const Value &left, const Value &right) const override;

  RC add(const Value &left, const Value &right, Value &result) const override;
  RC subtract(const Value &left, const Value &right, Value &result) const override;
  RC multiply(const Value &left, const Value &right, Value &result) const override;
  RC divide(const Value &left, const Value &right, Value &result) const override;

  RC cast_to(const Value &val, AttrType type, Value &result) const override;

  RC to_string(const Value &val, string &result) const override;

  RC set_value_from_str(Value &val, const string &data) const override;

  // 注意：cast_cost 在基类中有默认实现，这里重写它

  /**
   * @brief 获取 DateType 的单例实例
   * @return DateType 实例指针
   */
  static const DateType *instance();

  /**
   * @brief 从字符串解析日期
   * @param str 日期字符串，格式为 YYYY-MM-DD
   * @param date_val 输出的日期值（YYYYMMDD 格式）
   * @return RC::SUCCESS 如果解析成功，否则返回错误码
   */
  RC str_to_date(const string &str, int &date_val) const;

private:
  /**
   * @brief 验证日期是否有效
   * @param year 年份
   * @param month 月份 (1-12)
   * @param day 日期 (1-31)
   * @return true 如果日期有效，false 否则
   */
  bool is_valid_date(int year, int month, int day) const;

  /**
   * @brief 判断是否为闰年
   * @param year 年份
   * @return true 如果是闰年，false 否则
   */
  bool is_leap_year(int year) const;

  /**
   * @brief 获取指定年月的天数
   * @param year 年份
   * @param month 月份 (1-12)
   * @return 该月的天数
   */
  int get_days_in_month(int year, int month) const;

  static const DateType instance_;
};