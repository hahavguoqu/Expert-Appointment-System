#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include <QDate>
#include <QDateTime>
#include <QString>


class Appointment {
 public:
  Appointment();

  QString patientName;
  QString idNumber;
  QString gender;
  int age;
  QString phone;
  QString expertName;
  QString expertSubject;
  QString serviceTime;
  QString description;
  int queueNumber;
  QDate appointmentDate;  
};

#endif 
