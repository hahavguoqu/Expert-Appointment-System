#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>

#include "appointmentManager.h"
#include "expertManager.h"

// 主窗口类：医院专家门诊预约系统的主界面与业务入口
namespace Ui {
class MainWindow;
}

class Expert;
class ExpertManager;
class AppointmentManager;
class AIChatDialog;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  // 构造与析构
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

  // 管理员修改密码接口（验证旧密码并设置新密码）
  bool changeAdminPassword(const QString& oldPassword,
                           const QString& newPassword);

 private slots:
  // UI 事件槽（由 UI 元件触发）
  void on_roleComboBox_currentIndexChanged(int index);
  void on_doctorLoginBtn_clicked();
  void on_adminLoginBtn_clicked();
  void importData();  // 手动导入数据（菜单触发）
  void exportData();  // 手动导出数据（菜单触发）
  void openAIChat();  // 打开 AI 助手对话框

 signals:
  // 窗口尺寸改变时发出的信号（用于内部控件重新布局）
  void resized();

 protected:
  // 重载 resizeEvent，用于发射 resized() 信号
  virtual void resizeEvent(QResizeEvent* event) override {
    QMainWindow::resizeEvent(event);
    emit resized();
  }

 private:
  Ui::MainWindow* ui;            // UI 指针（由 Qt Designer 生成）
  ExpertManager* expertManager;  // 专家数据管理器（负责读写/查询专家数据）
  AppointmentManager*
      appointmentManager;     // 预约数据管理器（负责读写/查询预约数据）
  QString adminPassword;      // 管理员密码（程序启动时加载）
  bool isDialogOpen = false;  // 防止重复打开对话框的标志

  // 数据与界面相关的私有方法
  void autoImportData();     // 启动时自动导入数据（资源或项目文件）
  void setupUI();            // 初始化界面元素
  void setupManagers();      // 创建并初始化 manager（expert/appointment）
  void setupRoleComboBox();  // 填充角色选择下拉框
  void resetUIState();       // 重置所有输入框与界面状态
  void openPatientDialog();  // 打开患者预约对话框
  void openExpertDialog(Expert* expert);  // 打开专家管理/查看对话框
  void resetComboBoxToDefault();          // 恢复角色下拉框到默认项
  bool validateInput(const QString& input, const QString& fieldName,
                     QWidget* focusWidget);  // 简单输入校验（非空等）
  QString loadAdminPassword();               // 从存储加载管理员密码
  void onApplicationAboutToQuit();           // 应用退出前保存数据
};

#endif