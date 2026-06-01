#ifndef _QX_DIGITALBLOOMORDERS_LOGISTICS_H_
#define _QX_DIGITALBLOOMORDERS_LOGISTICS_H_

class orders;
class delivery;
class ids;

class QX_DIGITALBLOOMORDERS_DLL_EXPORT logistics : public qx::IxPersistable
{

QX_PERSISTABLE_HPP(logistics)
public:
// -- typedef


typedef std::shared_ptr<ids> logiz_ptr;


// -- properties
   QString     logi_id;
   QString     logi_deliverer;
   QString     logi_deliverer_location;
   QString     logi_transport_means;
   bool        logi_verified;
   QString     logi_deliverer_phonenumber;
   QString     logi_deliverer_avatar;
   QString     logi_deliverer_videoclip;
   QString     logi_deliverer_organisationorgroup;
   QString        logi_uuid;
   QDateTime   logi_created_at;
   logiz_ptr    logi_myidsz;

// -- contructor, virtual destructor
//   logistics() { ; }
 //  logistics() : qx::IxPersistable(), logi_id("0") { ; }
  logistics() : qx::IxPersistable() { ; }
   virtual ~logistics() { ; }
};

QX_REGISTER_PRIMARY_KEY(logistics, QString)
QX_REGISTER_HPP_QX_DIGITALBLOOMORDERS(logistics, qx::trait::no_base_class_defined, 0)

typedef std::shared_ptr<logistics> logistics_ptr;
//typedef QList<logistics_ptr> list_logistics;
//typedef std::vector<logistics_ptr> list_logistics;

typedef qx::QxCollection<QString, logistics_ptr> list_logistics;

#endif // _QX_DIGITALBLOOMORDERS_LOGISTICS_H_
