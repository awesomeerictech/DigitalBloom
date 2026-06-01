#ifndef _QX_DIGITALBLOOMORDERS_CRYPTOCENTER_H_
#define _QX_DIGITALBLOOMORDERS_CRYPTOCENTER_H_


class QX_DIGITALBLOOMORDERS_DLL_EXPORT cryptocenter : public qx::IxPersistable
{

QX_PERSISTABLE_HPP(cryptocenter)
public:

// -- enums

    enum enum_roles { normaluser,guest,webmaster,admin,superpower };
    enum enum_responsibiities { nill,agent,marketer };
    enum enum_account { individual,logistics,businesses,authorities };
    enum enum_verification { waiting,valid,integrious,unethical,invalid };
    enum enum_policy { save,retain };
    enum enum_sub { trial,active,inactive };
    enum enum_blacklist { none,temporary,permanently };



// -- properties

   QString     crypto_id;
   QString     crypto_username;
   QString     crypto_key;
   QString     crypto_passhint;
   QString     crypto_iv;
   QString  crypto_adminnames;
   QString     crypto_cipher;
   QString     crypto_phone;
   QString     crypto_email;
   long long     token;
   enum_sub    crypto_subscription;
   enum_blacklist     crypto_blacklist;
   enum_policy crypto_policy;
   enum_roles  crypto_roles;
   enum_responsibiities  crypto_responsibilities;
   QString crypto_businesstype;
   QString crypto_authoritytype;
   QString crypto_logisticstype;
   enum_account  crypto_account;
   enum_verification  crypto_verification;
   QDateTime   crypto_created_at;

// -- contructor, virtual destructor
//   cryptocenter() { ; }
 //  cryptocenter() : qx::IxPersistable(), crypto_id("0") { ; }
  cryptocenter() : qx::IxPersistable() { ; }
   virtual ~cryptocenter() { ; }
};

QX_REGISTER_PRIMARY_KEY(cryptocenter, QString)
QX_REGISTER_HPP_QX_DIGITALBLOOMORDERS(cryptocenter, qx::trait::no_base_class_defined, 0)

typedef std::shared_ptr<cryptocenter> cryptocenter_ptr;
//typedef QList<cryptocenter_ptr> list_cryptocenter;
typedef std::vector<cryptocenter_ptr> list_cryptocenter;

//typedef qx::QxCollection<QString, cryptocenter_ptr> list_cryptocenter;


#endif // _QX_DIGITALBLOOMORDERS_CRYPTOCENTER_H_
