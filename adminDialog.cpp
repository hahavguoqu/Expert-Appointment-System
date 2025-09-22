#include "adminDialog.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QVBoxLayout>
#include <algorithm>

#include "mainwindow.h"
#include "patientDialog.h"
#include "ui_adminDialog.h"

AdminDialog::AdminDialog(ExpertManager* expertMgr,
                         AppointmentManager* appointmentMgr, QWidget* parent)
    : QDialog(parent),
      ui(new Ui::AdminDialog),
      expertManager(expertMgr),
      appointmentManager(appointmentMgr),
      mainWindow(qobject_cast<MainWindow*>(parent)) {
  ui->setupUi(this);
  setupUI();
  setupTable();
  loadAppointments();

  // 初始化验证对话框（不显示，仅用于验证）
  validatorDialog = new PatientDialog(expertMgr, appointmentMgr, this);
  validatorDialog->hide();
}

AdminDialog::~AdminDialog() {
  delete validatorDialog;
  delete ui;
}

void AdminDialog::setupUI() {
  QFile styleFile(":/styles/application.qss");

  if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
    QTextStream stream(&styleFile);
    QString style = stream.readAll();
    this->setStyleSheet(style);
    qDebug() << "资源样式文件加载成功";
  } else {
    qDebug() << "资源样式文件加载失败，使用默认样式";
  }
}

void AdminDialog::setupTable() {
  // 设置表格列
  QStringList headers = {"患者姓名", "身份证号", "性别",     "年龄",
                         "预约时间", "联系电话", "症状描述", "专家姓名",
                         "专家科室", "时间段",   "排队号",   "操作"};
  ui->appointmentTable->setColumnCount(headers.size());
  ui->appointmentTable->setHorizontalHeaderLabels(headers);

  // 设置表格属性
  ui->appointmentTable->setAlternatingRowColors(true);
  ui->appointmentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->appointmentTable->horizontalHeader()->setStretchLastSection(true);
  ui->appointmentTable->setShowGrid(true);
  ui->appointmentTable->setSortingEnabled(true);
  ui->appointmentTable->verticalHeader()->hide();
  ui->appointmentTable->verticalHeader()->setDefaultSectionSize(45);

  // 设置列宽（新增身份证号列）
  ui->appointmentTable->setColumnWidth(0, 100);  // 患者姓名
  ui->appointmentTable->setColumnWidth(1, 200);  // 身份证号
  ui->appointmentTable->setColumnWidth(2, 60);   // 性别
  ui->appointmentTable->setColumnWidth(3, 60);   // 年龄
  ui->appointmentTable->setColumnWidth(4, 120);  // 预约时间
  ui->appointmentTable->setColumnWidth(5, 150);  // 联系电话
  ui->appointmentTable->setColumnWidth(6, 150);  // 症状描述
  ui->appointmentTable->setColumnWidth(7, 100);  // 专家姓名
  ui->appointmentTable->setColumnWidth(8, 100);  // 专家科室
  ui->appointmentTable->setColumnWidth(9, 200);  // 时间段
  ui->appointmentTable->setColumnWidth(10, 80);  // 排队号
  ui->appointmentTable->setColumnWidth(11, 40);  // 操作（删除按钮）

  // 连接单元格变化信号，用于验证修改
  connect(ui->appointmentTable, &QTableWidget::itemChanged, this,
          &AdminDialog::onItemChanged);

  // 双击“专家姓名”列时，弹出下拉选择专家
  connect(ui->appointmentTable, &QTableWidget::cellDoubleClicked, this,
          &AdminDialog::onExpertCellDoubleClicked);
}

