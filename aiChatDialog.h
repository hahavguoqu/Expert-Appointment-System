#ifndef AICHATDIALOG_H
#define AICHATDIALOG_H

#include <QDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "appointmentManager.h"
#include "expertManager.h"


namespace Ui {
class AIChatDialog;
}

class QNetworkAccessManager;
class QNetworkReply;

class AIChatDialog : public QDialog {
  Q_OBJECT

 public:
  explicit AIChatDialog(ExpertManager* expertMgr,
                        AppointmentManager* appointmentMgr,
                        QWidget* parent = nullptr);
  ~AIChatDialog();

 private slots:
  void on_sendButton_clicked();
  void handleNetworkReply(QNetworkReply* reply);

 private:
  Ui::AIChatDialog* ui;
  ExpertManager* expertManager;
  AppointmentManager* appointmentManager;
  QNetworkAccessManager* networkManager;
  QString apiKey;

  QString formatSystemData();
  void appendMessage(const QString& message, bool isUser);
  void sendToDeepseekAPI(const QString& userQuery, const QString& systemData);
};

#endif  // AICHATDIALOG_H