#include "precompiled.h"

#include "delivery.h"
#include "orders.h"
#include "ids.hpp"


QX_REGISTER_CPP_QX_DIGITALBLOOMORDERS(delivery)
QX_PERSISTABLE_CPP(delivery)

namespace qx {
template <> void register_class(QxClass<delivery> & t)
{
   t.id(& delivery::deli_id, "delivery_id");
   t.data(& delivery::deli_delivereruuid,"username");
   t.data(& delivery::deli_recipientuuid, "recipient" );
   t.data(& delivery::deli_itemuuid, "item");
   t.data(& delivery::deli_locationdelivereruuid, "locationdeliverer");
   t.data(& delivery::deli_locationrecipientuuid, "locationdeliverer");
   t.data(& delivery::deli_logisticsuuid, "logistics");
   t.data(& delivery::deli_smstxtsent, "smstextsent" );
   t.data(& delivery::deli_smstxtreceived, "smstxtreceived");
   t.data(& delivery::deli_order_uuid, "order_uuid");
   t.data(& delivery::deli_delivery_status, "delivery_status");
   t.data(& delivery::deli_sent_created_at, "sent_created_at");
   t.data(& delivery::deli_received_created_at, "received_created_at");
   t.relationManyToOne(& delivery::deli_myidsz, "ids_id");

}}
