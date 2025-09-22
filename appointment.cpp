#include "appointment.h"

Appointment::Appointment() : age(0), queueNumber(0) {
  appointmentDate = QDate::currentDate();
}