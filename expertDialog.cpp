#include "expertDialog.h"

#include <QColor>
#include <QDate>
#include <QDebug>
#include <QHeaderView>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QRegularExpression>
#include <QStandardItemModel>
#include <QTextCharFormat>
#include <QTime>

#include "ui_expertDialog.h"

ExpertDialog::ExpertDialog(Expert* expert, AppointmentManager* appointmentMgr,
                           QWidget* parent)
    : QDialog(parent),
      ui(new Ui::ExpertDialog),
      currentExpert(expert),
      appointmentManager(appointmentMgr),
      appointmentModel(new QStandardItemModel(this)) {
  ui->setupUi(this);
  setupUI();
  loadExpertInfo();
  setupAppointmentTable();
  loadAppointments();
  loadServiceTimes();
  loadScheduleDates();
}

ExpertDialog::~ExpertDialog() { delete ui; }

// 设置界面样式和属性
void ExpertDialog::setupUI() {
  // 设置窗口标题
  setWindowTitle(QString("专家工作站 - %1")
                     .arg(currentExpert ? currentExpert->name : "未知专家"));

  // 设置表单为只读（除了密码和服务时间）
  setFormReadOnly(true);

  ui->ageSpinBox->setRange(15, 120);
  ui->calendar->setGridVisible(true);
  ui->calendar->setNavigationBarVisible(true);

  // 移除性别下拉框选项填充

  // 设置科室输入框为只读
  ui->departmentInput->setReadOnly(true);

  connect(ui->calendar, &QCalendarWidget::clicked, this,
          &ExpertDialog::on_calendar_clicked);

  ui->tabWidget->setCurrentIndex(0);
}

void ExpertDialog::setFormReadOnly(bool readOnly) {
  // 基本信息设为只读
  ui->idInput->setReadOnly(readOnly);
  ui->nameInput->setReadOnly(readOnly);
  ui->genderInput->setReadOnly(readOnly);
  ui->ageSpinBox->setReadOnly(readOnly);
  ui->titleInput->setReadOnly(readOnly);
  ui->departmentInput->setReadOnly(readOnly);
  ui->timeInput->setReadOnly(readOnly);
}

void ExpertDialog::loadExpertInfo() {
  if (!currentExpert) return;

  ui->idInput->setText(currentExpert->id);
  ui->nameInput->setText(currentExpert->name);
  ui->genderInput->setText(currentExpert->gender); 
  ui->ageSpinBox->setValue(currentExpert->age);
  ui->titleInput->setText(currentExpert->title);
  ui->departmentInput->setText(currentExpert->subject);

  // 显示服务时间（格式化显示）
  QString timeDisplay = currentExpert->serviceTimes.join(", ");
  ui->timeInput->setText(timeDisplay);
}

void ExpertDialog::setupAppointmentTable() {
  // 设置表格模型
  appointmentModel->setHorizontalHeaderLabels({"患者姓名", "性别", "年龄",
                                               "联系电话", "预约时间",
                                               "症状描述", "排队号", "状态"});

  ui->appointmentTable->setModel(appointmentModel);

  // 设置表格属性
  ui->appointmentTable->setAlternatingRowColors(true);
  ui->appointmentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->appointmentTable->horizontalHeader()->setStretchLastSection(true);
  ui->appointmentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

  // 设置列宽
  ui->appointmentTable->setColumnWidth(0, 100);  // 患者姓名
  ui->appointmentTable->setColumnWidth(1, 60);   // 性别
  ui->appointmentTable->setColumnWidth(2, 60);   // 年龄
  ui->appointmentTable->setColumnWidth(3, 120);  // 联系电话
  ui->appointmentTable->setColumnWidth(4, 150);  // 预约时间
  ui->appointmentTable->setColumnWidth(5, 200);  // 症状描述
  ui->appointmentTable->setColumnWidth(6, 80);   // 排队号
  ui->appointmentTable->setColumnWidth(7, 80);   // 状态
}

void ExpertDialog::loadAppointments() { updateAppointmentTable(); }

