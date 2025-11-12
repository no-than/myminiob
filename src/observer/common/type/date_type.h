#pragma once


#include "common/type/data_type.h"
#include <string>

class DateType : public DataType
{
public:
  DateType();
  virtual ~DateType() = default;

  static DateType *instance();

  int compare(const Value &left, const Value &right) const override;
  RC  to_string(const Value &val, string &result) const override;
  RC  set_value_from_str(Value &val, const string &data) const override;
  
  RC  str_to_date(const char *s, int32_t &date_val) const;
  RC  date_to_str(int32_t date_val, string &result) const;
};