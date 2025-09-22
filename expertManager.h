#ifndef EXPERTMANAGER_H
#define EXPERTMANAGER_H

#include <QDate>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>

#include "expert.h"
#include "expertManager.h"

class ExpertManager {
 public:
  ExpertManager();
  QList<Expert> experts;

  Expert* findExpertById(const QString& id);
  bool verifyExpert(const QString& id, const QString& password);
  bool saveToFile(const QString& filename) const;
  bool loadFromFile(const QString& filename);
  void updateExpert(int index, const Expert& updatedExpert);
};

#endif 