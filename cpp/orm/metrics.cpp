#include "precompiled.h"
#include "ids.hpp"
#include "metrics.hpp"

QX_REGISTER_CPP_QX_DIGITALBLOOMORDERS(metrics)
QX_PERSISTABLE_CPP(metrics)

namespace qx {
template <> void register_class(QxClass<metrics> & t)
{
   t.id(& metrics::met_id, "metrics_id");
   t.data(& metrics::met_uuid, "uuid");
   t.data(& metrics::met_frequency, "frequency");
   t.data(& metrics::met_sessionid, "sessionuuid");
   t.data(& metrics::met_agent, "agent");
   t.data(& metrics::met_locationinfo, "locationinfo");
   t.data(& metrics::met_ipaddress, "ipaddress");
   t.data(& metrics::met_metadata, "metadata");
   t.data(& metrics::met_created_at, "created_at");
   t.relationManyToOne(& metrics::met_myidsmet, "ids_id");

}}
