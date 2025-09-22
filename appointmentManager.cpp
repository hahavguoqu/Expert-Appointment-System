#include "appointmentManager.h"

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

AppointmentManager::AppointmentManager() {
  // 构造函数
}

bool AppointmentManager::addAppointment(const Appointment& appointment) {
  appointments.append(appointment);
  qDebug() << "添加预约：" << appointment.patientName << " -> "
           << appointment.expertName;
  return true;
}

void AppointmentManager::removeAppointment(int index) {
  if (index >= 0 && index < appointments.size()) {
    qDebug() << "删除预约：" << appointments[index].patientName;
    appointments.removeAt(index);
  }
}

void AppointmentManager::updateAppointment(int index,
                                           const Appointment& appointment) {
  if (index >= 0 && index < appointments.size()) {
    appointments[index] = appointment;
    qDebug() << "更新预约：" << appointment.patientName;
  }
}

// 非const版本 - 允许修改
QList<Appointment>& AppointmentManager::getAllAppointments() {
  return appointments;
}

// const版本 - 只读访问
const QList<Appointment>& AppointmentManager::getAllAppointments() const {
  return appointments;
}

QList<Appointment> AppointmentManager::getAppointmentsByExpert(
    const QString& expertName) const {
  QList<Appointment> result;
  for (const auto& appointment : appointments) {
    if (appointment.expertName == expertName) {
      result.append(appointment);
    }
  }
  return result;
}

void AppointmentManager::updateServiceTimeForExpert(const QString& expertName,
                                                    const QString& oldTime,
                                                    const QString& newTime) {
  int updatedCount = 0;
  for (auto& appointment : appointments) {
    if (appointment.expertName == expertName &&
        appointment.serviceTime == oldTime) {
      appointment.serviceTime = newTime;
      updatedCount++;
      qDebug() << "更新预约时间：" << appointment.patientName << oldTime
               << " -> " << newTime;
    }
  }
  qDebug() << "共更新了" << updatedCount << "个预约的服务时间";
}

bool AppointmentManager::saveToFile(const QString& filename) const {
  // 使用临时文件确保原子性写入
  QString tempFilename = filename + ".tmp";
  QJsonArray appointmentArray;

  // 将所有预约转换为JSON对象
  for (const Appointment& appointment : appointments) {
    QJsonObject appointmentObj;
    appointmentObj["patientName"] = appointment.patientName;
    appointmentObj["gender"] = appointment.gender;
    appointmentObj["age"] = appointment.age;
    appointmentObj["idNumber"] = appointment.idNumber;
    appointmentObj["phone"] = appointment.phone;
    appointmentObj["description"] = appointment.description;
    appointmentObj["expertName"] = appointment.expertName;
    appointmentObj["expertSubject"] = appointment.expertSubject;
    appointmentObj["serviceTime"] = appointment.serviceTime;
    appointmentObj["queueNumber"] = appointment.queueNumber;

    // 保存日期
    if (appointment.appointmentDate.isValid()) {
      appointmentObj["appointmentDate"] =
          appointment.appointmentDate.toString("yyyy-MM-dd");
    }

    appointmentArray.append(appointmentObj);
  }

  // 创建JSON文档
  QJsonDocument doc(appointmentArray);
  QFile file(tempFilename);

  if (!file.open(QIODevice::WriteOnly)) {
    qDebug() << "无法打开临时文件进行写入: " << tempFilename;
    return false;
  }

  QByteArray data = doc.toJson();
  qint64 bytesWritten = file.write(data);
  file.close();

  if (bytesWritten != data.size()) {
    qDebug() << "写入数据不完整，期望" << data.size() << "字节，实际写入" << bytesWritten << "字节";
    QFile::remove(tempFilename);
    return false;
  }

  // 原子性重命名
  if (QFile::exists(filename)) {
    QFile::remove(filename);
  }
  
  if (!QFile::rename(tempFilename, filename)) {
    qDebug() << "无法重命名临时文件到目标文件: " << tempFilename << " -> " << filename;
    QFile::remove(tempFilename);
    return false;
  }

  qDebug() << "成功保存" << appointments.size() << "个预约到文件：" << filename;
  return true;
}

bool AppointmentManager::loadFromFile(const QString& filename) {
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "无法打开文件: " << filename;
    return false;
  }

  QByteArray data = file.readAll();
  file.close();

  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (doc.isNull() || !doc.isArray()) {
    qDebug() << "无效的JSON格式";
    return false;
  }

  // 清空现有数据
  appointments.clear();

  // 解析JSON数组
  QJsonArray appointmentArray = doc.array();
  for (const QJsonValue& value : appointmentArray) {
    if (!value.isObject()) continue;

    QJsonObject obj = value.toObject();
    Appointment appointment;

    appointment.patientName = obj["patientName"].toString();
    appointment.gender = obj["gender"].toString();
    appointment.age = obj["age"].toInt();
    appointment.idNumber = obj["idNumber"].toString();
    appointment.phone = obj["phone"].toString();
    appointment.description = obj["description"].toString();
    appointment.expertName = obj["expertName"].toString();
    appointment.expertSubject = obj["expertSubject"].toString();
    appointment.serviceTime = obj["serviceTime"].toString();
    appointment.queueNumber = obj["queueNumber"].toInt();

    // 加载日期
    if (obj.contains("appointmentDate")) {
      appointment.appointmentDate =
          QDate::fromString(obj["appointmentDate"].toString(), "yyyy-MM-dd");
    }

    appointments.append(appointment);
  }

  qDebug() << "成功从文件加载" << appointments.size() << "个预约：" << filename;
  return true;
}

bool AppointmentManager::updateAppointment(
    const Appointment& updatedAppointment) {
  for (int i = 0; i < appointments.size(); ++i) {
    if (appointments[i].patientName == updatedAppointment.patientName &&
        appointments[i].appointmentDate == updatedAppointment.appointmentDate &&
        appointments[i].expertName == updatedAppointment.expertName &&
        appointments[i].serviceTime == updatedAppointment.serviceTime) {
      appointments[i] = updatedAppointment;
      return true;
    }
  }
  return false;
}

