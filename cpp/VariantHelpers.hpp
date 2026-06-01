#ifndef VARIANTMAPHELPER_HPP
#define VARIANTMAPHELPER_HPP
#include <QObject>
#include <qvariant.h>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>


class VariantHelpers : public QObject
{
Q_OBJECT
public:
  Q_INVOKABLE QVariantMap jsonmapdata(const QString data) const {



        QVariantMap map;

            if(data=="") {
                qWarning() << "Data not Found " << data;
                return map;
            }

            QJsonDocument jda = QJsonDocument::fromJson(data.toUtf8());

            if(!jda.isObject()) {
                qWarning() << "Couldn't create JSON from data: " << data;
                return map;
            }
            qDebug() << "QJsonDocument for data with Object :)";
            map = jda.toVariant().toMap();
            return map;

  }

    Q_INVOKABLE  QVariantList jsonlistdata(const QString key,QVariantMap dataMap) const {


            QStringList jsondata = dataMap.value(key).toStringList();
          QVariantList dataList;

              if(jsondata.contains("")) {
                  qWarning() << "No Data see it: " << jsondata;
                  return dataList;
              }

              else{

      //  QVector<QVariant> vector(jsondata.size());
      //  std::copy(jsondata.begin(), jsondata.end(), vector.begin());
      //  dataList = vector.toList();

                  Q_FOREACH(QString s, jsondata){
                    dataList << s;
                  }


              }
              return dataList;

    }

    Q_INVOKABLE QString valuefrommap(QVariantMap m,QString key) {


        return m.value(key).toString();


    }

    Q_INVOKABLE  QString jsonstrdata(const QString key,QVariantMap dataMap) const {


            QString jsondata = dataMap.value(key).toString();


              if(jsondata.contains("")) {
                  qWarning() << "No Data see it: " << jsondata;
                  return jsondata;

              }




              return jsondata;



    }




};

#endif // VARIANTMAPHELPER_HPP
