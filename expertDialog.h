#ifndef EXPERT_DIALOG_H
#define EXPERT_DIALOG_H

#include <QDateTime>
#include <QDialog>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QStandardItemModel>

#include "appointmentManager.h"
#include "expert.h"

namespace Ui {
class ExpertDialog;  // 界面指针类（由 Qt Designer 生成）
}

class ExpertDialog : public QDialog {
  Q_OBJECT

 public:
  explicit ExpertDialog(
      Expert* expert, AppointmentManager* appointmentMgr,
      QWidget* parent =
          nullptr);  // 构造函数：使用指定专家与预约管理器初始化对话框
  ~ExpertDialog();   // 析构函数：释放对话框资源

 private slots:
  void on_saveButton_clicked();            // 保存按钮点击处理槽
  void on_closeButton_clicked();           // 关闭按钮点击处理槽
  void on_addScheduleBtn_clicked();        // 添加排班日期按钮点击处理槽
  void on_cancelScheduleBtn_clicked();     // 取消排班日期按钮点击处理槽
  void on_addServiceTimeBtn_clicked();     // 添加服务时间段按钮点击处理槽
  void on_deleteServiceTimeBtn_clicked();  // 删除服务时间段按钮点击处理槽
  void on_changePasswordBtn_clicked();     // 修改密码按钮点击处理槽
  void on_setCapacityBtn_clicked();        // 设置时段容量按钮点击处理槽
  void on_serviceTimeList_itemDoubleClicked(
      QListWidgetItem* item);  // 服务时间列表项双击处理槽
  void on_calendar_clicked(
      const QDate& date);         // 日历点击处理槽，参数为所选日期
  void loadAppointments();        // 加载并显示该专家的预约数据
  void updateAppointmentTable();  // 刷新预约表格数据的显示

 private:
  Ui::ExpertDialog* ui;                    // 指向 UI 对象的指针
  Expert* currentExpert;                   // 当前正在编辑/查看的专家对象指针
  AppointmentManager* appointmentManager;  // 预约管理器指针（用于读写预约数据）
  QStandardItemModel* appointmentModel;    // 用于展示预约列表的模型

  void setupUI();            // 初始化并绑定界面元素
  void loadExpertInfo();     // 将 currentExpert 的信息加载到界面表单中
  void saveExpertInfo();     // 将界面表单内容保存回 currentExpert
  void loadServiceTimes();   // 加载并显示专家的服务时间列表
  void loadScheduleDates();  // 加载并显示专家的排班日期
  void setFormReadOnly(bool readOnly);  // 设置表单可编辑性（只读或可编辑）
  void setupAppointmentTable();         // 配置预约表格的列与模型
  void on_setClosedBtn_clicked();       // 设置专家休诊/关闭日期的按钮处理槽
  void updateCalendarDisplay();         // 根据排班/关闭日期更新日历显示
  bool isValidTimeFormat(const QString& timeStr);  // 校验时间字符串格式是否有效
  bool hasTimeConflict(
      const QString& newTimeSlot);  // 检查新时段是否与现有时段冲突
  void updateServiceTimeDisplay();  // 更新界面上服务时段的文本显示
  void mergeTimeSlots(const QStringList& conflictingSlots,
                      const QString& newTimeSlot, const QString& mergedSlot,
                      int maxCapacity);  // 合并冲突时段并调整容量的辅助方法
};

#endif
