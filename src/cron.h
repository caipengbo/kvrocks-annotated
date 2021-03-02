#pragma once

#include <ctime>
#include <iostream>
#include <vector>
#include <string>
#include "status.h"

// 周期设置（精确到分钟，再细的粒度，根据cron函数中的时间嘀嗒进行判断）
struct Scheduler {
  int minute;
  int hour;
  int mday;
  int month;
  int wday;

  std::string ToString() const;
};

// 这个相当于一个时间设置器，根据param提供的时间数据，封装成 Scheduler(时间点) 数据结构
// 提供到时判断函数 IsTimeMatch
class Cron {
 public:
  Cron() = default;
  ~Cron() = default;

  Status SetScheduleTime(const std::vector<std::string> &args);
  bool IsTimeMatch(struct tm *tm);
  std::string ToString();
  bool IsEnabled();

 private:
  std::vector<Scheduler> schedulers_;
  struct tm last_tm_ = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, nullptr};

  Status convertToScheduleTime(
      const std::string &minute,
      const std::string &hour,
      const std::string &mday,
      const std::string &month,
      const std::string &wday,
      Scheduler *st);
  Status convertParam(const std::string &param, int lower_bound, int upper_bound, int *value);
};
