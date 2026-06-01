#pragma once

#include <QDebug>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "JsonUtils.h"
#include "cpp/JsonTools.hpp"
#include "eric_data.h"
#include <iostream>



class json  : public QObject
{
  Q_OBJECT

  public:


   Q_INVOKABLE  QStringList jsontostringlist(QString myjsondoc,QString myjsonkey) {


    QStringList datafiles;

    QByteArray det =  myjsondoc.toUtf8();
    //qDebug()<< "all data as byte array are" << qPrintable(det);
    QJsonParseError jsonError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(det,&jsonError);
    if (jsonError.error != QJsonParseError::NoError){
    qDebug() <<"Json error is: " << jsonError.errorString();
    }

    QJsonArray json_array = jsonResponse.array();
    Q_FOREACH (const QJsonValue & value, json_array) {
        qDebug() << "list all data as strings" << value.toObject().value(myjsonkey).toString();
        datafiles <<value.toObject().value(myjsonkey).toString();
    }

    for (int i = 0; i < datafiles.size(); ++i) {
           std::cout <<"data files: " << datafiles.at(i).toLocal8Bit().constData() << std::endl;

    }


    return datafiles;



    }



  Q_INVOKABLE   void testcomplexJson()
    {




        QString jsondata = "[{ \"pid\": \"123\", \"Name\" : \"Eric\" },{ \"pid\": \"124\" , \"Name\" : \"John\" },{ \"pid\": \"125\" , \"Name\" : \"Felix\" },{ \"pid\": \"126\" , \"Name\" : \"holo\" },{ \"pid\": \"127\" , \"Name\" : \"Felixo\" },{ \"pid\": \"128\" , \"Name\" : \"kk\" }]";
        QVariantList myjsonmapdat = eric::JsonUtils::toVariantList(jsondata);

        QJsonArray arr= QJsonArray::fromVariantList(myjsonmapdat);
        QJsonObject obj;
        obj [ "eric" ] = arr;
        const auto docjson = QJsonDocument{ obj };
        const auto jsonstr = QString::fromLatin1( docjson.toJson() );
        qDebug() << "custom document is:\n" << qPrintable( jsonstr );
        QVariantMap mymap = eric::JsonUtils::toVariantMap(jsonstr);
        JsonTools thetools;
        thetools.writeJsonFile(mymap,"/root/qtprojects/json/test.json");
        QVariantMap mydatamap = thetools.readJsonFile("/root/qtprojects/json/test.json");
        QString docfile = eric::JsonUtils::toJsonString(mydatamap);
        qDebug()<< "doc string from variant map from file system is: " << qPrintable(docfile) <<"\n";

     // Test from Dir


        auto filePath = "/root/qtprojects/json/test.json";

         QFile file;
         file.setFileName(filePath);
         QJsonDocument confFile;

         if (file.open(QIODevice::ReadOnly)) {
           QJsonParseError error;
           confFile = QJsonDocument::fromJson(file.readAll(), &error);
         }


         //   Check the validity of file.
         if (!confFile.isNull()) {
           auto contentFile = confFile.object();
           if (contentFile.contains("eric") && contentFile["eric"].isArray()) {

             QJsonArray dataArray = contentFile["eric"].toArray();

             for (auto i = 0; i < dataArray.size(); ++i) {
               if (dataArray[i].isObject()) {
                 auto dataObject = dataArray[i].toObject();


                 qDebug() << "pid are: " << dataObject["pid"].toString();
                 qDebug() << "names are: " << dataObject["Name"].toString();

                ;
               }
             }
           }
           else {

               qDebug()<<"Not Json Array or Object" << Qt::endl;

           }
         }









    }


  Q_INVOKABLE  QVariantList frommaptovariantlist(QString path,QString jsonfile) {




    JsonTools thetools;
    QVariantMap mydatamap = thetools.readJsonFile(path+"/"+jsonfile);
    QList<QVariant> lists = mydatamap.value("eric").toList();
    return lists;







    }


    Q_INVOKABLE  void tomap(QString json,QString path,QString jsonfile) {



        QVariantList myjsonmapdat = eric::JsonUtils::toVariantList(json);
        QJsonArray arr= QJsonArray::fromVariantList(myjsonmapdat);
        QJsonObject obj;
        obj [ "eric" ] = arr;
        const auto docjson = QJsonDocument{ obj };
        const auto jsonstr = QString::fromLatin1( docjson.toJson() );
      //  qDebug() << "custom document is:\n" << qPrintable( jsonstr );
        QVariantMap mydatamap = obj.toVariantMap();
        JsonTools thetools;
        thetools.writeJsonFile(mydatamap,path+"/"+jsonfile);







        return ;







      }
    
    
    
