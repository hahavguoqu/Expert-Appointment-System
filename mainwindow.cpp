#include "mainwindow.h"

#include <QAction>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QSslSocket>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QTimer>

#include "adminDialog.h"
#include "aiChatDialog.h"
#include "expertDialog.h"
#include "patientDialog.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      expertManager(nullptr),
      appointmentManager(nullptr),
      adminPassword(loadAdminPassword()) {  
  ui->setupUi(this);
  setupManagers();
  autoImportData();
  setupUI();

  connect(qApp, &QApplication::aboutToQuit, this,
          &MainWindow::onApplicationAboutToQuit);

  qDebug() << "QSslSocket::supportsSsl() =" << QSslSocket::supportsSsl();
  qDebug() << "Build version:" << QSslSocket::sslLibraryBuildVersionString();
  qDebug() << "Runtime version:" << QSslSocket::sslLibraryVersionString();
}

MainWindow::~MainWindow() {
  delete ui;
  delete expertManager;
  delete appointmentManager;
}

void MainWindow::setupManagers() {
  expertManager = new ExpertManager();
  appointmentManager = new AppointmentManager();
  // appointmentManager->initializeDefaultData();
}

void MainWindow::setupUI() {
  // 设置窗口标题和图标
  this->setWindowTitle("医院专家门诊预约系统");

  // 加载样式表
  QFile styleFile(":/styles/application.qss");
  if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
    QTextStream stream(&styleFile);
    QString style = stream.readAll();
    qApp->setStyleSheet(style); 
    qDebug() << "全局样式表加载成功";
  } else {
    qDebug() << "样式表加载失败: " << styleFile.errorString();
  }

  ui->inputStack->setCurrentIndex(0);
  setupRoleComboBox();
  connect(ui->roleComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &MainWindow::on_roleComboBox_currentIndexChanged);

  // 设置菜单栏
  QMenu* fileMenu = menuBar()->addMenu("数据导入导出");
  QAction* importAction = new QAction("导入数据", this);
  QAction* exportAction = new QAction("导出数据", this);

  fileMenu->addAction(importAction);
  fileMenu->addAction(exportAction);

  connect(importAction, &QAction::triggered, this, &MainWindow::importData);
  connect(exportAction, &QAction::triggered, this, &MainWindow::exportData);

  QAction* aiAction = new QAction("AI助手", this);
  menuBar()->addAction(aiAction);  // 添加到 menubar（位于 fileMenu 右侧）
  connect(aiAction, &QAction::triggered, this, &MainWindow::openAIChat);

  // 创建标签并添加到右下角
  QLabel* logoLabel = new QLabel(this);
  QPixmap originalPixmap(":/images/hospital_logo.png");
  QPixmap scaledPixmap = originalPixmap.scaled(150, 150, Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation);
  logoLabel->setPixmap(scaledPixmap);
  logoLabel->setGeometry(this->width() - 170, this->height() - 170, 150, 150);
  logoLabel->setAttribute(Qt::WA_TransparentForMouseEvents);  
  logoLabel->lower();  

  // 窗口大小改变时重新定位
  connect(this, &MainWindow::resized, [=]() {
    logoLabel->setGeometry(this->width() - 170, this->height() - 170, 150, 150);
  });
}

void MainWindow::setupRoleComboBox() {
  // 清空现有选项
  ui->roleComboBox->clear();

  // 添加选项
  ui->roleComboBox->addItem("请选择用户身份");  
  ui->roleComboBox->addItem("患者预约");        
  ui->roleComboBox->addItem("医生专家");        
  ui->roleComboBox->addItem("管理员");          

  QStandardItemModel* model =
      qobject_cast<QStandardItemModel*>(ui->roleComboBox->model());
  if (model) {
    QStandardItem* item = model->item(0);
    if (item) {
      item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
      item->setData(QVariant(), Qt::UserRole - 1);
    }
  }

  ui->roleComboBox->setCurrentIndex(0);  // 默认设置
}

void MainWindow::resetUIState() {
  // 清空所有输入框
  ui->doctorIdInput->clear();
  ui->doctorPwdInput->clear();
  ui->adminPwdInput->clear();

  resetComboBoxToDefault();

  // 显示空白页面
  ui->inputStack->setCurrentIndex(0);
}

void MainWindow::resetComboBoxToDefault() {
  ui->roleComboBox->blockSignals(true);
  ui->roleComboBox->setCurrentIndex(0);
  ui->roleComboBox->blockSignals(false);
}

bool MainWindow::validateInput(const QString& input, const QString& fieldName,
                               QWidget* focusWidget) {
  if (input.trimmed().isEmpty()) {
    QMessageBox::warning(this, "提示", QString("请输入%1！").arg(fieldName));
    if (focusWidget) {
      focusWidget->setFocus();
    }
    return false;
  }
  return true;
}

