#ifndef _QX_DIGITALBLOOMORDERS_DELIVERY_H_
#define _QX_DIGITALBLOOMORDERS_DELIVERY_H_
#include "ids.hpp"
#include "categories.hpp"
#include "items.hpp"

class orders;
class locations;
class logistics;


class QX_DIGITALBLOOMORDERS_DLL_EXPORT delivery : public qx::IxPersistable
{

QX_PERSISTABLE_HPP(delivery)

public:
// -- typedef



    typedef std::shared_ptr<ids> deliz_ptr;


// -- properties
   QString     deli_id;
   QString     deli_delivereruuid;
   QString     deli_recipientuuid;
   QString     deli_locationdelivereruuid;
   QString     deli_locationrecipientuuid;
   QString     deli_logisticsuuid;
   QString     deli_itemuuid;
   QString     deli_smstxtsent;
   QString     deli_smstxtreceived;
   QString     deli_order_uuid;
   QString     deli_delivery_status;
   QString     deli_media_sent;
   QString     deli_media_received;
   QDateTime   deli_sent_created_at;
   QDateTime   deli_received_created_at;
   deliz_ptr       deli_myidsz;
// -- contructor, virtual destructor
//  delivery() { ; }
//delivery() : qx::IxPersistable(), deli_id("0") { ; }
 delivery() : qx::IxPersistable() { ; }
   virtual ~delivery() { ; }
};

QX_REGISTER_PRIMARY_KEY(delivery, QString)
QX_REGISTER_HPP_QX_DIGITALBLOOMORDERS(delivery, qx::trait::no_base_class_defined, 0)

typedef QSharedPointer<delivery> delivery_ptr;
typedef qx::QxCollection<QString, delivery_ptr> list_delivery;

#endif // _QX_DIGITALBLOOMORDERS_DELIVERY_H_
