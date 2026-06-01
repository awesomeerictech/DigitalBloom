#include "precompiled.h"
#include "ids.hpp"
#include "categories.hpp"
#include "orders.h"
#include "delivery.h"




QX_REGISTER_CPP_QX_DIGITALBLOOMORDERS(categories)
QX_PERSISTABLE_CPP(categories)

namespace qx {
template <> void register_class(QxClass<categories> & t)
{
   t.id(& categories::cat_id, "categories_id");
   t.data(& categories::cat_name, "name");
   t.data(& categories::cat_username, "username");
   t.data(& categories::cat_created_at, "created_at");
   t.relationManyToOne(& categories::cat_myidsz, "ids_id");

}}