void MainWindow::openPatientDialog() {
  PatientDialog dialog(expertManager, appointmentManager, this);
  dialog.setModal(true);
  dialog.exec();
}

void MainWindow::openExpertDialog(Expert* expert) {
  ExpertDialog dialog(expert, appointmentManager, this);
  dialog.exec();
}

// 角色选择变化处理
void MainWindow::on_roleComboBox_currentIndexChanged(int index) {
  switch (index) {
    case 0:
      ui->inputStack->setCurrentIndex(0);
      break;

    case 1:  // "患者预约"
      ui->roleComboBox->blockSignals(true);
      ui->roleComboBox->setCurrentIndex(0);
      ui->roleComboBox->blockSignals(false);

      // 使用延迟重置标志的方式打开对话框
      if (!isDialogOpen) {
        isDialogOpen = true;
        openPatientDialog();
        QTimer::singleShot(300, this, [this]() { isDialogOpen = false; });
      }
      break;

    case 2:                                // "医生专家"
      ui->inputStack->setCurrentIndex(1);  // 显示专家登录页面
      ui->doctorIdInput->clear();
      ui->doctorPwdInput->clear();
      ui->doctorIdInput->setFocus();
      break;

    case 3:                                // "管理员"
      ui->inputStack->setCurrentIndex(2);  // 显示管理员登录页面
      // 清空之前的输入
      ui->adminPwdInput->clear();
      ui->adminPwdInput->setFocus();
      break;

    default:
      qDebug() << "未知的选择索引：" << index;
      ui->inputStack->setCurrentIndex(3);
      // last_opt = 0;
      break;
  }
}

// 专家登录
void MainWindow::on_doctorLoginBtn_clicked() {
  QString id = ui->doctorIdInput->text().trimmed();
  QString password = ui->doctorPwdInput->text().trimmed();

  if (!validateInput(id, "专家编号", ui->doctorIdInput) ||
      !validateInput(password, "密码", ui->doctorPwdInput)) {
    return;
  }

  if (expertManager->verifyExpert(id, password)) {
    Expert* expert = expertManager->findExpertById(id);
    if (expert) {
      resetUIState();
      openExpertDialog(expert);
    }
  } else {
    QMessageBox::warning(this, "登录失败", "专家编号或密码错误！");
    ui->doctorPwdInput->clear();
    ui->doctorPwdInput->setFocus();
  }
}

// 管理员登录
void MainWindow::on_adminLoginBtn_clicked() {
  QString password = ui->adminPwdInput->text().trimmed();

  if (!validateInput(password, "管理员密码", ui->adminPwdInput)) {
    return;
  }

  if (password == adminPassword) {
    resetUIState();
    AdminDialog dialog(expertManager, appointmentManager, this);
    dialog.exec();
  } else {
    QMessageBox::warning(this, "登录失败", "管理员密码错误！");
    ui->adminPwdInput->clear();
    ui->adminPwdInput->setFocus();
  }
}

// 加载密码
QString MainWindow::loadAdminPassword() {
  QSettings settings("HospitalApp", "AppointmentSystem");
  return settings.value("adminPassword", "123456").toString();
}

// 修改密码更改方法，添加保存功能
bool MainWindow::changeAdminPassword(const QString& oldPassword,
                                     const QString& newPassword) {
  if (oldPassword == adminPassword) {
    adminPassword = newPassword;

    // 保存新密码到配置文件
    QSettings settings("HospitalApp", "AppointmentSystem");
    settings.setValue("adminPassword", newPassword);
    settings.sync();  // 确保立即写入

    return true;
  }
  return false;
}

// 导入数据
void MainWindow::importData() {
  bool ok;
  QString password =
      QInputDialog::getText(this, "验证管理员权限", "请输入管理员密码：",
                            QLineEdit::Password, "", &ok);

  if (!ok || password != adminPassword) {
    QMessageBox::warning(this, "访问拒绝", "密码错误或取消操作！");
    return;
  }

  int result = QMessageBox::question(
      this, "导入确认", "导入数据将覆盖当前所有数据，确定要继续吗？",
      QMessageBox::Yes | QMessageBox::No);

  if (result != QMessageBox::Yes) {
    return;
  }

  QString defaultDir = QDir::homePath();
  QString expertFilename = QFileDialog::getOpenFileName(
      this, "导入专家数据", defaultDir, "JSON文件 (*.json);;所有文件 (*)");

  if (!expertFilename.isEmpty()) {
    if (expertManager->loadFromFile(expertFilename)) {
      QMessageBox::information(this, "成功", "专家数据导入成功！");
    } else {
      QMessageBox::warning(this, "失败", "专家数据导入失败！");
    }
  }

  QString appointmentFilename = QFileDialog::getOpenFileName(
      this, "导入预约数据", defaultDir, "JSON文件 (*.json);;所有文件 (*)");

  if (!appointmentFilename.isEmpty()) {
    if (appointmentManager->loadFromFile(appointmentFilename)) {
      QMessageBox::information(this, "成功", "预约数据导入成功！");
    } else {
      QMessageBox::warning(this, "失败", "预约数据导入失败！");
    }
  }
}

