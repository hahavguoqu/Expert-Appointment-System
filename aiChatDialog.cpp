#include "aiChatDialog.h"

#include <QLibrary>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QScrollBar>
#include <QSettings>
#include <QSslSocket>
#include <QUrl>
#include <QUrlQuery>

#include "ui_aiChatDialog.h"


AIChatDialog::AIChatDialog(ExpertManager* expertMgr,
                           AppointmentManager* appointmentMgr, QWidget* parent)
    : QDialog(parent),
      ui(new Ui::AIChatDialog),
      expertManager(expertMgr),
      appointmentManager(appointmentMgr),
      networkManager(new QNetworkAccessManager(this)) {
  ui->setupUi(this);

  resize(600, 500); 
  setWindowTitle("医疗预约AI助手");

  apiKey = "sk-114514";

  connect(networkManager, &QNetworkAccessManager::finished, this,
          &AIChatDialog::handleNetworkReply);

  appendMessage("您好！我是医院预约AI助手。请问有什么可以帮您？", false);
}

AIChatDialog::~AIChatDialog() { delete ui; }

void AIChatDialog::on_sendButton_clicked() {
  QString userMessage = ui->messageInput->text().trimmed();
  if (userMessage.isEmpty()) return;

  // 显示用户消息
  appendMessage(userMessage, true);
  ui->messageInput->clear();

  // 准备系统数据
  QString systemData = formatSystemData();

  // 发送到API
  sendToDeepseekAPI(userMessage, systemData);
}

QString AIChatDialog::formatSystemData() {
  QJsonObject data;

  // 添加专家信息
  QJsonArray expertsArray;
  for (const Expert& expert : expertManager->experts) {
    QJsonObject expertObj;
    expertObj["id"] = expert.id;
    expertObj["name"] = expert.name;
    expertObj["subject"] = expert.subject;
    expertObj["gender"] = expert.gender;
    expertObj["title"] = expert.title;

    // 添加出诊时间
    QJsonArray timesArray;
    for (const QString& timeSlot : expert.serviceTimes) {
      timesArray.append(timeSlot);
    }
    expertObj["serviceTimes"] = timesArray;

    expertsArray.append(expertObj);
  }
  data["experts"] = expertsArray;

  // 添加预约信息
  QJsonArray appointmentsArray;
  for (const Appointment& appt : appointmentManager->getAllAppointments()) {
    QJsonObject apptObj;
    apptObj["patientName"] = appt.patientName;
    apptObj["expertName"] = appt.expertName;
    apptObj["date"] = appt.appointmentDate.toString("yyyy-MM-dd");
    apptObj["timeSlot"] = appt.serviceTime;
    apptObj["subject"] = appt.expertSubject;

    appointmentsArray.append(apptObj);
  }
  data["appointments"] = appointmentsArray;

  // 添加当前日期
  data["currentDate"] = QDate::currentDate().toString("yyyy-MM-dd");

  return QJsonDocument(data).toJson(QJsonDocument::Compact);
}

void AIChatDialog::sendToDeepseekAPI(const QString& userQuery,
                                     const QString& systemData) {
  QUrl url("https://api.deepseek.com/v1/chat/completions");
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("Authorization",
                       QString("Bearer %1").arg(apiKey).toUtf8());

  // 准备请求体
  QJsonObject requestBody;
  requestBody["model"] = "deepseek-chat";

  QJsonArray messagesArray;

  // 系统消息包含所有数据
  QJsonObject systemMessage;
  systemMessage["role"] = "system";
  systemMessage["content"] =
      QString(
          "你是一个医院预约系统的AI助手，负责回答用户关于医生和预约的问题。"
          "以下是当前的医生和预约信息: %1"
          "请根据这些数据回答用户问题，回答要简洁、准确，并且只使用中文。"
          "你可以帮助用户查询医生出诊时间、推荐合适的医生、查看预约情况等。")
          .arg(systemData);
  messagesArray.append(systemMessage);

  // 用户消息
  QJsonObject userMessage;
  userMessage["role"] = "user";
  userMessage["content"] = userQuery;
  messagesArray.append(userMessage);

  requestBody["messages"] = messagesArray;
  requestBody["temperature"] = 0.3;  // 低温度保证回答准确性

  // 发送请求
  QJsonDocument doc(requestBody);
  networkManager->post(request, doc.toJson());

  // 显示等待消息
  ui->chatHistory->setEnabled(false);
  ui->sendButton->setEnabled(false);
  appendMessage("正在思考...", false);
}

void AIChatDialog::handleNetworkReply(QNetworkReply* reply) {
  ui->chatHistory->setEnabled(true);
  ui->sendButton->setEnabled(true);

  // 删除"正在思考..."消息
  QTextCursor cursor = ui->chatHistory->textCursor();
  cursor.movePosition(QTextCursor::End);
  cursor.select(QTextCursor::LineUnderCursor);
  cursor.removeSelectedText();

  if (reply->error() == QNetworkReply::NoError) {
    // 解析JSON响应
    QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
    QString aiMessage;

    if (response.isObject()) {
      QJsonObject responseObj = response.object();
      if (responseObj.contains("choices") && responseObj["choices"].isArray()) {
        QJsonArray choices = responseObj["choices"].toArray();
        if (!choices.isEmpty() && choices[0].isObject()) {
          QJsonObject firstChoice = choices[0].toObject();
          if (firstChoice.contains("message") &&
              firstChoice["message"].isObject()) {
            QJsonObject message = firstChoice["message"].toObject();
            aiMessage = message["content"].toString();
          }
        }
      }
    }

    if (!aiMessage.isEmpty()) {
      appendMessage(aiMessage, false);
    } else {
      appendMessage("抱歉，我无法正确理解API返回的结果。", false);
    }
  } else {
    // 处理错误
    appendMessage("抱歉，发生了错误：" + reply->errorString(), false);
  }

  reply->deleteLater();
}

void AIChatDialog::appendMessage(const QString& message, bool isUser) {
  QString style = isUser ? "background-color: #DCF8C6; border-radius: 10px; "
                           "padding: 8px; margin: 4px;"
                         : "background-color: #F5F5F5; border-radius: 10px; "
                           "padding: 8px; margin: 4px;";

  QString alignStyle = isUser ? "text-align: right;" : "text-align: left;";
  QString sender = isUser ? "您" : "AI助手";

  ui->chatHistory->append(
      QString("<div style='%1'><b>%2:</b><div style='%3'>%4</div></div>")
          .arg(alignStyle)
          .arg(sender)
          .arg(style)
          .arg(message));

  // 滚动到底部
  QScrollBar* sb = ui->chatHistory->verticalScrollBar();
  sb->setValue(sb->maximum());
}
