#ifndef APPOINTMENTMANAGER_H
#define APPOINTMENTMANAGER_H

#include <QList>

#include "appointment.h"

class AppointmentManager {
 public:
  AppointmentManager();

  bool addAppointment(const Appointment& appointment);
  void removeAppointment(int index);
  void updateAppointment(int index, const Appointment& appointment);

  QList<Appointment>& getAllAppointments();
  const QList<Appointment>& getAllAppointments() const;
  QList<Appointment> getAppointmentsByExpert(const QString& expertName) const;
  void updateServiceTimeForExpert(const QString& expertName,
                                  const QString& oldTime,
                                  const QString& newTime);
  bool saveToFile(const QString& filename) const;
  bool loadFromFile(const QString& filename);
  bool updateAppointment(const Appointment& updatedAppointment);

 private: 
  QList<Appointment> appointments;
};

#endif