// 导出数据
void MainWindow::exportData() {
  bool ok;
  QString password =
      QInputDialog::getText(this, "验证管理员权限", "请输入管理员密码：",
                            QLineEdit::Password, "", &ok);

  if (!ok || password != adminPassword) {
    QMessageBox::warning(this, "访问拒绝", "密码错误或取消操作！");
    return;
  }

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

void MainWindow::openAIChat() {
  AIChatDialog* dlg = new AIChatDialog(expertManager, appointmentManager, this);
  dlg->setAttribute(Qt::WA_DeleteOnClose);
  dlg->show();
}

void MainWindow::autoImportData() {
  const QString expertsFilePath = "resource/experts.json";
  const QString appointmentsFilePath = "resource/appointments.json";

  // 加载专家数据
  QFile expertsFile(expertsFilePath);
  if (expertsFile.exists() &&
      expertsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    // 如果文件存在且可以打开，直接加载
    if (expertManager->loadFromFile(expertsFilePath)) {
      qDebug() << "专家数据从文件加载成功：" << expertsFilePath;
    } else {
      qDebug() << "专家数据从文件加载失败：" << expertsFilePath;
      // 尝试从资源文件加载
      if (expertManager->loadFromFile(":/resource/experts.json")) {
        qDebug() << "专家数据从资源文件加载成功";
      }
    }
    expertsFile.close();
  } else {
    // 如果文件不存在或无法打开，从资源文件加载
    qDebug() << "专家数据文件不存在或无法打开，尝试从资源文件加载";
    if (expertManager->loadFromFile(":/resource/experts.json")) {
      qDebug() << "专家数据从资源文件加载成功";
      // 保存到相对路径，以便下次使用
      QDir dir;
      if (!dir.exists("resource")) {
        dir.mkpath("resource");
      }
      if (expertManager->saveToFile(expertsFilePath)) {
        qDebug() << "专家数据已保存到文件：" << expertsFilePath;
      }
    } else {
      qDebug() << "专家数据从资源文件加载失败";
    }
  }

  // 加载预约数据
  QFile appointmentsFile(appointmentsFilePath);
  if (appointmentsFile.exists() &&
      appointmentsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    // 如果文件存在且可以打开，直接加载
    if (appointmentManager->loadFromFile(appointmentsFilePath)) {
      qDebug() << "预约数据从文件加载成功：" << appointmentsFilePath;
    } else {
      qDebug() << "预约数据从文件加载失败：" << appointmentsFilePath;
      // 尝试从资源文件加载
      if (appointmentManager->loadFromFile(":/resource/appointments.json")) {
        qDebug() << "预约数据从资源文件加载成功";
      }
    }
    appointmentsFile.close();
  } else {
    // 如果文件不存在或无法打开，从资源文件加载
    qDebug() << "预约数据文件不存在或无法打开，尝试从资源文件加载";
    if (appointmentManager->loadFromFile(":/resource/appointments.json")) {
      qDebug() << "预约数据从资源文件加载成功";
      // 保存到相对路径，以便下次使用
      QDir dir;
      if (!dir.exists("resource")) {
        dir.mkpath("resource");
      }
      if (appointmentManager->saveToFile(appointmentsFilePath)) {
        qDebug() << "预约数据已保存到文件：" << appointmentsFilePath;
      }
    } else {
      qDebug() << "预约数据从资源文件加载失败";
    }
  }
}

void MainWindow::onApplicationAboutToQuit() {
  const QString expertsFilePath = "resource/experts.json";
  const QString appointmentsFilePath = "resource/appointments.json";

  // 确保目录存在
  QDir dir;
  if (!dir.exists("resource")) {
    dir.mkpath("resource");
  }

  // 保存专家数据
  if (expertManager) {
    if (expertManager->saveToFile(expertsFilePath)) {
      qDebug() << "专家数据成功保存到：" << expertsFilePath;
    } else {
      qDebug() << "专家数据保存失败：" << expertsFilePath;
    }
  }

  // 保存预约数据
  if (appointmentManager) {
    if (appointmentManager->saveToFile(appointmentsFilePath)) {
      qDebug() << "预约数据成功保存到：" << appointmentsFilePath;
    } else {
      qDebug() << "预约数据保存失败：" << appointmentsFilePath;
    }
  }
  qDebug() << "保存时 QDir::currentPath() =" << QDir::currentPath();
  qDebug() << "将尝试保存到：" << QFileInfo(expertsFilePath).absoluteFilePath();
  qDebug() << "将尝试保存到："
           << QFileInfo(appointmentsFilePath).absoluteFilePath();
}
