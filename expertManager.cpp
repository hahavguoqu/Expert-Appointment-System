#include "expertManager.h"

ExpertManager::ExpertManager() {
  
}

Expert* ExpertManager::findExpertById(const QString& id) {
  for (auto& expert : experts) {
    if (expert.id == id) return &expert;
  }
  return nullptr;
}

bool ExpertManager::verifyExpert(const QString& id, const QString& password) {
  Expert* expert = findExpertById(id);
  return expert && expert->password == password;
}

bool ExpertManager::saveToFile(const QString& filename) const {
  // 使用临时文件确保原子性写入
  QString tempFilename = filename + ".tmp";
  QJsonArray expertArray;

  // 将所有专家信息转换为JSON对象
  for (const Expert& expert : experts) {
    QJsonObject expertObj;
    expertObj["id"] = expert.id;
    expertObj["name"] = expert.name;
    expertObj["password"] = expert.password;
    expertObj["gender"] = expert.gender;
    expertObj["age"] = expert.age;
    expertObj["title"] = expert.title;
    expertObj["subject"] = expert.subject;

    // 处理服务时间
    QJsonArray timeArray;
    for (const QString& time : expert.serviceTimes) {
      timeArray.append(time);
    }
    expertObj["serviceTimes"] = timeArray;

    // 处理出诊日期
    QJsonArray dateArray;
    for (const QDate& date : expert.scheduleDates) {
      dateArray.append(date.toString("yyyy-MM-dd"));
    }
    expertObj["scheduleDates"] = dateArray;

    // 处理停诊日期
    QJsonArray closedArray;
    for (const QDate& date : expert.closedDates) {
      closedArray.append(date.toString("yyyy-MM-dd"));
    }
    expertObj["closedDates"] = closedArray;

    // 处理时间段容量
    QJsonObject capacityObj;
    QMapIterator<QString, int> it(expert.timeSlotCapacity);
    while (it.hasNext()) {
      it.next();
      capacityObj[it.key()] = it.value();
    }
    expertObj["timeSlotCapacity"] = capacityObj;

    expertArray.append(expertObj);
  }

  // 创建JSON文档
  QJsonDocument doc(expertArray);
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

  qDebug() << "成功保存" << experts.size() << "个专家信息到文件：" << filename;
  return true;
}

bool ExpertManager::loadFromFile(const QString& filename) {
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
  experts.clear();

  // 解析JSON数组
  QJsonArray expertArray = doc.array();
  for (const QJsonValue& value : expertArray) {
    if (!value.isObject()) continue;

    QJsonObject obj = value.toObject();
    Expert expert;

    expert.id = obj["id"].toString();
    expert.name = obj["name"].toString();
    expert.password = obj["password"].toString();
    expert.gender = obj["gender"].toString();
    expert.age = obj["age"].toInt();
    expert.title = obj["title"].toString();
    expert.subject = obj["subject"].toString();

    // 加载服务时间
    QJsonArray timeArray = obj["serviceTimes"].toArray();
    for (const QJsonValue& timeValue : timeArray) {
      expert.serviceTimes.append(timeValue.toString());
    }

    // 加载出诊日期
    QJsonArray dateArray = obj["scheduleDates"].toArray();
    for (const QJsonValue& dateValue : dateArray) {
      QDate date = QDate::fromString(dateValue.toString(), "yyyy-MM-dd");
      if (date.isValid()) {
        expert.scheduleDates.append(date);
      }
    }

    // 加载停诊日期
    QJsonArray closedArray = obj["closedDates"].toArray();
    for (const QJsonValue& dateValue : closedArray) {
      QDate date = QDate::fromString(dateValue.toString(), "yyyy-MM-dd");
      if (date.isValid()) {
        expert.closedDates.append(date);
      }
    }

    // 加载时间段容量
    QJsonObject capacityObj = obj["timeSlotCapacity"].toObject();
    for (const QString& key : capacityObj.keys()) {
      expert.setTimeSlotCapacity(key, capacityObj[key].toInt());
    }

    experts.append(expert);
  }

  qDebug() << "成功从文件加载" << experts.size() << "个专家信息：" << filename;
  return true;
}

void ExpertManager::updateExpert(int index, const Expert& updatedExpert) {
  if (index >= 0 && index < experts.size()) {
    experts[index] = updatedExpert;
  }
}