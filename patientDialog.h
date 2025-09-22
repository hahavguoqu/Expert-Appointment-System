#ifndef PATIENTDIALOG_H
#define PATIENTDIALOG_H

#include <QDialog> 
#include <QCalendarWidget>

#include "appointmentManager.h"
#include "expertManager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class PatientDialog;
}
QT_END_NAMESPACE

class PatientDialog : public QDialog {  
  Q_OBJECT

 public:
  explicit PatientDialog(ExpertManager* expertMgr,
                         AppointmentManager* appointmentMgr,
                         QWidget* parent = nullptr);
  ~PatientDialog();

 private slots:
  void on_submitButton_clicked();
  void on_cancelButton_clicked();
  void on_departmentCombo_currentIndexChanged(int index);
  void on_idInput_textChanged();
  void on_phoneInput_textChanged();
  void on_expertCombo_currentIndexChanged(int index);
  void on_appointmentDateEdit_dateChanged(const QDate& date);

 public:
  Ui::PatientDialog* ui;
  ExpertManager* expertManager;
  AppointmentManager* appointmentManager;

  bool isValidIdNumber(const QString& idNumber);
  bool isValidPhoneNumber(const QString& phone);
  QString getGenderFromId(const QString& idNumber);
  int getAgeFromId(const QString& idNumber);
  void updateAgeAndGender(const QString& idNumber);
  void loadDepartments();
  void loadExperts(const QString& department);
  void loadServiceTimes(const QString& expertName);
  void updateAvailableDates();
  void loadExpertsByDepartment(const QString& department);
  void loadServiceTimesByDate(const QString& expertName, const QDate& date);

};

#endif 
