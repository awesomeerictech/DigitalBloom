#include "precompiled.h"

#include "ids.hpp"
#include "categories.hpp"
#include "items.hpp"
#include "orders.h"
#include "delivery.h"
#include "locations.h"
#include "metrics.hpp"
#include "MyJsonTools.hpp"


QX_REGISTER_CPP_QX_DIGITALBLOOMORDERS(ids)
QX_PERSISTABLE_CPP(ids)

namespace qx {
template <> void register_class(QxClass<ids> & t)
{
   t.id(& ids::myids_id, "ids_id");
   t.data(& ids::myids_username, "username");
   t.data(& ids::myids_phone, "phone");
   t.data(& ids::myids_avatarname, "avatarname");
   t.data(& ids::myids_idcardfront, "idfront");
   t.data(& ids::myids_idcardback, "idback");
   t.data(& ids::myids_adminnames, "adminnames");
   t.data(& ids::myids_adname, "adname");
   t.data(& ids::myids_email, "email");
   t.data(& ids::myids_mainurl, "mainurl");
   t.data(& ids::myids_websiteurl, "websiteurl");
   t.data(& ids::myids_helpurl, "helpurl");
   t.data(& ids::myids_subscription, "subscription");
   t.data(& ids::myids_blacklist, "blacklist");
   t.data(& ids::myids_policy, "policy");
   t.data(& ids::myids_roles, "roles");
   t.data(& ids::myids_responsibilities, "responsibilities");
   t.data(& ids::myids_verification, "verification");
   t.data(& ids::myids_account, "account");
   t.data(& ids::myids_businesstype, "business");
   t.data(& ids::myids_authoritytype,"authority");
   t.data(& ids::myids_logisticstype,"logistics");
   t.data(& ids::myids_created_at, "created_at");
   t.relationOneToMany(& ids::myids_list_catx, "list_catids", "ids_id");
   t.relationOneToMany(& ids::myids_list_itemsx, "list_itemsids", "ids_id");
   t.relationOneToMany(& ids::myids_list_ordersx, "list_ordersids", "ids_id");
   t.relationOneToMany(& ids::myids_list_deliveryx, "list_deliveryids", "ids_id");
   t.relationOneToMany(& ids::myids_list_logisticsx, "list_logisticsids", "ids_id");
   t.relationOneToMany(& ids::myids_list_locationsx, "list_locationsids", "ids_id");
   t.relationOneToMany(& ids::myids_list_metricsx, "list_metricsids", "ids_id");
   t.fctStatic_1<QJsonValue, const QJsonValue>(& ids::deletemediaids, "deletemediaids");




}}


QJsonValue ids::deletemediaids(const QJsonValue & request) {

    std::shared_ptr<MyJsonTools> mypayload {new MyJsonTools};
    mypayload->processRequest(request);
    QJsonObject mydearpayload = mypayload->m_requestJson.toObject();
    QJsonValue myregvalue = mydearpayload.value("media");
    QJsonArray idsitems = myregvalue.toArray();
    bool wasdeleted = false;
    for (qsizetype i = 0; i < idsitems.size(); ++i) {


     std::string oldavatarfile = mypayload->UPLOAD_DIRIDS +'/'+idsitems.at(i).toString().toStdString();
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





