#include "precompiled.h"
#include "ids.hpp"
#include "locations.h"
#include "orders.h"



QX_REGISTER_CPP_QX_DIGITALBLOOMORDERS(locations)
QX_PERSISTABLE_CPP(locations)

namespace qx {
template <> void register_class(QxClass<locations> & t)
{
   t.id(& locations::loc_id, "locations_id");
   t.data(& locations::loc_name, "name");
   t.data(& locations::loc_latitude, "latitude");
   t.data(& locations::loc_longitude, "longitude");
   t.data(& locations::loc_locationjsonmaps, "locationjsonmaps");
   t.data(& locations::loc_uuid, "uuid");
   t.data(& locations::loc_created_at, "created_at");
   t.relationManyToOne(& locations::loc_myidsloc, "ids_id");

}}


