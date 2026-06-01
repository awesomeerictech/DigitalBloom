#include "precompiled.h"

#include "logistics.h"
#include "orders.h"
#include "ids.hpp"



QX_REGISTER_CPP_QX_DIGITALBLOOMORDERS(logistics)
QX_PERSISTABLE_CPP(logistics)

namespace qx {
template <> void register_class(QxClass<logistics> & t)
{
   t.id(& logistics::logi_id, "logistics_id");
   t.data(& logistics::logi_deliverer, "deliverer");
   t.data(& logistics::logi_deliverer_location, "deliverer_location");
   t.data(& logistics::logi_transport_means, "transport_means");
   t.data(& logistics::logi_verified, "verified");
   t.data(& logistics::logi_deliverer_phonenumber, "deliverer_phonenumber");
   t.data(& logistics::logi_deliverer_avatar, "deliverer_avatar");
   t.data(& logistics::logi_deliverer_videoclip, "deliverer_videoclip");
   t.data(& logistics::logi_deliverer_organisationorgroup, "deliverer_organisationorgroup");
   t.data(& logistics::logi_uuid, "deliverer_uuid");
   t.data(& logistics::logi_created_at, "created_at");
   t.relationManyToOne(& logistics::logi_myidsz, "ids_id");

}}