// 加载预约数据到表格
void AdminDialog::loadAppointments() {
  if (!appointmentManager) return;

  // 断开信号
  disconnect(ui->appointmentTable, &QTableWidget::itemChanged, this,
             &AdminDialog::onItemChanged);

  // 清理所有 cellWidget
  int oldRows = ui->appointmentTable->rowCount();
  for (int i = 0; i < oldRows; ++i) {
    QWidget* widget = ui->appointmentTable->cellWidget(i, 11);
    if (widget) {
      ui->appointmentTable->removeCellWidget(i, 11);
      delete widget;
    }
  }

  const auto& appointments = appointmentManager->getAllAppointments();
  ui->appointmentTable->setRowCount(appointments.size());

  for (int i = 0; i < appointments.size(); ++i) {
    const auto& appointment = appointments[i];

    // 设置所有列的数据
    ui->appointmentTable->setItem(
        i, 0, new QTableWidgetItem(appointment.patientName));
    ui->appointmentTable->setItem(i, 1,
                                  new QTableWidgetItem(appointment.idNumber));
    ui->appointmentTable->setItem(i, 2,
                                  new QTableWidgetItem(appointment.gender));
    ui->appointmentTable->setItem(
        i, 3, new QTableWidgetItem(QString::number(appointment.age)));
    ui->appointmentTable->setItem(
        i, 4,
        new QTableWidgetItem(
            appointment.appointmentDate.toString("yyyy-MM-dd")));
    ui->appointmentTable->setItem(i, 5,
                                  new QTableWidgetItem(appointment.phone));
    ui->appointmentTable->setItem(
        i, 6, new QTableWidgetItem(appointment.description));
    ui->appointmentTable->setItem(i, 7,
                                  new QTableWidgetItem(appointment.expertName));
    ui->appointmentTable->setItem(
        i, 8, new QTableWidgetItem(appointment.expertSubject));
    ui->appointmentTable->setItem(
        i, 9, new QTableWidgetItem(appointment.serviceTime));
    ui->appointmentTable->setItem(
        i, 10, new QTableWidgetItem(QString::number(appointment.queueNumber)));

    // 设置性别和年龄列为不可编辑
    if (ui->appointmentTable->item(i, 2))
      ui->appointmentTable->item(i, 2)->setFlags(Qt::ItemIsEnabled |
                                                 Qt::ItemIsSelectable);
    if (ui->appointmentTable->item(i, 3))
      ui->appointmentTable->item(i, 3)->setFlags(Qt::ItemIsEnabled |
                                                 Qt::ItemIsSelectable);

    // 存储预约索引到第一列项的用户数据中
    if (ui->appointmentTable->item(i, 0)) {
      ui->appointmentTable->item(i, 0)->setData(Qt::UserRole, i);
    }

    // 删除按钮
    QPushButton* delBtn = new QPushButton("删除");
    delBtn->setObjectName("deleteAppointmentBtn");
    delBtn->setProperty("patientName", appointment.patientName);
    delBtn->setProperty("appointmentDate",
                        appointment.appointmentDate.toString("yyyy-MM-dd"));
    delBtn->setProperty("expertName", appointment.expertName);
    delBtn->setProperty("serviceTime", appointment.serviceTime);

    connect(delBtn, &QPushButton::clicked, this, [=]() {
      QString patientName = delBtn->property("patientName").toString();
      QString dateStr = delBtn->property("appointmentDate").toString();
      QString expertName = delBtn->property("expertName").toString();
      QString serviceTime = delBtn->property("serviceTime").toString();
      onDeleteAppointmentByKey(patientName, dateStr, expertName, serviceTime);
    });

    ui->appointmentTable->setCellWidget(i, 11, delBtn);
  }

  // 恢复信号
  connect(ui->appointmentTable, &QTableWidget::itemChanged, this,
          &AdminDialog::onItemChanged);
}

void AdminDialog::onExpertCellDoubleClicked(int row, int column) {
  if (column != 7) return;  // 仅在“专家姓名”列处理

  // 保护性检查
  const auto& appointments = appointmentManager->getAllAppointments();
  if (row < 0 || row >= ui->appointmentTable->rowCount()) return;
  if (row >= appointments.size()) return;

  // 构建下拉框
  QComboBox* combo = new QComboBox(ui->appointmentTable);
  int currentIndex = -1;
  QString currentExpertName = ui->appointmentTable->item(row, 7)
                                  ? ui->appointmentTable->item(row, 7)->text()
                                  : QString();
  for (int ei = 0; ei < expertManager->experts.size(); ++ei) {
    const auto& ex = expertManager->experts[ei];
    combo->addItem(ex.name);
    if (ex.name == currentExpertName) currentIndex = ei;
  }
  if (currentIndex >= 0) combo->setCurrentIndex(currentIndex);

  // 放入单元格
  ui->appointmentTable->setCellWidget(row, 7, combo);

  // 选择后应用变更
  connect(combo, QOverload<int>::of(&QComboBox::activated), this, [=](int idx) {
    QSignalBlocker blocker(ui->appointmentTable);
    const auto& selExpert = expertManager->experts[idx];

    // 更新显示：专家、科室
    ui->appointmentTable->removeCellWidget(row, 7);
    ui->appointmentTable->setItem(row, 7, new QTableWidgetItem(selExpert.name));
    ui->appointmentTable->setItem(row, 8,
                                  new QTableWidgetItem(selExpert.subject));

    // 清空日期、时间段、排队号
    ui->appointmentTable->setItem(row, 4, new QTableWidgetItem(""));
    ui->appointmentTable->setItem(row, 9, new QTableWidgetItem(""));
    ui->appointmentTable->setItem(row, 10, new QTableWidgetItem(""));

    // 写回数据模型
    int appointmentIndex =
        ui->appointmentTable->item(row, 0)->data(Qt::UserRole).toInt();
    if (appointmentIndex >= 0 && appointmentIndex < appointments.size()) {
      Appointment updated = appointments[appointmentIndex];
      updated.expertName = selExpert.name;
      updated.expertSubject = selExpert.subject;
      updated.appointmentDate = QDate();  // 无效，需重选
      updated.serviceTime.clear();
      updated.queueNumber = 0;  // 清空排队号
      appointmentManager->updateAppointment(appointmentIndex, updated);
    }

    QMessageBox::information(
        this, "提示",
        "已更改专家并清空了日期/时间，请重新选择预约日期与时间段。");
  });
}

// 表格项变化
void AdminDialog::onDeleteAppointmentByKey(const QString& patientName,
                                           const QString& dateStr,
                                           const QString& expertName,
                                           const QString& serviceTime) {
  int ret = QMessageBox::question(this, "确认删除", "确定要删除该预约记录吗？",
                                  QMessageBox::Yes | QMessageBox::No);
  if (ret != QMessageBox::Yes) return;

  QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
  auto& appointments = appointmentManager->getAllAppointments();
  int index = -1;
  for (int i = 0; i < appointments.size(); ++i) {
    const auto& appt = appointments[i];
    if (appt.patientName == patientName && appt.appointmentDate == date &&
        appt.expertName == expertName && appt.serviceTime == serviceTime) {
      index = i;
      break;
    }
  }
  if (index >= 0) {
    appointmentManager->removeAppointment(index);
    loadAppointments();
    QMessageBox::information(this, "成功", "预约记录删除成功！");
  } else {
    QMessageBox::warning(this, "失败", "未找到对应的预约记录！");
  }
}

