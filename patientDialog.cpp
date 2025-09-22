#include "patientDialog.h"

#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QRegularExpression>

#include "appointmentManager.h"
#include "ui_PatientDialog.h"

PatientDialog::PatientDialog(ExpertManager* expertMgr,
                             AppointmentManager* appointmentMgr,
                             QWidget* parent)
    : QDialog(parent),
      ui(new Ui::PatientDialog),
      expertManager(expertMgr),
      appointmentManager(appointmentMgr) {
  ui->setupUi(this);

  // 设置日期范围（今天到未来60天）
  QDate currentDate = QDate::currentDate();
  ui->appointmentDateEdit->setMinimumDate(currentDate);
  ui->appointmentDateEdit->setMaximumDate(currentDate.addDays(60));
  ui->appointmentDateEdit->setDate(currentDate);

  // 加载科室信息
  loadDepartments();

  // 设置年龄控件为只读
  ui->ageSpinBox->setReadOnly(true);
  // 设置性别输入框为只读
  ui->genderInput->setReadOnly(true);

  // 连接信号槽
  connect(ui->departmentCombo, &QComboBox::currentTextChanged, this,
          &PatientDialog::loadExpertsByDepartment);
  connect(ui->expertCombo, &QComboBox::currentTextChanged, this,
          [this](const QString& expertName) {
            loadServiceTimesByDate(expertName, ui->appointmentDateEdit->date());
          });
  connect(ui->appointmentDateEdit, &QDateEdit::dateChanged, this,
          [this](const QDate& date) {
            QString expertName = ui->expertCombo->currentData().toString();
            if (!expertName.isEmpty()) {
              loadServiceTimesByDate(expertName, date);
            }
          });

  // 连接身份证和电话输入变化事件
  connect(ui->idInput, &QLineEdit::textChanged, this,
          &PatientDialog::on_idInput_textChanged);
  connect(ui->phoneInput, &QLineEdit::textChanged, this,
          &PatientDialog::on_phoneInput_textChanged);
}

PatientDialog::~PatientDialog() { delete ui; }

// 身份证输入变化处理
void PatientDialog::on_idInput_textChanged() {
  QString idNumber = ui->idInput->text().trimmed();

  // 实时验证身份证格式
  if (idNumber.length() == 18) {
    if (isValidIdNumber(idNumber)) {
      // 身份证有效，自动填充年龄和性别
      updateAgeAndGender(idNumber);
      ui->idInput->setStyleSheet("");  // 清除错误样式
    } else {
      // 身份证无效，显示错误样式
      ui->idInput->setStyleSheet("QLineEdit { border: 2px solid red; }");
      ui->ageSpinBox->setValue(0);
      ui->genderInput->setText("");  // 清除性别
    }
  } else if (idNumber.length() > 0) {
    // 长度不够，清除填充的信息但不显示错误
    ui->idInput->setStyleSheet("");
    ui->ageSpinBox->setValue(0);
    ui->genderInput->setText("");  // 清除性别
  } else {
    // 空输入，清除所有
    ui->idInput->setStyleSheet("");
    ui->ageSpinBox->setValue(0);
    ui->genderInput->setText("");  // 清除性别
  }
}

// 电话号码输入变化处理
void PatientDialog::on_phoneInput_textChanged() {
  QString phone = ui->phoneInput->text().trimmed();

  if (!phone.isEmpty()) {
    if (isValidPhoneNumber(phone)) {
      ui->phoneInput->setStyleSheet("");  // 清除错误样式
    } else {
      ui->phoneInput->setStyleSheet("QLineEdit { border: 2px solid red; }");
    }
  } else {
    ui->phoneInput->setStyleSheet("");
  }
}

