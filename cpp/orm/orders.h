#ifndef _QX_DIGITALBLOOMORDERS_ORDERS_H_
#define _QX_DIGITALBLOOMORDERS_ORDERS_H_

#include "locations.h"
#include "logistics.h"
#include "ids.hpp"
#include "categories.hpp"
#include "items.hpp"

//#include "delivery.h"



class QX_DIGITALBLOOMORDERS_DLL_EXPORT orders : public qx::IxPersistable
{

QX_PERSISTABLE_HPP(orders)
QX_REGISTER_FRIEND_CLASS(orders)

public:
// -- properties
  // -- typedef


  typedef std::shared_ptr<ids> ordz_ptr;



   QString        ord_id;
   QString        ord_delivereruuid;
   QString        ord_recipientuuid;
   QString        ord_locationto_uuid;
   QString        ord_locationfrom_uuid;
   QString        ord_itemuuid;
   QDateTime      ord_created_at;
   ordz_ptr       ord_myidsz;

// -- contructor, virtual destructor
//   orders() { ; }
// orders() : qx::IxPersistable(), ord_id("0") { ; }
   orders() : qx::IxPersistable() { ; }
   virtual ~orders() { ; }
};

QX_REGISTER_PRIMARY_KEY(orders, QString)
QX_REGISTER_HPP_QX_DIGITALBLOOMORDERS(orders, qx::trait::no_base_class_defined, 0)

typedef std::shared_ptr<orders> orders_ptr;
//typedef std::vector<orders_ptr> list_orders;
typedef qx::QxCollection<QString, orders_ptr> list_orders;


#endif // _QX_DIGITALBLOOMORDERS_ORDERS_H_