// 增加预约
void AdminDialog::on_addAppointmentBtn_clicked() {
  if (!expertManager || !appointmentManager) {
    QMessageBox::warning(this, "错误", "数据管理器未初始化！");
    return;
  }

  // 以管理员身份弹出患者预约对话框
  PatientDialog dlg(expertManager, appointmentManager, this);
  dlg.setWindowTitle("管理员添加预约");
  if (dlg.exec() == QDialog::Accepted) {
    // 检查身份证号是否出现重复（若出现，保留第一条，删除后续重复记录）
    const auto& appts = appointmentManager->getAllAppointments();
    QMap<QString, QList<int>> positions;
    for (int i = 0; i < appts.size(); ++i) {
      positions[appts[i].idNumber].append(i);
    }

    QList<int> toRemove;
    for (auto it = positions.constBegin(); it != positions.constEnd(); ++it) {
      const QList<int>& idxs = it.value();
      if (idxs.size() > 1) {
        for (int k = 1; k < idxs.size(); ++k)
          toRemove.append(idxs[k]);  // 保留第一个，删除其余
      }
    }

    if (!toRemove.isEmpty()) {
      // 从大到小删除，避免下标错位
      std::sort(toRemove.begin(), toRemove.end(), std::greater<int>());
      for (int idx : toRemove) {
        appointmentManager->removeAppointment(idx);
      }
      loadAppointments();
      QMessageBox::warning(
          this, "错误",
          "检测到重复身份证号的预约记录，已移除重复项。请重新添加。");
      return;
    }

    loadAppointments();
    QMessageBox::information(this, "成功", "预约添加成功！");
  }
}

// 删除预约
void AdminDialog::on_deleteAppointmentBtn_clicked() {
  int currentRow = ui->appointmentTable->currentRow();
  if (currentRow < 0) {
    QMessageBox::warning(this, "提示", "请先选择要删除的预约记录！");
    return;
  }

  int ret =
      QMessageBox::question(this, "确认删除", "确定要删除选中的预约记录吗？",
                            QMessageBox::Yes | QMessageBox::No);

  if (ret == QMessageBox::Yes) {
    appointmentManager->removeAppointment(currentRow);
    loadAppointments();  // 重新加载表格
    QMessageBox::information(this, "成功", "预约记录删除成功！");
  }
}

// 修改密码
void AdminDialog::on_changePasswordBtn_clicked() {
  if (!mainWindow) {
    QMessageBox::warning(this, "错误", "无法访问主窗口！");
    return;
  }

  bool ok;
  QString oldPassword = QInputDialog::getText(
      this, "修改管理员密码", "请输入当前密码：", QLineEdit::Password, "", &ok);

  if (!ok || oldPassword.isEmpty()) {
    return;
  }

  QString newPassword = QInputDialog::getText(
      this, "修改管理员密码", "请输入新密码：", QLineEdit::Password, "", &ok);

  if (!ok || newPassword.isEmpty()) {
    return;
  }

  // 密码强度验证
  if (newPassword.length() < 6) {
    QMessageBox::warning(this, "密码太简单", "密码长度至少6位！");
    return;
  }

  QString confirmPassword = QInputDialog::getText(
      this, "修改管理员密码", "请确认新密码：", QLineEdit::Password, "", &ok);

  if (!ok || confirmPassword != newPassword) {
    QMessageBox::warning(this, "错误", "两次输入的密码不一致！");
    return;
  }

  // 调用主窗口的密码修改方法
  if (mainWindow->changeAdminPassword(oldPassword, newPassword)) {
    QMessageBox::information(
        this, "成功", "管理员密码修改成功！\n新密码将在下次登录时生效。");
    qDebug() << "管理员密码已修改";
  } else {
    QMessageBox::warning(this, "失败", "当前密码不正确！");
  }
}