// 验证身份证号是否合法
bool PatientDialog::isValidIdNumber(const QString& idNumber) {
  if (idNumber.length() != 18) {
    return false;
  }

  // 检查前17位是否为数字
  QRegularExpression re("^\\d{17}[\\dXx]$");
  if (!re.match(idNumber).hasMatch()) {
    return false;
  }

  // 验证出生日期
  QString birthDate = idNumber.mid(6, 8);
  QDate birth = QDate::fromString(birthDate, "yyyyMMdd");
  if (!birth.isValid()) {
    return false;
  }

  // 验证年龄合理性（0-150岁）
  QDate today = QDate::currentDate();
  int age = birth.daysTo(today) / 365;
  if (age < 0 || age > 150) {
    return false;
  }

  // 验证校验码
  const int weights[] = {7, 9, 10, 5, 8, 4, 2, 1, 6, 3, 7, 9, 10, 5, 8, 4, 2};
  const char checkCodes[] = {'1', '0', 'X', '9', '8', '7',
                             '6', '5', '4', '3', '2'};

  int sum = 0;
  for (int i = 0; i < 17; ++i) {
    sum += idNumber[i].digitValue() * weights[i];
  }

  char expectedCheck = checkCodes[sum % 11];
  char actualCheck = idNumber[17].toUpper().toLatin1();

  return expectedCheck == actualCheck;
}

// 验证电话号码是否合法
bool PatientDialog::isValidPhoneNumber(const QString& phone) {
  // 手机号码：1开头，11位数字
  QRegularExpression mobileRe("^1[3-9]\\d{9}$");

  // 固定电话：区号+号码，支持多种格式
  QRegularExpression landlineRe("^0\\d{2,3}-?\\d{7,8}$");

  // 400电话等特殊号码
  QRegularExpression specialRe("^400-?\\d{7}$");

  return mobileRe.match(phone).hasMatch() ||
         landlineRe.match(phone).hasMatch() ||
         specialRe.match(phone).hasMatch();
}

// 从身份证号获取性别
QString PatientDialog::getGenderFromId(const QString& idNumber) {
  if (idNumber.length() != 18) {
    return "";
  }
  QChar genderChar = idNumber.at(16);
  int genderCode = genderChar.digitValue();
  bool isMale = (genderCode % 2 == 1);
  return isMale ? "男" : "女";
}

// 从身份证号获取年龄
int PatientDialog::getAgeFromId(const QString& idNumber) {
  if (idNumber.length() != 18) {
    return 0;
  }

  QString birthDate = idNumber.mid(6, 8);
  QDate birth = QDate::fromString(birthDate, "yyyyMMdd");

  if (!birth.isValid()) {
    return 0;
  }

  QDate today = QDate::currentDate();
  int age = birth.daysTo(today) / 365;

  return age;
}

// 更新年龄和性别
void PatientDialog::updateAgeAndGender(const QString& idNumber) {
  QString gender = getGenderFromId(idNumber);
  int age = getAgeFromId(idNumber);

  ui->genderInput->setText(gender);  // 使用输入框显示性别
  ui->ageSpinBox->setValue(age);

  qDebug() << "自动填充 - 性别:" << gender << ", 年龄:" << age;
}

// 加载科室信息
void PatientDialog::loadDepartments() {
  QSet<QString> departments;
  for (const auto& expert : expertManager->experts) {
    departments.insert(expert.subject);
  }
  ui->departmentCombo->clear();
  for (const auto& dept : departments) {
    ui->departmentCombo->addItem(dept);
  }
}

// 加载对应科室的专家
void PatientDialog::loadExpertsByDepartment(const QString& department) {
  ui->expertCombo->clear();

  for (const auto& expert : expertManager->experts) {
    if (expert.subject == department) {
      // 检查专家是否有可用预约日期
      bool hasAvailableDates = false;
      QDate currentDate = QDate::currentDate();

      // 检查未来60天
      for (int i = 0; i < 60; i++) {
        QDate date = currentDate.addDays(i);
        if (expert.isAvailableOnDate(date)) {
          hasAvailableDates = true;
          break;
        }
      }

      // 只添加有可用日期的专家，并显示职称
      if (hasAvailableDates) {
        // 组合显示格式：专家姓名（职称）
        QString displayText =
            QString("%1（%2）").arg(expert.name).arg(expert.title);

        // 将专家名称存储为项的用户数据，以便后续使用
        QVariant userData = expert.name;

        ui->expertCombo->addItem(displayText, userData);
      }
    }
  }

  // 如果选择了专家，更新可用日期
  if (ui->expertCombo->count() > 0) {
    updateAvailableDates();
  } else {
    ui->serviceTimeCombo->clear();
  }
}

