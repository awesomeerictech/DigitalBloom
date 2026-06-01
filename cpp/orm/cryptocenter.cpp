#include "precompiled.h"

#include "cryptocenter.hpp"


QX_REGISTER_CPP_QX_DIGITALBLOOMORDERS(cryptocenter)
QX_PERSISTABLE_CPP(cryptocenter)

namespace qx {
template <> void register_class(QxClass<cryptocenter> & t)
{
   t.id(& cryptocenter::crypto_id, "cryptocenter_id");
   t.data(& cryptocenter::crypto_username, "username");
   t.data(& cryptocenter::crypto_responsibilities, "responsibilities");
   t.data(& cryptocenter::crypto_verification, "verification");
   t.data(& cryptocenter::crypto_account, "account");
   t.data(& cryptocenter::crypto_businesstype, "business");
   t.data(& cryptocenter::crypto_authoritytype,"authority");
   t.data(& cryptocenter::crypto_logisticstype,"logistics");
   t.data(& cryptocenter::crypto_adminnames, "adminnames");
   t.data(& cryptocenter::crypto_key,"key");
   t.data(& cryptocenter::crypto_iv, "iv");
   t.data(& cryptocenter::crypto_passhint, "hint");
   t.data(& cryptocenter::crypto_cipher, "cipher");
   t.data(& cryptocenter::token, "token");
   t.data(& cryptocenter::crypto_phone, "phone");
   t.data(& cryptocenter::crypto_email, "email");
   t.data(& cryptocenter::crypto_subscription, "subscription");
   t.data(& cryptocenter::crypto_blacklist, "blacklist");
   t.data(& cryptocenter::crypto_policy,"policy" );
   t.data(& cryptocenter::crypto_roles, "roles");
   t.data(& cryptocenter::crypto_created_at, "created_at");

}}