// 搜索功能
void AdminDialog::on_searchBtn_clicked() {
  QString keyword = ui->searchLineEdit->text().trimmed();
  int type = ui->searchTypeCombo->currentIndex();

  // 断开信号，防止递归
  disconnect(ui->appointmentTable, &QTableWidget::itemChanged, this,
             &AdminDialog::onItemChanged);

  // 清理所有 cellWidget
  int oldRows = ui->appointmentTable->rowCount();
  for (int i = 0; i < oldRows; ++i) {
    QWidget* widget = ui->appointmentTable->cellWidget(i, 11);
    if (widget) {
      ui->appointmentTable->removeCellWidget(i, 11);
      delete widget;
    }
  }

  if (keyword.isEmpty()) {
    // 重新加载全部内容
    loadAppointments();
    connect(ui->appointmentTable, &QTableWidget::itemChanged, this,
            &AdminDialog::onItemChanged);
    return;
  }

  // 查询逻辑
  const auto& appointments = appointmentManager->getAllAppointments();
  QVector<int> matchedRows;
  for (int i = 0; i < appointments.size(); ++i) {
    const auto& appt = appointments[i];
    bool match = false;
    if (type == 0 && appt.expertName.contains(keyword, Qt::CaseInsensitive))
      match = true;
    if (type == 1 && appt.patientName.contains(keyword, Qt::CaseInsensitive))
      match = true;
    if (type == 2 && appt.phone.contains(keyword, Qt::CaseInsensitive))
      match = true;
    if (match) matchedRows.append(i);
  }

  ui->appointmentTable->setRowCount(matchedRows.size());
  for (int row = 0; row < matchedRows.size(); ++row) {
    const auto& appt = appointments[matchedRows[row]];
    ui->appointmentTable->setItem(row, 0,
                                  new QTableWidgetItem(appt.patientName));
    ui->appointmentTable->setItem(row, 1, new QTableWidgetItem(appt.idNumber));
    ui->appointmentTable->setItem(row, 2, new QTableWidgetItem(appt.gender));
    ui->appointmentTable->setItem(
        row, 3, new QTableWidgetItem(QString::number(appt.age)));
    ui->appointmentTable->setItem(
        row, 4,
        new QTableWidgetItem(appt.appointmentDate.toString("yyyy-MM-dd")));
    ui->appointmentTable->setItem(row, 5, new QTableWidgetItem(appt.phone));
    ui->appointmentTable->setItem(row, 6,
                                  new QTableWidgetItem(appt.description));
    ui->appointmentTable->setItem(row, 7,
                                  new QTableWidgetItem(appt.expertName));
    ui->appointmentTable->setItem(row, 8,
                                  new QTableWidgetItem(appt.expertSubject));
    ui->appointmentTable->setItem(row, 9,
                                  new QTableWidgetItem(appt.serviceTime));
    ui->appointmentTable->setItem(
        row, 10, new QTableWidgetItem(QString::number(appt.queueNumber)));

    // 存储原始预约索引到第一列项的用户数据中
    if (ui->appointmentTable->item(row, 0)) {
      ui->appointmentTable->item(row, 0)->setData(Qt::UserRole,
                                                  matchedRows[row]);
    }

    // 添加删除按钮
    QPushButton* delBtn = new QPushButton("删除");
    delBtn->setObjectName("deleteAppointmentBtn");
    delBtn->setProperty("patientName", appt.patientName);
    delBtn->setProperty("appointmentDate",
                        appt.appointmentDate.toString("yyyy-MM-dd"));
    delBtn->setProperty("expertName", appt.expertName);
    delBtn->setProperty("serviceTime", appt.serviceTime);

    connect(delBtn, &QPushButton::clicked, this, [=]() {
      QString patientName = delBtn->property("patientName").toString();
      QString dateStr = delBtn->property("appointmentDate").toString();
      QString expertName = delBtn->property("expertName").toString();
      QString serviceTime = delBtn->property("serviceTime").toString();
      onDeleteAppointmentByKey(patientName, dateStr, expertName, serviceTime);
    });
    ui->appointmentTable->setCellWidget(row, 11, delBtn);
  }

  // 恢复信号
  connect(ui->appointmentTable, &QTableWidget::itemChanged, this,
          &AdminDialog::onItemChanged);
}

// 删除预约记录
void AdminDialog::onDeleteAppointmentRow(int row) {
  int ret = QMessageBox::question(this, "确认删除", "确定要删除该预约记录吗？",
                                  QMessageBox::Yes | QMessageBox::No);
  if (ret == QMessageBox::Yes) {
    appointmentManager->removeAppointment(row);
    loadAppointments();
    QMessageBox::information(this, "成功", "预约记录删除成功！");
  }
}