// 更新可用日期
void PatientDialog::updateAvailableDates() {
  QString expertName = ui->expertCombo->currentData().toString();
  if (expertName.isEmpty()) return;

  // 查找专家
  Expert* selectedExpert = nullptr;
  for (auto& expert : expertManager->experts) {
    if (expert.name == expertName) {
      selectedExpert = &expert;
      break;
    }
  }

  if (!selectedExpert) return;

  // 保存当前选择的日期
  QDate currentSelectedDate = ui->appointmentDateEdit->date();

  // 禁用所有日期的事件处理器
  ui->appointmentDateEdit->disconnect();

  // 更新日期选择控件
  QDate today = QDate::currentDate();

  // 创建自定义日历模型
  QCalendarWidget* calendar = ui->appointmentDateEdit->calendarWidget();
  if (calendar) {
    // 清除所有日期格式
    calendar->setDateTextFormat(QDate(), QTextCharFormat());

    // 设置特殊出诊日期格式（浅绿色背景，深绿色文字）
    QTextCharFormat specialScheduleFormat;
    specialScheduleFormat.setBackground(QColor(144, 238, 144));  // 浅绿色背景
    specialScheduleFormat.setForeground(QColor(0, 100, 0));  // 深绿色文字

    // 设置特殊停诊日期格式（浅红色背景，深红色文字）
    QTextCharFormat closedFormat;
    closedFormat.setBackground(QColor(255, 182, 193));  // 浅红色背景
    closedFormat.setForeground(QColor(139, 0, 0));      // 深红色文字

    // 设置常规出诊日期格式（浅蓝色背景）
    QTextCharFormat regularFormat;
    regularFormat.setBackground(QColor(173, 216, 230));  // 浅蓝色背景

    // 设置不可用日期格式（灰色）
    QTextCharFormat unavailableFormat;
    unavailableFormat.setForeground(QColor(180, 180, 180));  // 灰色

    // 检查未来60天
    for (int i = 0; i < 60; i++) {
      QDate date = today.addDays(i);

      if (selectedExpert->scheduleDates.contains(date)) {
        // 特殊出诊日
        calendar->setDateTextFormat(date, specialScheduleFormat);
      } else if (selectedExpert->closedDates.contains(date)) {
        // 特殊停诊日
        calendar->setDateTextFormat(date, closedFormat);
      } else {
        // 检查是否为常规出诊日
        QString dayOfWeek = Expert::getDayOfWeekString(date);
        bool isRegularScheduled = false;

        for (const QString& timeSlot : selectedExpert->serviceTimes) {
          if (timeSlot.startsWith(dayOfWeek)) {
            isRegularScheduled = true;
            break;
          }
        }

        if (isRegularScheduled) {
          // 常规出诊日
          calendar->setDateTextFormat(date, regularFormat);
        } else {
          // 不可用日期
          calendar->setDateTextFormat(date, unavailableFormat);
        }
      }
    }
  }

  // 如果当前选择的日期在有效范围内且专家可预约，则保持选择
  if (currentSelectedDate >= today &&
      currentSelectedDate <= today.addDays(60) &&
      selectedExpert->isAvailableOnDate(currentSelectedDate)) {
    ui->appointmentDateEdit->setDate(currentSelectedDate);
  } else {
    // 否则，找到第一个可用日期
    QDate firstAvailableDate = today;
    bool foundAvailable = false;

    for (int i = 0; i < 60; i++) {
      QDate date = today.addDays(i);
      if (selectedExpert->isAvailableOnDate(date)) {
        firstAvailableDate = date;
        foundAvailable = true;
        break;
      }
    }

    if (foundAvailable) {
      ui->appointmentDateEdit->setDate(firstAvailableDate);
    }
  }

  // 重新连接日期变更事件
  connect(ui->appointmentDateEdit, &QDateEdit::dateChanged, this,
          &PatientDialog::on_appointmentDateEdit_dateChanged);

  // 加载该日期的时间段
  loadServiceTimesByDate(expertName, ui->appointmentDateEdit->date());
}

// 日期变化处理
void PatientDialog::on_appointmentDateEdit_dateChanged(const QDate& date) {
  QString expertName = ui->expertCombo->currentData().toString();
  if (!expertName.isEmpty()) {
    loadServiceTimesByDate(expertName, date);
  }
}