void ExpertDialog::updateAppointmentTable() {
  if (!currentExpert || !appointmentManager) return;

  appointmentModel->clear();
  appointmentModel->setHorizontalHeaderLabels({"患者姓名", "性别", "年龄",
                                               "联系电话", "预约时间",
                                               "症状描述", "排队号", "状态"});

  // 获取该专家的所有预约
  auto appointments = appointmentManager->getAllAppointments();
  int row = 0;

  for (const auto& appointment : appointments) {
    if (appointment.expertName == currentExpert->name) {
      appointmentModel->setItem(row, 0,
                                new QStandardItem(appointment.patientName));
      appointmentModel->setItem(row, 1, new QStandardItem(appointment.gender));
      appointmentModel->setItem(
          row, 2, new QStandardItem(QString::number(appointment.age)));
      appointmentModel->setItem(row, 3, new QStandardItem(appointment.phone));
      appointmentModel->setItem(row, 4,
                                new QStandardItem(appointment.serviceTime));
      appointmentModel->setItem(row, 5,
                                new QStandardItem(appointment.description));
      appointmentModel->setItem(
          row, 6, new QStandardItem(QString::number(appointment.queueNumber)));
      appointmentModel->setItem(row, 7, new QStandardItem("待诊"));
      row++;
    }
  }

  qDebug() << QString("加载了 %1 条预约记录").arg(row);
}

void ExpertDialog::loadServiceTimes() {
  if (!currentExpert) return;

  ui->serviceTimeList->clear();
  for (const QString& timeSlot : currentExpert->serviceTimes) {
    int capacity = currentExpert->getTimeSlotCapacity(timeSlot);

    // 统计该时间段的当前预约数
    int currentAppointments = 0;
    if (appointmentManager) {
      for (const auto& appointment : appointmentManager->getAllAppointments()) {
        if (appointment.expertName == currentExpert->name &&
            appointment.serviceTime == timeSlot) {
          currentAppointments++;
        }
      }
    }

    QString displayText = QString("%1 (预约:%2/%3人)")
                              .arg(timeSlot)
                              .arg(currentAppointments)
                              .arg(capacity);
    ui->serviceTimeList->addItem(displayText);
  }
}

void ExpertDialog::loadScheduleDates() {
  if (!currentExpert) return;

  // 清除原有的格式
  updateCalendarDisplay();
}

// 更新日历显示
void ExpertDialog::updateCalendarDisplay() {
  if (!currentExpert) return;

  // 先清除所有日期格式
  ui->calendar->setDateTextFormat(QDate(), QTextCharFormat());

  // 高亮显示出诊日期
  QTextCharFormat outFormat;
  outFormat.setBackground(QColor(144, 238, 144));  // 浅绿色背景
  outFormat.setForeground(QColor(0, 100, 0));      // 深绿色文字

  for (const QDate& date : currentExpert->scheduleDates) {
    ui->calendar->setDateTextFormat(date, outFormat);
  }

  // 高亮显示停诊日期
  QTextCharFormat closedFormat;
  closedFormat.setBackground(QColor(255, 128, 128));  // 浅红色背景
  closedFormat.setForeground(QColor(200, 0, 0));      // 深红色文字
  
  for (const QDate& date : currentExpert->closedDates) {
    ui->calendar->setDateTextFormat(date, closedFormat);
  }

  // 标记常规出诊日期
  QDate startDate = QDate::currentDate();
  QDate endDate = startDate.addDays(60);  // 显示未来60天

  QTextCharFormat regularFormat;
  regularFormat.setBackground(QColor(220, 240, 255));  // 浅蓝色背景

  for (QDate date = startDate; date <= endDate; date = date.addDays(1)) {
    QString dayOfWeek = Expert::getDayOfWeekString(date);

    // 如果已经是特殊出诊或停诊日，则跳过
    if (currentExpert->scheduleDates.contains(date) ||
        currentExpert->closedDates.contains(date))
      continue;

    // 检查该星期几是否有安排
    bool hasSchedule = false;
    for (const QString& timeSlot : currentExpert->serviceTimes) {
      if (timeSlot.startsWith(dayOfWeek)) {
        hasSchedule = true;
        break;
      }
    }

    if (hasSchedule) {
      ui->calendar->setDateTextFormat(date, regularFormat);
    }
  }
}

