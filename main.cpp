#include <QApplication>
#include <QIcon>
#include <QStandardPaths>

#include "mainwindow.h"


int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  // 设置应用程序图标（会显示在窗口左上角和任务栏）
  QIcon appIcon(":/images/doctor.png");
  app.setWindowIcon(appIcon);

  QFile styleFile(":/styles/application.qss");
  if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
    QTextStream stream(&styleFile);
    app.setStyleSheet(stream.readAll());
  }

  MainWindow w;
  w.show();

  return app.exec();
}