// 加载指定日期的时间段，支持具体日期格式
void PatientDialog::loadServiceTimesByDate(const QString& expertName,
                                           const QDate& date) {
  ui->serviceTimeCombo->clear();
  if (expertName.isEmpty() || !date.isValid()) return;

  // 查找专家
  Expert* selectedExpert = nullptr;
  for (auto& expert : expertManager->experts) {
    if (expert.name == expertName) {
      selectedExpert = &expert;
      break;
    }
  }

  if (!selectedExpert) return;

  // 检查该日期是否可预约
  if (!selectedExpert->isAvailableOnDate(date)) {
    QMessageBox::warning(this, "提示",
                         QString("专家 %1 在 %2 没有出诊安排，请选择其他日期！")
                             .arg(expertName)
                             .arg(date.toString("yyyy年MM月dd日")));
    return;
  }

  // 获取该日期对应的星期几
  QString dayOfWeek = Expert::getDayOfWeekString(date);

  // 加载匹配的时间段
  for (const QString& timeSlot : selectedExpert->serviceTimes) {
    bool isMatchingSlot = false;

    // 检查是否为特殊出诊日
    if (selectedExpert->scheduleDates.contains(date)) {
      // 特殊出诊日：加载以具体日期开头的时间段
      QString dateStr = date.toString("MM-dd：");
      if (timeSlot.startsWith(dateStr)) {
        isMatchingSlot = true;
      }
    } else {
      // 常规出诊日：加载以星期几开头的时间段
      if (timeSlot.startsWith(dayOfWeek)) {
        isMatchingSlot = true;
      }
    }

    if (isMatchingSlot) {
      // 统计该时间段的当前预约数
      int count = 0;
      for (const Appointment& a : appointmentManager->getAllAppointments()) {
        if (a.expertName == expertName && a.serviceTime == timeSlot &&
            a.appointmentDate == date) {
          ++count;
        }
      }

      int capacity = selectedExpert->getTimeSlotCapacity(timeSlot);
      if (count < capacity) {
        QString displayText =
            QString("%1 (已预约:%2/%3)").arg(timeSlot).arg(count).arg(capacity);
        ui->serviceTimeCombo->addItem(displayText);
      } else {
        QString displayText =
            QString("%1 (已满:%2/%3)").arg(timeSlot).arg(count).arg(capacity);
        ui->serviceTimeCombo->addItem(displayText);
      }
    }
  }
}

// 修改科室选择变化处理
void PatientDialog::on_departmentCombo_currentIndexChanged(int index) {
  if (index >= 0) {
    QString department = ui->departmentCombo->currentText();
    loadExpertsByDepartment(department);
  }
}