// 修改日历点击事件处理
void ExpertDialog::on_calendar_clicked(const QDate& date) {
  if (!currentExpert) return;

  // 检查是否已经安排出诊
  bool isSpecialScheduled = currentExpert->scheduleDates.contains(date);
  bool isSpecialClosed = currentExpert->closedDates.contains(date);

  // 检查是否为常规出诊日
  QString dayOfWeek = Expert::getDayOfWeekString(date);
  bool isRegularScheduled = false;

  for (const QString& timeSlot : currentExpert->serviceTimes) {
    if (timeSlot.startsWith(dayOfWeek)) {
      isRegularScheduled = true;
      break;
    }
  }

  QString message;
  if (isSpecialScheduled) {
    message =
        QString(
            "日期：%1\n状态：特殊出诊日\n\n您可以：\n- 取消该日期的出诊安排")
            .arg(date.toString("yyyy年MM月dd日"));
  } else if (isSpecialClosed) {
    message = QString("日期：%1\n状态：特殊停诊日\n\n您可以：\n- 取消停诊安排")
                  .arg(date.toString("yyyy年MM月dd日"));
  } else if (isRegularScheduled) {
    message =
        QString("日期：%1\n状态：常规出诊日 (%2)\n\n您可以：\n- 设为特殊停诊日")
            .arg(date.toString("yyyy年MM月dd日"))
            .arg(dayOfWeek);
  } else {
    message =
        QString("日期：%1\n状态：非出诊日\n\n您可以：\n- 添加为特殊出诊日")
            .arg(date.toString("yyyy年MM月dd日"));
  }

  ui->calendarLegendLabel->setText(message);
}

// 修改添加出诊按钮逻辑，确保时间段容量key使用具体日期格式
void ExpertDialog::on_addScheduleBtn_clicked() {
  if (!currentExpert) return;

  QDate selectedDate = ui->calendar->selectedDate();

  // 检查日期有效性
  if (selectedDate < QDate::currentDate()) {
    QMessageBox::warning(this, "错误", "不能为过去的日期安排出诊！");
    return;
  }

  // 检查是否已经安排出诊
  if (currentExpert->scheduleDates.contains(selectedDate)) {
    QMessageBox::information(this, "提示", "该日期已经安排为出诊日！");
    return;
  }

  // 检查时间段输入
  QString timeSlot = ui->timeSlotInput->text().trimmed();
  QRegularExpression re("^\\d{2}:\\d{2}-\\d{2}:\\d{2}$");
  if (!re.match(timeSlot).hasMatch()) {
    QMessageBox::warning(this, "格式错误",
                         "请按正确格式输入时间段: 09:00-12:00");
    return;
  }

  // 检查人数上限输入
  int capacity = ui->capacityInput->value();

  // 添加出诊安排，使用具体日期格式
  currentExpert->scheduleDates.append(selectedDate);
  QString fullTimeSlot =
      QString("%1：%2").arg(selectedDate.toString("MM-dd")).arg(timeSlot);
  currentExpert->serviceTimes.append(fullTimeSlot);
  currentExpert->setTimeSlotCapacity(fullTimeSlot, capacity);  // 使用完整的key

  // 更新日历显示
  updateCalendarDisplay();

  QMessageBox::information(
      this, "成功",
      QString("已成功添加 %1 的出诊安排！\n时间段: %2\n人数上限: %3")
          .arg(selectedDate.toString("yyyy年MM月dd日"))
          .arg(timeSlot)
          .arg(capacity));
}

