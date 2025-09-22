#ifndef EXPERT_H
#define EXPERT_H

#include <QDate>
#include <QList>
#include <QMap>
#include <QString>

class Expert {
 public:
  QString id;
  QString name;
  QString gender;
  int age;
  QString title;
  QString subject;
  QString password;
  QList<QString> serviceTimes;  // 每周固定时间段 
  QList<QDate> scheduleDates;  // 特殊出诊日期
  QList<QDate> closedDates;    // 特殊停诊日期
  QMap<QString, int> timeSlotCapacity;

  Expert();

  void setTimeSlotCapacity(const QString& timeSlot, int capacity);
  int getTimeSlotCapacity(const QString& timeSlot) const;
  void removeTimeSlotCapacity(const QString& timeSlot);
  bool isAvailableOnDate(const QDate& date) const;  // 判断某日期是否出诊
  QStringList getAvailableTimeSlotsForDate(
      const QDate& date) const;  // 获取给定日期的有效时间段
  static QString getDayOfWeekString(const QDate& date);  // 获取日期对应的星期几
  void addSpecialDateTimeSlot(const QDate& date, const QString& timeRange,
                              int capacity = 5);  // 添加特殊出诊日及其时间段
  bool hasSpecialTimeSlot(
      const QDate& date) const;  // 检查某日期是否有特殊时间安排
  QStringList getSpecialTimeSlotsForDate(
      const QDate& date) const;  // 获取特殊出诊日的所有时间段
};

#endif