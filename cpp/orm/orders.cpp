#include "precompiled.h"

#include "orders.h"
#include "ids.hpp"


QX_REGISTER_CPP_QX_DIGITALBLOOMORDERS(orders)
QX_PERSISTABLE_CPP(orders)

namespace qx {
template <> void register_class(QxClass<orders> & t)
{
   t.id(& orders::ord_id, "orders_id");
   t.data(& orders::ord_delivereruuid, "deliverer");
   t.data(& orders::ord_recipientuuid, "recipient");
   t.data(& orders::ord_locationto_uuid, "locationto");
   t.data(& orders::ord_locationfrom_uuid, "locationfrom");
   t.data(& orders::ord_itemuuid, "item");
   t.data(& orders::ord_created_at, "created_at");
   t.relationManyToOne(& orders::ord_myidsz, "ids_id");

}}
