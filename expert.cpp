#include "expert.h"

Expert::Expert() : age(0) {}

void Expert::setTimeSlotCapacity(const QString& timeSlot, int capacity) {
  timeSlotCapacity[timeSlot] = capacity;
}

int Expert::getTimeSlotCapacity(const QString& timeSlot) const {
  return timeSlotCapacity.value(timeSlot, 1);  // 返回默认值10
}

void Expert::removeTimeSlotCapacity(const QString& timeSlot) {
  timeSlotCapacity.remove(timeSlot);
}

// 新判断某日期是否出诊
bool Expert::isAvailableOnDate(const QDate& date) const {
  if (closedDates.contains(date)) return false;  // 检查是否为特殊停诊日期
  if (scheduleDates.contains(date)) return true;  // 检查是否为特殊出诊日期
  QString dayOfWeek = getDayOfWeekString(date);  // 检查是否为常规出诊日

  // 检查该星期几是否有安排
  for (const QString& timeSlot : serviceTimes) {
    if (timeSlot.startsWith(dayOfWeek)) return true;
  }

  return false;
}

// 获取给定日期的有效时间段
QStringList Expert::getAvailableTimeSlotsForDate(const QDate& date) const {
  QStringList availableSlots;

  // 检查是否为特殊出诊日
  if (scheduleDates.contains(date)) {
    // 特殊出诊日：返回以具体日期开头的时间段
    QString dateStr = date.toString("MM-dd：");
    for (const QString& timeSlot : serviceTimes) {
      if (timeSlot.startsWith(dateStr)) {
        availableSlots.append(timeSlot);  // 返回完整时间段，包括日期前缀
      }
    }

    // 如果没有找到特定时间段，检查是否该日的周几有常规安排
    if (availableSlots.isEmpty()) {
      QString dayOfWeek = getDayOfWeekString(date);
      for (const QString& timeSlot : serviceTimes) {
        if (timeSlot.startsWith(dayOfWeek)) {
          availableSlots.append(timeSlot);
        }
      }
    }
  } else {
    // 常规出诊日：返回以星期几开头的时间段
    QString dayOfWeek = getDayOfWeekString(date);
    for (const QString& timeSlot : serviceTimes) {
      if (timeSlot.startsWith(dayOfWeek)) {
        availableSlots.append(timeSlot);
      }
    }
  }

  return availableSlots;
}

// 获取日期对应的星期几字符串
QString Expert::getDayOfWeekString(const QDate& date) {
  int dayOfWeek = date.dayOfWeek();
  switch (dayOfWeek) {
    case 1:
      return "周一";
    case 2:
      return "周二";
    case 3:
      return "周三";
    case 4:
      return "周四";
    case 5:
      return "周五";
    case 6:
      return "周六";
    case 7:
      return "周日";
    default:
      return "";
  }
}


void Expert::addSpecialDateTimeSlot(const QDate& date, const QString& timeRange,
                                    int capacity) {
  // 添加到特殊出诊日列表
  if (!scheduleDates.contains(date)) {
    scheduleDates.append(date);
  }

  // 添加特殊时间段，格式为 "MM-dd：HH:mm-HH:mm"
  QString timeSlot = date.toString("MM-dd：") + timeRange;
  if (!serviceTimes.contains(timeSlot)) {
    serviceTimes.append(timeSlot);
  }

  // 设置容量
  setTimeSlotCapacity(timeSlot, capacity);
}

bool Expert::hasSpecialTimeSlot(const QDate& date) const {
  if (!scheduleDates.contains(date)) {
    return false;
  }

  QString dateStr = date.toString("MM-dd：");
  for (const QString& timeSlot : serviceTimes) {
    if (timeSlot.startsWith(dateStr)) {
      return true;
    }
  }

  return false;
}

QStringList Expert::getSpecialTimeSlotsForDate(const QDate& date) const {
  QStringList result;

  QString dateStr = date.toString("MM-dd：");
  for (const QString& timeSlot : serviceTimes) {
    if (timeSlot.startsWith(dateStr)) {
      // 只返回时间部分，去掉日期前缀
      result.append(timeSlot.mid(dateStr.length()));
    }
  }

  return result;
}