    Q_INVOKABLE  bool isarrayempty(QString json) {



        QVariantList myjsonmapdat = eric::JsonUtils::toVariantList(json);
        QJsonArray arr= QJsonArray::fromVariantList(myjsonmapdat);
        
        bool empty = arr.isEmpty();



        return empty;






      }
    





 Q_INVOKABLE QString valuefromkey(QString key,QString path,QString jsonfile) {


        QString value ="";
        JsonTools thetools;
        QVariantMap mydatamap = thetools.readJsonFile(path+"/"+jsonfile);
        eric::JsonUtils myutils;
        QString mydoc = myutils.toJsonString(mydatamap);
        QJsonParseError jsonError;
        QJsonDocument doc = QJsonDocument::fromJson(mydoc.toUtf8(),&jsonError);
        if (jsonError.error != QJsonParseError::NoError){
        qDebug() <<"Json error is: " << jsonError.errorString();
        }

        QVariantMap map  = doc.object().toVariantMap();
         QList<QVariant> lists  = map.value("eric").toList();
        int listsCount = lists.size();

        for (int listsIdx = 0 ; listsIdx < listsCount ; listsIdx++) {

             QMap<QString,QVariant> listsMap  = lists[listsIdx].toMap();
             value  = listsMap.value(key).toString();
             qDebug() <<"value is: " << value<< Qt::endl;





            }

        return value;





    }


    Q_INVOKABLE QStringList valuesfromkey(QString key,QString path,QString jsonfile) {


           QStringList m;
           JsonTools thetools;
           QVariantMap mydatamap = thetools.readJsonFile(path+"/"+jsonfile);
           eric::JsonUtils myutils;
           QString mydoc = myutils.toJsonString(mydatamap);
           QJsonParseError jsonError;
           QJsonDocument doc = QJsonDocument::fromJson(mydoc.toUtf8(),&jsonError);
           if (jsonError.error != QJsonParseError::NoError){
           qDebug() <<"Json error is: " << jsonError.errorString();
           }

           QVariantMap map  = doc.object().toVariantMap();
           QList<QVariant> lists  = map.value("eric").toList();
           int listsCount = lists.size();

           for (int listsIdx = 0 ; listsIdx < listsCount ; listsIdx++) {

                QMap<QString,QVariant> listsMap  = lists[listsIdx].toMap();
                m << listsMap.value(key).toString();
                qDebug() <<"values are: " << listsMap.value(key).toString()<< Qt::endl;





               }

           return m;





       }




