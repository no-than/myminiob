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
  RC      rc = str_to_date(data.c_str(), date_val);
  if (OB_FAIL(rc)) {
    LOG_WARN("failed to convert string to date. s=%s", data.c_str());
    return rc;
  }
  val.set_date(date_val);
  return RC::SUCCESS;
}

RC DateType::str_to_date(const char* s, int32_t& date) const {
  if (s == nullptr) {
    return RC::INVALID_ARGUMENT; // 或其他错误码
  }

  int year, month, day;
  if (sscanf(s, "%4d-%2d-%2d", &year, &month, &day) != 3) {
    return RC::INVALID_ARGUMENT;
  }

  date = year * 10000 + month * 100 + day;
  return RC::SUCCESS;
}