// 修改取消出诊按钮逻辑，移除对应的时间段
void ExpertDialog::on_cancelScheduleBtn_clicked() {
  if (!currentExpert) return;

  QDate selectedDate = ui->calendar->selectedDate();

  // 检查是否为特殊出诊日
  if (currentExpert->scheduleDates.contains(selectedDate)) {
    // 移除出诊安排
    currentExpert->scheduleDates.removeAll(selectedDate);

    // 移除对应的时间段（以MM-dd开头）
    QString datePrefix = selectedDate.toString("MM-dd：");
    QStringList timeSlotsToRemove;
    for (const QString& timeSlot : currentExpert->serviceTimes) {
      if (timeSlot.startsWith(datePrefix)) {
        timeSlotsToRemove.append(timeSlot);
      }
    }
    for (const QString& timeSlot : timeSlotsToRemove) {
      currentExpert->serviceTimes.removeOne(timeSlot);
      currentExpert->removeTimeSlotCapacity(timeSlot);
    }
  }
  // 检查是否为特殊停诊日
  else if (currentExpert->closedDates.contains(selectedDate)) {
    // 移除停诊安排
    currentExpert->closedDates.removeAll(selectedDate);
  } else {
    QMessageBox::information(this, "提示", "该日期不是特殊出诊日或停诊日！");
    return;
  }

  // 更新日历显示
  updateCalendarDisplay();

  QMessageBox::information(this, "成功",
                           QString("已重置 %1 的出诊状态！")
                               .arg(selectedDate.toString("yyyy年MM月dd日")));
}

// 设置停诊日按钮
void ExpertDialog::on_setClosedBtn_clicked() {
  if (!currentExpert) return;

  QDate selectedDate = ui->calendar->selectedDate();

  // 检查日期有效性
  if (selectedDate < QDate::currentDate()) {
    QMessageBox::warning(this, "错误", "不能为过去的日期设置停诊！");
    return;
  }

  // 检查是否已经是停诊日
  if (currentExpert->closedDates.contains(selectedDate)) {
    QMessageBox::information(this, "提示", "该日期已经设置为停诊日！");
    return;
  }

  // 移除该日期的服务时间段（以MM-dd开头）
  QString datePrefix = selectedDate.toString("MM-dd：");
  QStringList timeSlotsToRemove;
  for (const QString& timeSlot : currentExpert->serviceTimes) {
    if (timeSlot.startsWith(datePrefix)) {
      timeSlotsToRemove.append(timeSlot);
    }
  }

  for (const QString& timeSlot : timeSlotsToRemove) {
    currentExpert->serviceTimes.removeOne(timeSlot);
    currentExpert->removeTimeSlotCapacity(timeSlot);
  }

  // 添加停诊安排
  currentExpert->closedDates.append(selectedDate);

  // 更新日历显示
  updateCalendarDisplay();

  QMessageBox::information(this, "成功",
                           QString("已成功设置 %1 为停诊日！")
                               .arg(selectedDate.toString("yyyy年MM月dd日")));
}

void ExpertDialog::saveExpertInfo() {
  qDebug() << "专家信息已保存：" << currentExpert->name;
}

void ExpertDialog::on_saveButton_clicked() {
  saveExpertInfo();
  QMessageBox::information(this, "成功", "信息保存成功！");
}