    Q_INVOKABLE  QVariantList custommaptovarialist(QString path,QString jsonfile) {




       JsonTools thetools;
       QVariantMap mydatamap = thetools.readJsonFile(path+"/"+jsonfile);
       eric::JsonUtils myutils;
       QString mydoc = myutils.toJsonString(mydatamap);
     //  qDebug() <<"mydoc is: " << mydoc << Qt::endl;
       QJsonParseError jsonError;
       QJsonDocument doc = QJsonDocument::fromJson(mydoc.toUtf8(),&jsonError);
       if (jsonError.error != QJsonParseError::NoError){
       qDebug() <<"Json error is: " << jsonError.errorString();
       }

       QVariantMap map  = doc.object().toVariantMap();
       QList<QVariant> mylists;
       QList<QVariant> lists  = map.value("eric").toList();
       QVariantList erclists;
       QVector< Appdetails> m_appdetails;
       Appdetails mydetails;
       QStringList lidfiles;
       QStringList avfiles;
       QStringList bbidfiles;
       QStringList namefiles;



       int listsCount = lists.size();


        for (int listsIdx = 0 ; listsIdx < listsCount ; listsIdx++) {

             QMap<QString,QVariant> listsMap  = lists[listsIdx].toMap();
             QString listName  = listsMap.value("donation_type").toString();
             QString bloodtypeName  = listsMap.value("blood_type").toString();
             int  listNum   = listName.mid(5).toInt();
             qDebug() <<"listName is: " << listName<< Qt::endl;
             qDebug() <<"listNum is: " << listNum<< Qt::endl;
             QList<QVariant>   listbags = listsMap.value("global_inventorys_bloodbags").toList();
             int datCount = listbags.size();
             for (int datIdx = 0 ; datIdx < datCount ; datIdx++) {

                QString lidName = listbags[datIdx].toMap().value("lid").toString();
                QString avName = listbags[datIdx].toMap().value("available").toString();
                qDebug() <<"lidName is: " << lidName<< Qt::endl;
                qDebug() <<"avName is: " << avName<< Qt::endl;


                }


             mylists.append(listbags);


            }

       // New Data

        QJsonObject newobj;
        QJsonArray newarr;
        QVariantMap res;

        // New Data

        // start new data which composes nested json

        QString jsondata = myutils.toJsonString(mylists);
        QVariantList myjsonmapdat = eric::JsonUtils::toVariantList(jsondata);
        QJsonArray arr= QJsonArray::fromVariantList(myjsonmapdat);
        QJsonObject obj;
        obj [ "eric" ] = arr;
        const auto docjson = QJsonDocument{ obj };
        QVariantMap mymap = obj.toVariantMap();

        if (!docjson.isNull()) {
          auto contentFile = docjson.object();
          if (contentFile.contains("eric") && contentFile["eric"].isArray()) {

            QJsonArray dataArray = contentFile["eric"].toArray();

            for (auto i = 0; i < dataArray.size(); ++i) {
              if (dataArray[i].isObject()) {
                auto dataObject = dataArray[i].toObject();
                lidfiles << dataObject["lid"].toString();
                avfiles << dataObject["available"].toString();
                bbidfiles << dataObject["bbid"].toString();
                namefiles << dataObject["name"].toString();
              //  qDebug() << "bbid are: " << dataObject["bbid"].toString();
             //   qDebug() << "lid are: " << dataObject["lid"].toString();



              }
            }
          }
          else {

              qDebug()<<"Not Json Array or Object" << Qt::endl;

          }
        }

    // end new data which composes nested json

    // start old data which composes all data

        QString oldjsondata = myutils.toJsonString(lists);
        QVariantList oldjsonmapdat = eric::JsonUtils::toVariantList(oldjsondata);
        QJsonArray oldarr= QJsonArray::fromVariantList(oldjsonmapdat);
        QJsonObject oldobj;
        oldobj [ "eric" ] = oldarr;
        const auto olddocjson = QJsonDocument{ oldobj };

        if (!olddocjson.isNull()) {
          auto oldcontentFile = olddocjson.object();
          if (oldcontentFile.contains("eric") && oldcontentFile["eric"].isArray()) {

            QJsonArray olddataArray = oldcontentFile["eric"].toArray();

            for (auto i = 0; i < olddataArray.size(); ++i) { //start for loop
              if (olddataArray[i].isObject()) {
                auto olddataObject = olddataArray[i].toObject();

               if(lidfiles.size() ==olddataArray.size()) {

                mydetails.mylid = lidfiles.at(i).toLocal8Bit().constData();
                mydetails.myav = avfiles.at(i).toLocal8Bit().constData();
                mydetails.mybbid= bbidfiles.at(i).toLocal8Bit().constData();
                mydetails.myname = namefiles.at(i).toLocal8Bit().constData();

               }


                mydetails.mybt = olddataObject["blood_type"].toString();
                m_appdetails.append(mydetails);
               // std::cout <<"lid files: " << lidfiles.at(i).toLocal8Bit().constData() << std::endl;
               // std::cout <<"av files: " << avfiles.at(i).toLocal8Bit().constData() << std::endl;
               // qDebug() << "blood_type are: " << olddataObject["blood_type"].toString();
               // qDebug() << "donation_type are: " << olddataObject["donation_type"].toString();



              }
            } // end for loop
          }
          else {

              qDebug()<<"Not Json Array or Object" << Qt::endl;

          }
        }

        //  end old data which composes all data


        for (int i = 0; i < m_appdetails.size(); ++i) {


            QJsonObject object;
            object.insert("lid", QJsonValue::fromVariant(m_appdetails.at(i).mylid));
            object.insert("available", QJsonValue::fromVariant(m_appdetails.at(i).myav));
            object.insert("blood_type", QJsonValue::fromVariant(m_appdetails.at(i).mybt));
            object.insert("bbid", QJsonValue::fromVariant(m_appdetails.at(i).mybbid));
            object.insert("name", QJsonValue::fromVariant(m_appdetails.at(i).myname));
            newarr << object;



         }



        newobj [ "eric" ] = newarr;
        const auto newdocjson = QJsonDocument{ newobj };
        QVariantMap newmap = newobj.toVariantMap();
        return newmap.value("eric").toList();







       }



    Q_INVOKABLE  void combinejson(QVariantList eric,QString path,QString jsonfile) {



         //   qDebug() <<"size of list from cpp: " << eric.size() << Qt::endl;
            QString json = eric::JsonUtils::toJsonString(eric);
         //   qDebug() <<"json from cpp is: " << json << Qt::endl;

            QJsonArray arr= QJsonArray::fromVariantList(eric);
            QJsonObject obj;
            obj [ "eric" ] = arr;
            const auto docjson = QJsonDocument{ obj };
            const auto jsonstr = QString::fromLatin1( docjson.toJson() );
          //  qDebug() << "custom document is:\n" << qPrintable( jsonstr );
            QVariantMap mydatamap = obj.toVariantMap();
            JsonTools thetools;
            thetools.writeJsonFile(mydatamap,path+"/"+jsonfile);







            return ;










          }







};