// 槽函数
void AdminDialog::onItemChanged(QTableWidgetItem* item) {
  disconnect(ui->appointmentTable, &QTableWidget::itemChanged, this,
             &AdminDialog::onItemChanged);

  int row = item->row();
  int col = item->column();
  QString newValue = item->text().trimmed();

  // 从原始数据源获取原值，而不是从表格获取
  const auto& appointments = appointmentManager->getAllAppointments();
  if (row >= appointments.size()) {
    connect(ui->appointmentTable, &QTableWidget::itemChanged, this,
            &AdminDialog::onItemChanged);
    return;
  }

  // 获取预约索引
  int appointmentIndex = -1;
  if (ui->appointmentTable->item(row, 0)) {
    appointmentIndex =
        ui->appointmentTable->item(row, 0)->data(Qt::UserRole).toInt();
  }
  if (appointmentIndex < 0 || appointmentIndex >= appointments.size()) {
    connect(ui->appointmentTable, &QTableWidget::itemChanged, this,
            &AdminDialog::onItemChanged);
    return;
  }

  // 使用原始索引获取正确的原始预约数据
  const auto& originalAppointment = appointments[appointmentIndex];

  QString patientName = ui->appointmentTable->item(row, 0)->text();
  QString expertName = ui->appointmentTable->item(row, 7)->text();
  QString subject = ui->appointmentTable->item(row, 8)->text();
  // 找到对应的 Expert 对象指针（用于后续日期/时间验证）
  Expert* expert = nullptr;
  if (expertManager) {
    auto& list = expertManager->experts;  // 假设是 QList<Expert>
    for (int ei = 0; ei < list.size(); ++ei) {
      if (list[ei].name == expertName) {
        expert = &list[ei];
        break;
      }
    }
  }
  QString dateStr = ui->appointmentTable->item(row, 4)->text();
  QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
  QString timeSlot = ui->appointmentTable->item(row, 9)->text();

  bool needUpdateAppointment = false;
  Appointment updatedAppointment =
      originalAppointment;  // 使用正确的原始数据初始化

  if (col == 0) {  // 修改患者姓名
    if (newValue.isEmpty()) {
      QMessageBox::warning(this, "错误", "患者姓名不能为空！");
      item->setText(originalAppointment.patientName);
      connect(ui->appointmentTable, &QTableWidget::itemChanged, this,
              &AdminDialog::onItemChanged);
      return;
    }
    updatedAppointment.patientName = newValue;
    needUpdateAppointment = true;

  } else if (col == 1) {  // 修改身份证号
    if (!validatorDialog->isValidIdNumber(newValue)) {
      QMessageBox::warning(this, "错误", "身份证号无效！");
      // 恢复原值
      item->setText(originalAppointment.idNumber);
      // 重新连接信号
      connect(ui->appointmentTable, &QTableWidget::itemChanged, this,
              &AdminDialog::onItemChanged);
      return;
    }

    // 自动更新性别和年龄
    QString gender = validatorDialog->getGenderFromId(newValue);
    int age = validatorDialog->getAgeFromId(newValue);

    ui->appointmentTable->item(row, 2)->setText(gender);
    ui->appointmentTable->item(row, 3)->setText(QString::number(age));

    updatedAppointment.idNumber = newValue;
    updatedAppointment.gender = gender;
    updatedAppointment.age = age;
    needUpdateAppointment = true;

  } else if (col == 5) {  // 修改电话号码
    if (!validatorDialog->isValidPhoneNumber(newValue)) {
      QMessageBox::warning(this, "错误", "请输入有效的电话号码！");
      item->setText(originalAppointment.phone);  // 恢复原值
      connect(ui->appointmentTable, &QTableWidget::itemChanged, this,
              &AdminDialog::onItemChanged);
      return;
    }

    updatedAppointment.phone = newValue;
    needUpdateAppointment = true;

  } else if (col == 6) {  // 修改症状描述
    updatedAppointment.description = newValue;
    needUpdateAppointment = true;

  } else if (col == 7 || col == 8) {  // 修改专家姓名或科室
    // 直接编辑被禁止：统一通过双击弹出的下拉来修改
    if (col == 7) {
      // 恢复原值（防止直接编辑）
      item->setText(originalAppointment.expertName);
    } else {
      item->setText(originalAppointment.expertSubject);
    }
    connect(ui->appointmentTable, &QTableWidget::itemChanged, this,
            &AdminDialog::onItemChanged);
    return;
  }

  // 处理日期变更
  if (col == 4) {
    QDate newDate = QDate::fromString(newValue, "yyyy-MM-dd");
    if (!newDate.isValid()) {
      QMessageBox::warning(this, "错误",
                           "日期格式无效，请使用yyyy-MM-dd格式！");
      item->setText(updatedAppointment.appointmentDate.toString("yyyy-MM-dd"));
      connect(ui->appointmentTable, &QTableWidget::itemChanged, this,
              &AdminDialog::onItemChanged);
      return;
    }

    // 检查日期是否在专家出诊范围内
    if (!expert->isAvailableOnDate(newDate)) {
      QMessageBox::warning(this, "错误", "该专家在此日期无出诊安排！");
      item->setText(updatedAppointment.appointmentDate.toString("yyyy-MM-dd"));
      connect(ui->appointmentTable, &QTableWidget::itemChanged, this,
              &AdminDialog::onItemChanged);
      return;
    }

    // 如果当前已有时间段，则校验时间段与新日期是否匹配（星期或特殊日期前缀）
    if (!updatedAppointment.serviceTime.isEmpty()) {
      QString dayOfWeek = Expert::getDayOfWeekString(newDate);
      QString datePrefix = newDate.toString("MM-dd：");
      const QString& slot = updatedAppointment.serviceTime;
      bool matchesDate =
          (slot.startsWith(dayOfWeek)) || (slot.startsWith(datePrefix));
      bool existsInExpert = false;
      for (const QString& s : expert->serviceTimes) {
        if (s == slot) {
          existsInExpert = true;
          break;
        }
      }
      if (!matchesDate || !existsInExpert) {
        QMessageBox::information(this, "提示",
                                 "所选时间段与新日期不匹配，已清空时间段与排队"
                                 "号，请重新选择时间段。");
        updatedAppointment.serviceTime.clear();
        updatedAppointment.queueNumber = 0;
        if (ui->appointmentTable->item(row, 9))
          ui->appointmentTable->item(row, 9)->setText("");
        if (ui->appointmentTable->item(row, 10))
          ui->appointmentTable->item(row, 10)->setText("");
      } else {
        // 立即计算新的排队号
        int count = 0;
        for (const auto& a : appointmentManager->getAllAppointments()) {
          if (a.expertName == updatedAppointment.expertName &&
              a.serviceTime == updatedAppointment.serviceTime &&
              a.appointmentDate == newDate) {
            ++count;
          }
        }
        updatedAppointment.queueNumber = count + 1;
        if (ui->appointmentTable->item(row, 10)) {
          ui->appointmentTable->item(row, 10)->setText(
              QString::number(updatedAppointment.queueNumber));
        }
      }
    }
    updatedAppointment.appointmentDate = newDate;
    needUpdateAppointment = true;
  }

  // 处理时间段变更
  if (col == 9) {
    bool timeSlotValid = false;
    if (!date.isValid()) {
      QMessageBox::warning(this, "错误", "请先填写有效的预约日期！");
      item->setText(updatedAppointment.serviceTime);
      connect(ui->appointmentTable, &QTableWidget::itemChanged, this,
              &AdminDialog::onItemChanged);
      return;
    }
    // 日期是否是专家就诊日
    if (!expert->isAvailableOnDate(date)) {
      QMessageBox::warning(this, "错误", "该专家在此日期无出诊安排！");
      item->setText(updatedAppointment.serviceTime);
      connect(ui->appointmentTable, &QTableWidget::itemChanged, this,
              &AdminDialog::onItemChanged);
      return;
    }
    QString dayOfWeek = Expert::getDayOfWeekString(date);
    QString datePrefix = date.toString("MM-dd：");

    for (const QString& slot : expert->serviceTimes) {
      if (slot == newValue &&
          (
              // 常规出诊：时间段前缀必须与日期对应的星期几一致
              slot.startsWith(dayOfWeek) ||
              // 特殊出诊：时间段前缀必须与具体日期匹配
              slot.startsWith(datePrefix))) {
        timeSlotValid = true;
        break;
      }
    }

    if (!timeSlotValid) {
      QMessageBox::warning(this, "错误", "该专家在此日期没有此时间段！");
      item->setText(updatedAppointment.serviceTime);
      connect(ui->appointmentTable, &QTableWidget::itemChanged, this,
              &AdminDialog::onItemChanged);
      return;
    }

    updatedAppointment.serviceTime = newValue;
    // 若日期已存在，则立即计算新的排队号
    if (updatedAppointment.appointmentDate.isValid()) {
      int count = 0;
      for (const auto& a : appointmentManager->getAllAppointments()) {
        if (a.expertName == updatedAppointment.expertName &&
            a.serviceTime == newValue &&
            a.appointmentDate == updatedAppointment.appointmentDate) {
          ++count;
        }
      }
      updatedAppointment.queueNumber = count + 1;
      if (ui->appointmentTable->item(row, 10)) {
        ui->appointmentTable->item(row, 10)->setText(
            QString::number(updatedAppointment.queueNumber));
      }
    }
    needUpdateAppointment = true;
  }

  // 更新预约信息
  if (needUpdateAppointment) {
    // 若专家+日期+时间均有效，自动重算排队号
    if (!updatedAppointment.expertName.isEmpty() &&
        updatedAppointment.appointmentDate.isValid() &&
        !updatedAppointment.serviceTime.isEmpty()) {
      int count = 0;
      const auto& all = appointmentManager->getAllAppointments();
      for (const auto& ap : all) {
        if (ap.expertName == updatedAppointment.expertName &&
            ap.appointmentDate == updatedAppointment.appointmentDate &&
            ap.serviceTime == updatedAppointment.serviceTime) {
          count++;
        }
      }
      // count 是包含该记录前已有的数量，因此将该记录设为 count
      updatedAppointment.queueNumber = count;
    } else {
      updatedAppointment.queueNumber = 0;
    }

    appointmentManager->updateAppointment(appointmentIndex, updatedAppointment);
  }

  connect(ui->appointmentTable, &QTableWidget::itemChanged, this,
          &AdminDialog::onItemChanged);
}

