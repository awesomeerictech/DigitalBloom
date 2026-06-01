#pragma once

#include <QMetaType>
#include <QString>


struct Appdetails {

  QString mybt;
  QString mylid;
  QString myname;
  QString myav;
  QString mybbid;


};

Q_DECLARE_METATYPE(Appdetails)