void ExpertDialog::on_closeButton_clicked() {
  // 提示是否保存修改
  int ret = QMessageBox::question(
      this, "确认", "是否保存修改并关闭？",
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

  switch (ret) {
    case QMessageBox::Save:
      saveExpertInfo();
      accept();
      break;
    case QMessageBox::Discard:
      reject();
      break;
    case QMessageBox::Cancel:
    default:
      break;
  }
}

bool ExpertDialog::hasTimeConflict(const QString& newTimeSlot) {
  if (!currentExpert) return false;

  // 解析新时间段
  QStringList parts = newTimeSlot.split("：");
  if (parts.size() != 2) return false;

  QString newWeekday = parts[0].trimmed();
  QString newTimeRange = parts[1].trimmed();

  QStringList newTimeParts = newTimeRange.split("-");
  if (newTimeParts.size() != 2) return false;

  QTime newStart = QTime::fromString(newTimeParts[0].trimmed(), "HH:mm");
  QTime newEnd = QTime::fromString(newTimeParts[1].trimmed(), "HH:mm");

  if (!newStart.isValid() || !newEnd.isValid()) return false;

  // 查找同一天的冲突时间段
  QStringList conflictingSlots;
  QTime mergedStart = newStart;
  QTime mergedEnd = newEnd;
  int maxCapacity = currentExpert->getTimeSlotCapacity(newTimeSlot);

  for (const QString& existingSlot : currentExpert->serviceTimes) {
    QStringList existingParts = existingSlot.split("：");
    if (existingParts.size() != 2) continue;

    QString existingWeekday = existingParts[0].trimmed();

    // 如果不是同一天，跳过
    if (newWeekday != existingWeekday) continue;

    QString existingTimeRange = existingParts[1].trimmed();
    QStringList existingTimeParts = existingTimeRange.split("-");
    if (existingTimeParts.size() != 2) continue;

    QTime existingStart =
        QTime::fromString(existingTimeParts[0].trimmed(), "HH:mm");
    QTime existingEnd =
        QTime::fromString(existingTimeParts[1].trimmed(), "HH:mm");

    if (!existingStart.isValid() || !existingEnd.isValid()) continue;

    // 检查是否有时间重叠
    if (!(newEnd <= existingStart || newStart >= existingEnd)) {
      // 有重叠，记录冲突的时间段
      conflictingSlots.append(existingSlot);

      // 扩展合并的时间范围
      if (existingStart < mergedStart) {
        mergedStart = existingStart;
      }
      if (existingEnd > mergedEnd) {
        mergedEnd = existingEnd;
      }

      // 取最大容量
      int existingCapacity = currentExpert->getTimeSlotCapacity(existingSlot);
      if (existingCapacity > maxCapacity) {
        maxCapacity = existingCapacity;
      }
    }
  }

  // 如果有冲突，进行合并
  if (!conflictingSlots.isEmpty()) {
    QString mergedSlot = QString("%1：%2-%3")
                             .arg(newWeekday)
                             .arg(mergedStart.toString("HH:mm"))
                             .arg(mergedEnd.toString("HH:mm"));

    // 询问用户是否要合并
    QString conflictInfo = conflictingSlots.join(", ");
    int ret = QMessageBox::question(this, "时间重叠",
                                    QString("检测到时间重叠！\n"
                                            "新时间段：%1\n"
                                            "冲突时间段：%2\n\n"
                                            "是否合并为：%3 (容量:%4人)？")
                                        .arg(newTimeSlot)
                                        .arg(conflictInfo)
                                        .arg(mergedSlot)
                                        .arg(maxCapacity),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
      // 执行合并
      mergeTimeSlots(conflictingSlots, newTimeSlot, mergedSlot, maxCapacity);
      return true;  // 返回true表示已处理冲突
    } else {
      return true;  // 用户拒绝合并，阻止添加
    }
  }

  return false;  // 无冲突
}

void ExpertDialog::on_changePasswordBtn_clicked() {
  bool ok;
  QString oldPassword = QInputDialog::getText(
      this, "修改密码", "请输入当前密码：", QLineEdit::Password, "", &ok);

  if (!ok || oldPassword.isEmpty()) {
    return;
  }

  // 验证旧密码（这里简化处理，实际应该加密验证）
  if (oldPassword != currentExpert->password) {
    QMessageBox::warning(this, "错误", "当前密码不正确！");
    return;
  }

  QString newPassword = QInputDialog::getText(
      this, "修改密码", "请输入新密码：", QLineEdit::Password, "", &ok);

  if (!ok || newPassword.isEmpty()) {
    return;
  }

  // 密码强度验证
  if (newPassword.length() < 6) {
    QMessageBox::warning(this, "密码太简单", "密码长度至少6位！");
    return;
  }

  QString confirmPassword = QInputDialog::getText(
      this, "修改密码", "请确认新密码：", QLineEdit::Password, "", &ok);

  if (!ok || confirmPassword != newPassword) {
    QMessageBox::warning(this, "错误", "两次输入的密码不一致！");
    return;
  }

  // 更新密码
  currentExpert->password = newPassword;

  QMessageBox::information(this, "成功", "密码修改成功！");
  qDebug() << "专家" << currentExpert->name << "修改了密码";
}

void ExpertDialog::updateServiceTimeDisplay() {
  if (!currentExpert) return;

  QStringList displayList;
  for (const QString& timeSlot : currentExpert->serviceTimes) {
    int capacity = currentExpert->getTimeSlotCapacity(timeSlot);
    displayList << QString("%1(容量:%2)").arg(timeSlot).arg(capacity);
  }
  ui->timeInput->setText(displayList.join(", "));
}

// 双击修改容量
void ExpertDialog::on_serviceTimeList_itemDoubleClicked(QListWidgetItem* item) {
  Q_UNUSED(item)                // 避免未使用参数的警告
  on_setCapacityBtn_clicked();  // 双击时直接调用设置容量功能
}

void ExpertDialog::mergeTimeSlots(const QStringList& conflictingSlots,
                                  const QString& newTimeSlot,
                                  const QString& mergedSlot, int maxCapacity) {
  if (!currentExpert) return;

  // 统计所有被合并时间段的预约数量
  int totalAppointments = 0;
  QStringList affectedSlots = conflictingSlots;
  // 注意：不要将newTimeSlot添加到affectedSlots中，因为它还没有预约

  if (appointmentManager) {
    for (const QString& slot : conflictingSlots) {
      auto expertAppointments =
          appointmentManager->getAppointmentsByExpert(currentExpert->name);
      for (const auto& appointment : expertAppointments) {
        if (appointment.serviceTime == slot) {
          totalAppointments++;
        }
      }
    }
  }

  // 检查合并后的容量是否足够
  if (totalAppointments > maxCapacity) {
    QMessageBox::warning(
        this, "合并失败",
        QString("合并后的时间段已有 %1 个预约，但最大容量只有 %2 人！\n"
                "请先调整预约或增加容量。")
            .arg(totalAppointments)
            .arg(maxCapacity));
    return;
  }

  // 批量更新预约的服务时间
  if (appointmentManager) {
    for (const QString& slot : conflictingSlots) {
      appointmentManager->updateServiceTimeForExpert(currentExpert->name, slot,
                                                     mergedSlot);
    }
  }

  // 移除所有冲突的时间段
  for (const QString& slot : conflictingSlots) {
    currentExpert->serviceTimes.removeAll(slot);
    currentExpert->removeTimeSlotCapacity(slot);
    qDebug() << "移除时间段：" << slot;
  }

  // 添加合并后的时间段
  currentExpert->serviceTimes.append(mergedSlot);
  currentExpert->setTimeSlotCapacity(mergedSlot, maxCapacity);

  // 更新显示
  loadServiceTimes();
  updateServiceTimeDisplay();
  updateAppointmentTable();

  QMessageBox::information(this, "合并成功",
                           QString("时间段合并成功！\n"
                                   "合并后时间段：%1\n"
                                   "容量：%2人\n"
                                   "受影响的预约：%3个")
                               .arg(mergedSlot)
                               .arg(maxCapacity)
                               .arg(totalAppointments));

  qDebug() << "时间段合并完成：" << mergedSlot << "，容量：" << maxCapacity;
}

// 设置时间段容量
void ExpertDialog::on_setCapacityBtn_clicked() {
  if (!currentExpert) return;

  QListWidgetItem* currentItem = ui->serviceTimeList->currentItem();
  if (!currentItem) {
    QMessageBox::warning(this, "提示", "请先选择要设置容量的时间段！");
    return;
  }

  QString displayText = currentItem->text();
  QString timeSlot = displayText.split(" (")[0];  // 提取时间段部分

  int currentCapacity = currentExpert->getTimeSlotCapacity(timeSlot);

  // 显示输入对话框让用户输入新容量
  bool ok;
  int newCapacity = QInputDialog::getInt(
      this, "设置容量", QString("请为时间段 %1 设置预约容量:").arg(timeSlot),
      currentCapacity, 1, 100, 1, &ok);

  if (ok) {
    // 检查新容量是否小于当前预约数
    int appointmentCount = 0;
    if (appointmentManager) {
      for (const auto& appointment : appointmentManager->getAllAppointments()) {
        if (appointment.expertName == currentExpert->name &&
            appointment.serviceTime == timeSlot) {
          appointmentCount++;
        }
      }
    }

    if (newCapacity < appointmentCount) {
      QMessageBox::warning(
          this, "容量不足",
          QString("该时间段已有 %1 个预约，不能将容量设置为 %2！")
              .arg(appointmentCount)
              .arg(newCapacity));
      return;
    }

    // 更新容量
    currentExpert->setTimeSlotCapacity(timeSlot, newCapacity);

    // 刷新显示
    loadServiceTimes();

    QMessageBox::information(this, "成功",
                             QString("已将时间段 %1 的容量设置为 %2")
                                 .arg(timeSlot)
                                 .arg(newCapacity));
  }
}

// 添加服务时间段
void ExpertDialog::on_addServiceTimeBtn_clicked() {
  if (!currentExpert) return;

  bool ok;
  QString newTimeSlot = QInputDialog::getText(
      this, "添加时间段",
      "请输入新的服务时间段 (格式: 周一：09:00-12:00):", QLineEdit::Normal, "",
      &ok);

  if (!ok || newTimeSlot.isEmpty()) {
    return;
  }

  // 验证时间格式
  QRegularExpression re("^周[一二三四五六日]：\\d{2}:\\d{2}-\\d{2}:\\d{2}$");
  if (!re.match(newTimeSlot).hasMatch()) {
    QMessageBox::warning(this, "格式错误",
                         "请按正确格式输入: 周一：09:00-12:00");
    return;
  }

  // 检查是否有时间冲突
  if (hasTimeConflict(newTimeSlot)) {
    return;  // 已在冲突处理方法中处理
  }

  // 添加时间段
  currentExpert->serviceTimes.append(newTimeSlot);

  // 设置默认容量为5
  currentExpert->setTimeSlotCapacity(newTimeSlot, 5);

  // 刷新显示
  loadServiceTimes();
  updateServiceTimeDisplay();

  QMessageBox::information(
      this, "成功",
      QString("已添加新时间段: %1，默认容量为5人").arg(newTimeSlot));
}

// 删除服务时间段
void ExpertDialog::on_deleteServiceTimeBtn_clicked() {
  if (!currentExpert) return;

  QListWidgetItem* currentItem = ui->serviceTimeList->currentItem();
  if (!currentItem) {
    QMessageBox::warning(this, "提示", "请先选择要删除的时间段！");
    return;
  }

  QString displayText = currentItem->text();
  QString timeSlot = displayText.split(" (")[0];  // 提取时间段部分

  // 检查该时间段是否有预约
  int appointmentCount = 0;
  if (appointmentManager) {
    for (const auto& appointment : appointmentManager->getAllAppointments()) {
      if (appointment.expertName == currentExpert->name &&
          appointment.serviceTime == timeSlot) {
        appointmentCount++;
      }
    }
  }

  if (appointmentCount > 0) {
    QMessageBox::warning(this, "无法删除",
                         QString("该时间段已有 %1 个预约，无法删除！\n"
                                 "请先联系患者修改预约。")
                             .arg(appointmentCount));
    return;
  }

  int ret = QMessageBox::question(
      this, "确认删除", QString("确定要删除时间段 %1 吗？").arg(timeSlot),
      QMessageBox::Yes | QMessageBox::No);

  if (ret == QMessageBox::Yes) {
    // 删除时间段
    currentExpert->serviceTimes.removeOne(timeSlot);
    currentExpert->removeTimeSlotCapacity(timeSlot);

    // 刷新显示
    loadServiceTimes();
    updateServiceTimeDisplay();

    QMessageBox::information(this, "成功",
                             QString("已删除时间段: %1").arg(timeSlot));
  }
}