void AdminDialog::on_exportAppointmentBtn_clicked() {
  QString defaultDir = QDir::homePath();
  QString appointmentFilename = QFileDialog::getSaveFileName(
      this, "导出预约数据", defaultDir + "/appointments.json",
      "JSON文件 (*.json);;所有文件 (*)");

  if (!appointmentFilename.isEmpty()) {
    if (appointmentManager->saveToFile(appointmentFilename)) {
      QMessageBox::information(this, "成功", "预约数据导出成功！");
    } else {
      QMessageBox::warning(this, "失败", "预约数据导出失败！");
    }
  }
}

void AdminDialog::on_exportExpertBtn_clicked() {
  QString defaultDir = QDir::homePath();
  QString expertFilename = QFileDialog::getSaveFileName(
      this, "导出专家数据", defaultDir + "/experts.json",
      "JSON文件 (*.json);;所有文件 (*)");

  if (!expertFilename.isEmpty()) {
    if (expertManager->saveToFile(expertFilename)) {
      QMessageBox::information(this, "成功", "专家数据导出成功！");
    } else {
      QMessageBox::warning(this, "失败", "专家数据导出失败！");
    }
  }
}

// 导入数据
void AdminDialog::on_importDataBtn_clicked() {
  int result = QMessageBox::question(
      this, "导入确认", "导入数据将覆盖当前所有数据，确定要继续吗？",
      QMessageBox::Yes | QMessageBox::No);

  if (result != QMessageBox::Yes) {
    return;
  }

  // 阻塞表格信号，防止导入过程中触发 itemChanged
  ui->appointmentTable->blockSignals(true);

  QString defaultDir = QDir::homePath();
  QString expertFilename = QFileDialog::getOpenFileName(
      this, "导入专家数据", defaultDir, "JSON文件 (*.json);;所有文件 (*)");

  if (!expertFilename.isEmpty()) {
    if (expertManager->loadFromFile(expertFilename)) {
      QMessageBox::information(this, "成功", "专家数据导入成功！");
    } else {
      QMessageBox::warning(this, "失败", "专家数据导入失败！");
      ui->appointmentTable->blockSignals(false);
      return;
    }
  }

  QString appointmentFilename = QFileDialog::getOpenFileName(
      this, "导入预约数据", defaultDir, "JSON文件 (*.json);;所有文件 (*)");

  if (!appointmentFilename.isEmpty()) {
    if (appointmentManager->loadFromFile(appointmentFilename)) {
      // 重新加载预约表格
      loadAppointments();
      QMessageBox::information(this, "成功", "预约数据导入成功！");
    } else {
      QMessageBox::warning(this, "失败", "预约数据导入失败！");
    }
  }

  // 恢复表格信号
  ui->appointmentTable->blockSignals(false);
}

