#include "precompiled.h"

#include "mysecrets.h"




QX_REGISTER_CPP_QX_DIGITALBLOOMORDERS(mysecrets)
QX_PERSISTABLE_CPP(mysecrets)

namespace qx {
template <> void register_class(QxClass<mysecrets> & t)
{
   t.id(& mysecrets::mysec_id, "mysecrets_id");
   t.data(& mysecrets::mysec_secretregister, "secretregister");
   t.data(& mysecrets::mysec_secretlogin, "secretlogin");
   t.data(& mysecrets::mysec_username, "username");
   t.fct_0<QDateTime>(std::mem_fn(& mysecrets::datecreated), "created_at");

}}


QDateTime mysecrets::datecreated() const
{
    QDateTime local = QDateTime::currentDateTime();
    return local;
}
