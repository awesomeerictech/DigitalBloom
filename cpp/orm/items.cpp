#include "precompiled.h"
#include "items.hpp"
#include "orders.h"
#include "delivery.h"
#include "ids.hpp"
#include "MyJsonTools.hpp"


QX_REGISTER_CPP_QX_DIGITALBLOOMORDERS(items)
QX_PERSISTABLE_CPP(items)

namespace qx {
template <> void register_class(QxClass<items> & t)
{

   t.id(& items::ite_id, "items_id");
   t.data(& items::ite_username, "username");
   t.data(& items::ite_itemprice, "price");
   t.data(& items::ite_itemname, "name");
   t.data(& items::ite_itemcategory, "category");
   t.data(& items::ite_mediafile, "mediafile");
   t.data(& items::ite_itemsizeorquantity, "sizeorquantity");
   t.data(& items::ite_standardized, "standardized");
   t.data(& items::ite_standardizedavatar, "standardizedavatar");
   t.data(& items::ite_iteer_uuid, "uuid");
   t.data(& items::ite_itemmanufacturer, "manufacturer");
   t.data(& items::ite_itemdescription, "description");
   t.data(& items::ite_created_at, "created_at");
   t.relationManyToOne(& items::ite_myidsz, "ids_id");
   t.fctStatic_1<QJsonValue,const QJsonValue &>(& items::deletemediaite, "deletemediaite");


}}


QJsonValue items::deletemediaite(const QJsonValue & request) {

    std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
    mypayload->processRequest(request);
    QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
    QJsonValue myregvalue = mydearpayload.value("media");
    QJsonArray mediaitems = myregvalue.toArray();
    bool wasdeleted = false;
    for (qsizetype i = 0; i < mediaitems.size(); ++i) {


     std::string oldavatarfile = mypayload->UPLOAD_DIRIDS +'/'+mediaitems.at(i).toString().toStdString();
     //   qDebug() << "oldavatarfile in deletemediaite is: " << QString::fromStdString(oldavatarfile) << Qt::endl;
      
        
     QFile file(QString::fromStdString(oldavatarfile));
     if (!file.open(QIODevice::ReadOnly | QIODevice::WriteOnly)) {
     
     qDebug() << "file could not be opened: " << QString::fromStdString(oldavatarfile) << Qt::endl;
     
     }
     
     else {
     
      if(file.remove()) {
      
      puts( "File successfully deleted" );
      wasdeleted = true;
      
      }
      
      else {
      
      perror( "Error deleting file:" );
      wasdeleted = false;
      
      }
     
     
     }
     
     file.close();

    
     }

    QJsonObject response;
    response.insert("deletestatus", wasdeleted);
    return response;



}