void AdminDialog::on_changeExpertBtn_clicked() {
  // 创建专家管理对话框
  QDialog* expertDialog = new QDialog(this);
  expertDialog->setWindowTitle("专家数据管理");
  expertDialog->setModal(true);
  expertDialog->resize(800, 600);

  // 创建布局
  QVBoxLayout* layout = new QVBoxLayout(expertDialog);

  // 创建专家表格
  QTableWidget* expertTable = new QTableWidget();
  expertTable->setColumnCount(8);
  expertTable->setHorizontalHeaderLabels(
      {"ID", "姓名", "性别", "年龄", "职称", "科室", "密码", "操作"});
  layout->addWidget(expertTable);

  // 创建按钮布局
  QHBoxLayout* buttonLayout = new QHBoxLayout();
  QPushButton* addBtn = new QPushButton("添加专家");
  QPushButton* saveBtn = new QPushButton("保存修改");
  QPushButton* cancelBtn = new QPushButton("取消");
  buttonLayout->addWidget(addBtn);
  buttonLayout->addStretch();
  buttonLayout->addWidget(saveBtn);
  buttonLayout->addWidget(cancelBtn);
  layout->addLayout(buttonLayout);

  // 加载专家数据到表格
  std::function<void()> loadExperts;
  loadExperts = [&]() {
    expertTable->setRowCount(expertManager->experts.size());
    for (int i = 0; i < expertManager->experts.size(); ++i) {
      const auto& expert = expertManager->experts[i];
      expertTable->setItem(i, 0, new QTableWidgetItem(expert.id));
      expertTable->setItem(i, 1, new QTableWidgetItem(expert.name));
      expertTable->setItem(i, 2, new QTableWidgetItem(expert.gender));
      expertTable->setItem(i, 3,
                           new QTableWidgetItem(QString::number(expert.age)));
      expertTable->setItem(i, 4, new QTableWidgetItem(expert.title));
      expertTable->setItem(i, 5, new QTableWidgetItem(expert.subject));
      expertTable->setItem(i, 6, new QTableWidgetItem(expert.password));
      QLineEdit* passwordEdit = new QLineEdit(expert.password);
      expertTable->setCellWidget(i, 6, passwordEdit);

      // 连接 QLineEdit 的 textChanged 信号来更新数据
      connect(
          passwordEdit, &QLineEdit::textChanged, this,
          [=](const QString& text) {
            if (i < expertManager->experts.size()) {  
              expertManager->experts[i].password =
                  text; 
            }
          });

      // 删除按钮
      QPushButton* delBtn = new QPushButton("删除");
      delBtn->setObjectName("deleteAppointmentBtn");
      delBtn->setProperty("expertId", expert.id);
      delBtn->setProperty("expertName", expert.name);
      connect(delBtn, &QPushButton::clicked, this, [=]() {
        if (QMessageBox::question(
                expertDialog, "确认删除",
                QString("确定要删除专家 %1 吗？").arg(expert.name),
                QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
          return;
        expertManager->experts.removeAt(i);
        loadExperts();
      });
      expertTable->setCellWidget(i, 7, delBtn);
    }
  };

  loadExperts();

  //连接表格 itemChanged 信号，实现直接编辑并实时更新数据
  connect(expertTable, &QTableWidget::itemChanged, this,
          [=](QTableWidgetItem* item) {
            int row = item->row();
            int col = item->column();
            QString newValue = item->text().trimmed();

            if (row < 0 || row >= expertManager->experts.size()) return;

            // 验证输入（可选，根据需要添加更多验证）
            if (col == 0 && newValue.isEmpty()) {  // ID 不能为空
              QMessageBox::warning(expertDialog, "错误", "专家ID不能为空！");
              item->setText(expertManager->experts[row].id);  // 恢复原值
              return;
            }
            if (col == 1 && newValue.isEmpty()) {  // 姓名不能为空
              QMessageBox::warning(expertDialog, "错误", "专家姓名不能为空！");
              item->setText(expertManager->experts[row].name);
              return;
            }
            if (col == 3) {  // 年龄必须是数字
              bool ok;
              int age = newValue.toInt(&ok);
              if (!ok || age < 0 || age > 150) {
                QMessageBox::warning(expertDialog, "错误",
                                     "请输入有效的年龄（0-150）！");
                item->setText(QString::number(expertManager->experts[row].age));
                return;
              }
            }

            // 实时更新 expertManager 中的数据
            switch (col) {
              case 0:
                expertManager->experts[row].id = newValue;
                break;
              case 1:
                expertManager->experts[row].name = newValue;
                break;
              case 2:
                expertManager->experts[row].gender = newValue;
                break;
              case 3:
                expertManager->experts[row].age = newValue.toInt();
                break;
              case 4:
                expertManager->experts[row].title = newValue;
                break;
              case 5:
                expertManager->experts[row].subject = newValue;
                break;
              case 6:
                expertManager->experts[row].password = newValue;
                break;
              default:
                break;
            }
          });

  // 添加专家
  connect(addBtn, &QPushButton::clicked, this, [=]() {
    Expert newExpert;
    if (addExpertDialog(newExpert)) {
      expertManager->experts.append(newExpert);
      loadExperts();
    }
  });

  // 取消
  connect(cancelBtn, &QPushButton::clicked, expertDialog, &QDialog::reject);

  expertDialog->exec();
  delete expertDialog;
}

void AdminDialog::editExpertDialog(int row) {
  if (row < 0 || row >= expertManager->experts.size()) return;

  // 创建专家对象的副本而不是引用
  Expert expertCopy = expertManager->experts[row];
  QDialog* editDialog = new QDialog(this);
  editDialog->setWindowTitle("编辑专家信息");
  editDialog->setModal(true);

  QFormLayout* form = new QFormLayout(editDialog);
  QLineEdit* idEdit = new QLineEdit(expertCopy.id);
  QLineEdit* nameEdit = new QLineEdit(expertCopy.name);
  QLineEdit* genderEdit = new QLineEdit(expertCopy.gender);
  QLineEdit* ageEdit = new QLineEdit(QString::number(expertCopy.age));
  QLineEdit* titleEdit = new QLineEdit(expertCopy.title);
  QLineEdit* subjectEdit = new QLineEdit(expertCopy.subject);
  QLineEdit* passwordEdit = new QLineEdit(expertCopy.password);
  passwordEdit->setEchoMode(QLineEdit::Password);

  // 其他表单设置保持不变
  form->addRow("ID:", idEdit);
  form->addRow("姓名:", nameEdit);
  form->addRow("性别:", genderEdit);
  form->addRow("年龄:", ageEdit);
  form->addRow("职称:", titleEdit);
  form->addRow("科室:", subjectEdit);
  form->addRow("密码:", passwordEdit);

  QHBoxLayout* buttonLayout = new QHBoxLayout();
  QPushButton* saveBtn = new QPushButton("保存");
  QPushButton* cancelBtn = new QPushButton("取消");
  buttonLayout->addStretch();
  buttonLayout->addWidget(saveBtn);
  buttonLayout->addWidget(cancelBtn);
  form->addRow(buttonLayout);

  connect(saveBtn, &QPushButton::clicked, this, [&]() {
    // 修改副本
    expertCopy.id = idEdit->text();
    expertCopy.name = nameEdit->text();
    expertCopy.gender = genderEdit->text();
    expertCopy.age = ageEdit->text().toInt();
    expertCopy.title = titleEdit->text();
    expertCopy.subject = subjectEdit->text();
    expertCopy.password = passwordEdit->text();

    // 更新原始数据
    expertManager->updateExpert(row, expertCopy);

    QMessageBox::information(editDialog, "成功", "专家信息已更新！");
    editDialog->accept();
  });

  connect(cancelBtn, &QPushButton::clicked, editDialog, &QDialog::reject);

  editDialog->exec();
  delete editDialog;
}

bool AdminDialog::addExpertDialog(Expert& newExpert) {
  QDialog* addDialog = new QDialog(this);
  addDialog->setWindowTitle("添加专家");
  addDialog->setModal(true);

  QFormLayout* form = new QFormLayout(addDialog);
  QLineEdit* idEdit = new QLineEdit();
  QLineEdit* nameEdit = new QLineEdit();
  QLineEdit* genderEdit = new QLineEdit();
  QLineEdit* ageEdit = new QLineEdit();
  QLineEdit* titleEdit = new QLineEdit();
  QLineEdit* subjectEdit = new QLineEdit();
  QLineEdit* passwordEdit = new QLineEdit();
  passwordEdit->setEchoMode(QLineEdit::Password);

  form->addRow("ID:", idEdit);
  form->addRow("姓名:", nameEdit);
  form->addRow("性别:", genderEdit);
  form->addRow("年龄:", ageEdit);
  form->addRow("职称:", titleEdit);
  form->addRow("科室:", subjectEdit);
  form->addRow("密码:", passwordEdit);

  QHBoxLayout* buttonLayout = new QHBoxLayout();
  QPushButton* saveBtn = new QPushButton("保存");
  QPushButton* cancelBtn = new QPushButton("取消");
  buttonLayout->addStretch();
  buttonLayout->addWidget(saveBtn);
  buttonLayout->addWidget(cancelBtn);
  form->addRow(buttonLayout);

  bool accepted = false;

  connect(saveBtn, &QPushButton::clicked, this, [&]() {
    // 验证输入
    if (idEdit->text().trimmed().isEmpty()) {
      QMessageBox::warning(addDialog, "错误", "请输入专家ID！");
      return;
    }
    if (nameEdit->text().trimmed().isEmpty()) {
      QMessageBox::warning(addDialog, "错误", "请输入专家姓名！");
      return;
    }
    if (passwordEdit->text().trimmed().isEmpty()) {
      QMessageBox::warning(addDialog, "错误", "请输入密码！");
      return;
    }

    // 检查ID是否已存在
    for (const auto& expert : expertManager->experts) {
      if (expert.id == idEdit->text().trimmed()) {
        QMessageBox::warning(addDialog, "错误", "该ID已存在，请使用其他ID！");
        return;
      }
    }

    // 设置专家信息
    newExpert.id = idEdit->text().trimmed();
    newExpert.name = nameEdit->text().trimmed();
    newExpert.gender = genderEdit->text().trimmed();
    newExpert.age = ageEdit->text().toInt();
    newExpert.title = titleEdit->text().trimmed();
    newExpert.subject = subjectEdit->text().trimmed();
    newExpert.password = passwordEdit->text().trimmed();

    accepted = true;
    addDialog->accept();
  });

  connect(cancelBtn, &QPushButton::clicked, addDialog, &QDialog::reject);

  addDialog->exec();
  delete addDialog;

  return accepted;
}