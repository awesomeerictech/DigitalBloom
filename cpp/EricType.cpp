
#include "EricType.hpp"
#include <QDebug>
#include <QtQml>
#include <QGuiApplication>
#include <iostream>




EricType::EricType(QObject *parent) : QObject(parent)

{

 qmlRegisterType<FieldsForm>("FieldsForm", 1, 0, "FForm");
 qmlRegisterType<MyTimer>("EricTimer", 1, 0, "MyTimer") ;
 qmlRegisterType<EricImage>("Img", 21, 1, "EricImage") ;

}

void EricType::addContextProperty(QQmlContext *context)
{


     context->setContextProperty("Gu", &gu);
     context->setContextProperty("jsonutils",&myutils);
     context->setContextProperty("jsontools", &mytools);
     context->setContextProperty("myjson", &ericjon);
     context->setContextProperty("fshelpereric", &fshelper);
     context->setContextProperty("varianthelpers", &myhelpers);
     context->setContextProperty("ericsettings", &mysettings);
     context->setContextProperty("unsafeArea", &mUnsafeArea);
     context->setContextProperty("ericsms", &mysms);




}