// 修改提交按钮处理逻辑，增加日期处理
void PatientDialog::on_submitButton_clicked() {
  // 验证身份证
  QString idNumber = ui->idInput->text().trimmed();
  if (idNumber.isEmpty()) {
    QMessageBox::warning(this, "提示", "请输入身份证号！");
    return;
  }

  if (!isValidIdNumber(idNumber)) {
    QMessageBox::warning(this, "提示", "请输入有效的身份证号！");
    ui->idInput->setFocus();
    return;
  }

  // 验证电话号码
  QString phone = ui->phoneInput->text().trimmed();
  if (phone.isEmpty()) {
    QMessageBox::warning(this, "提示", "请输入电话号码！");
    return;
  }

  if (!isValidPhoneNumber(phone)) {
    QMessageBox::warning(
        this, "提示",
        "请输入有效的电话号码！\n支持格式：\n• 手机号：13812345678\n• "
        "固话：010-12345678\n• 400电话：400-1234567");
    ui->phoneInput->setFocus();
    return;
  }

  if (appointmentManager) {
    for (const Appointment& a : appointmentManager->getAllAppointments()) {
      if (a.idNumber == idNumber) {
        QMessageBox::warning(this, "提示",
                              "该身份证号已有预约记录，不能重复预约！");
        return;
        
      }
    }
  }

  // 收集输入内容
  Appointment appointment;
  appointment.patientName = ui->nameInput->text().trimmed();
  appointment.idNumber = idNumber;
  appointment.gender = ui->genderInput->text();  // 使用输入框获取性别
  appointment.age = ui->ageSpinBox->value();
  appointment.phone = phone;
  appointment.expertName =
      ui->expertCombo->currentData().toString();  // 使用currentData获取专家名称
  appointment.expertSubject = ui->departmentCombo->currentText();

  // 从服务时间显示中提取原始服务时间
  QString displayTime = ui->serviceTimeCombo->currentText();
  QString serviceTime =
      displayTime.split(" (")[0];  // 只保留时间部分，去掉容量信息
  appointment.serviceTime = serviceTime;

  appointment.description = ui->symptomsText->toPlainText().trimmed();
  appointment.appointmentDate = ui->appointmentDateEdit->date();

  // 验证其他必填项
  if (appointment.patientName.isEmpty()) {
    QMessageBox::warning(this, "提示", "请输入患者姓名！");
    return;
  }

  if (appointment.expertName.isEmpty()) {
    QMessageBox::warning(this, "提示", "请选择专家！");
    return;
  }

  if (appointment.serviceTime.isEmpty()) {
    QMessageBox::warning(this, "提示", "请选择服务时间！");
    return;
  }

  // 查找专家对象
  Expert* selectedExpert = nullptr;
  for (auto& expert : expertManager->experts) {
    if (expert.name == appointment.expertName) {
      selectedExpert = &expert;
      break;
    }
  }

  if (!selectedExpert) {
    QMessageBox::warning(this, "错误", "找不到选择的专家信息！");
    return;
  }

  // 验证专家在该日期是否出诊
  if (!selectedExpert->isAvailableOnDate(appointment.appointmentDate)) {
    QMessageBox::warning(
        this, "错误",
        QString("专家 %1 在 %2 没有出诊安排！")
            .arg(appointment.expertName)
            .arg(appointment.appointmentDate.toString("yyyy年MM月dd日")));
    return;
  }

  // 统计该专家该日期该时间段已有预约人数
  int count = 0;
  for (const Appointment& a : appointmentManager->getAllAppointments()) {
    if (a.expertName == appointment.expertName &&
        a.serviceTime == appointment.serviceTime &&
        a.appointmentDate == appointment.appointmentDate) {
      ++count;
    }
  }

  // 使用专家设置的容量
  int maxCapacity =
      selectedExpert->getTimeSlotCapacity(appointment.serviceTime);
  if (count >= maxCapacity) {
    QMessageBox::warning(this, "提示",
                         QString("该时间段预约已满！\n当前预约：%"
                                 "1人\n最大容量：%2人\n请选择其他时间段！")
                             .arg(count)
                             .arg(maxCapacity));
    return;
  }

  appointment.queueNumber = count + 1;

  if (appointmentManager) {
    if (appointmentManager->addAppointment(appointment)) {
      QMessageBox::information(
          this, "成功",
          QString(
              "预约成功！\n患者：%1\n专家：%2\n日期：%3\n时间：%4\n排队号：%5")
              .arg(appointment.patientName)
              .arg(appointment.expertName)
              .arg(appointment.appointmentDate.toString("yyyy年MM月dd日"))
              .arg(appointment.serviceTime)
              .arg(appointment.queueNumber));
      on_cancelButton_clicked();
    } else {
      QMessageBox::warning(this, "失败", "预约失败，请稍后重试！");
    }
  } else {
    QMessageBox::warning(this, "错误", "预约管理器未初始化！");
  }
}

// 取消按钮点击事件处理
void PatientDialog::on_cancelButton_clicked() {
  // 重置表单内容
  ui->nameInput->clear();
  ui->idInput->clear();
  ui->phoneInput->clear();
  ui->symptomsText->clear();
  ui->ageSpinBox->setValue(0);
  ui->genderInput->clear();  // 清除性别输入框

  // 清除样式
  ui->idInput->setStyleSheet("");
  ui->phoneInput->setStyleSheet("");

  // 重新启用年龄和性别控件的禁用状态
  ui->ageSpinBox->setReadOnly(true);
  ui->genderInput->setReadOnly(true);  // 性别输入框始终只读

  accept();
}

void PatientDialog::on_expertCombo_currentIndexChanged(int index) {
  if (index >= 0) {
    QString expertName = ui->expertCombo->currentData().toString();
    if (!expertName.isEmpty()) {
      // 更新日历显示专家的可用日期
      updateAvailableDates();
      // 加载该日期的时间段
      loadServiceTimesByDate(expertName, ui->appointmentDateEdit->date());
    } else {
      ui->serviceTimeCombo->clear();
    }
  }
}
