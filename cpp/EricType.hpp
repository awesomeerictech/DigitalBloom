
#ifndef ERICTYPE_HPP
#define ERICTYPE_HPP

#include <QObject>
#include <QtQml>
#include "JsonTools.hpp"
#include "JsonUtils.h"
#include "json.hpp"
#include "fieldsform.h"
#include "Net/gu.h"
#include "FileHelper.hpp"
#include "VariantHelpers.hpp"
#include "MyTimer.hpp"
#include "ericsettings/SettingsManagerEric.h"
#include "unsafearea.hpp"
#include "sms/sms.h"
#include "EricImage.hpp"




class EricType : public QObject
{
    Q_OBJECT

public:
     EricType(QObject *parent = nullptr);
     void addContextProperty(QQmlContext* context);





Q_SIGNALS:

public Q_SLOTS:

private:

     eric::JsonUtils myutils;
     JsonTools mytools;
     json ericjon;
     eric::Gu gu;
     VariantHelpers myhelpers;
     SettingsManagerEric mysettings;
     FileHelper  fshelper;
     UnsafeArea mUnsafeArea;
     Sms mysms;





};

#endif // ERICTYPE_HPP
