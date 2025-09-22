#ifndef ADMINDIALOG_H
#define ADMINDIALOG_H

#include <QDialog>
#include <QTableWidgetItem>

#include "appointmentManager.h"
#include "expertManager.h"
#include "patientDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class AdminDialog;
}
QT_END_NAMESPACE

// 前向声明
class MainWindow;

class AdminDialog : public QDialog {
  Q_OBJECT

 public:
  explicit AdminDialog(ExpertManager* expertMgr,
                       AppointmentManager* appointmentMgr,
                       QWidget* parent = nullptr);
  ~AdminDialog();

 private slots:
  void on_changeExpertBtn_clicked();       // 修改专家数据按钮
  void on_addAppointmentBtn_clicked();     // 添加预约按钮
  void on_deleteAppointmentBtn_clicked();  // 删除预约按钮
  void on_changePasswordBtn_clicked();     // 修改密码按钮
  void on_importDataBtn_clicked();         // 导入数据按钮
  void on_exportAppointmentBtn_clicked();  // 导出预约数据按钮
  void on_exportExpertBtn_clicked();       // 导出专家数据按钮
  void on_searchBtn_clicked();             // 搜索按钮
  void onDeleteAppointmentRow(int row);    // 删除指定行（动态连接）
  void onItemChanged(QTableWidgetItem* item);  // 表格项变化（用于验证）
  void onDeleteAppointmentByKey(const QString& patientName,  
                                const QString& dateStr,
                                const QString& expertName,
                                const QString& serviceTime);// 按键删除预约
  bool addExpertDialog(Expert& newExpert);  // 添加专家对话框
  void onExpertCellDoubleClicked(int row, int column);  // 双击专家列弹出下拉

 private:
  void recalcAndApplyQueueNumber(int row, int appointmentIndex);

private:
  Ui::AdminDialog* ui;  // UI 对象指针，访问界面元素
  ExpertManager* expertManager;  // 专家管理器指针
  AppointmentManager* appointmentManager;  // 预约管理器指针
  MainWindow* mainWindow;          // 主窗口指针（用于密码修改）
  PatientDialog* validatorDialog;  // 验证对话框（用于身份证验证）

  void setupUI();                   // 设置界面样式
  void setupTable();                // 配置表格
  void loadAppointments();          // 加载预约数据到表格
  void editExpertDialog(int row);      // 编辑专家对话框
};

#endif 
