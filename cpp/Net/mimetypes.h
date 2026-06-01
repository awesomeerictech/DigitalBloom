#ifndef MIMETYPES_H
#define MIMETYPES_H

#include "configi.h"
#include <QFile>
#include <QDir>
#include <QStandardPaths>

class MimeTypes
{
public:
    MimeTypes(){



        configi.read(mConfigPath+"mime-types.ini");
        typesToExtentions();
    }
    QString mDataRoot = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0);
    QString mConfigPath = mDataRoot+"/config/";





QByteArray type(QByteArray extention){
        return types[extention];
    }

    QByteArray type(QString extention){
        return types[QByteArray().append(extention.toStdString())];
    }

    QByteArray extention(QByteArray type){

        // list all type's extentions
        QByteArrayList list = configi.get("MIME-TYPES", type.trimmed()).toByteArray().split(',');

        // return first extention, considered as default
        if(!list.empty()) return list.at(0);

        return QByteArray();
    }

private:

    Configi configi;

    // in mime-types.ini, key is type name not extention
    // so reverse it here to map extention to type
    QHash<QByteArray, QByteArray> types;

    void typesToExtentions(){

        ConfigiSection *mime_section = configi.section("MIME-TYPES");

        if(mime_section)
        {
            for(QString type : mime_section->values.keys())
            {
                // list all extentions for every key
                QByteArrayList extentions = mime_section->values[type].toByteArray().split(',');

                // map extention for it's type
                for (QByteArray ext : extentions)
                {
                    types.insert(ext, QByteArray().append(type.toStdString()));
                }
            }
        }
    }

};

#endif // MIMETYPES_H
