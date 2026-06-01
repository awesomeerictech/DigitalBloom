#ifndef _QX_DIGITALBLOOMORDERS_IDS_H_
#define _QX_DIGITALBLOOMORDERS_IDS_H_

class categories;
class items;
class orders;
class delivery;
class logistics;
class locations;
class metrics;






class QX_DIGITALBLOOMORDERS_DLL_EXPORT ids : public qx::IxPersistable
{

QX_PERSISTABLE_HPP(ids)
public:
// -- typedef

    typedef std::shared_ptr<categories> catx_ptr;
    typedef std::vector<catx_ptr> list_catx;

    typedef std::shared_ptr<items> itemsx_ptr;
    typedef std::vector<itemsx_ptr> list_itemsx;

    typedef std::shared_ptr<orders> ordersx_ptr;
    typedef std::vector<ordersx_ptr> list_ordersx;

    typedef std::shared_ptr<delivery> deliveryx_ptr;
    typedef std::vector<deliveryx_ptr> list_deliveryx;

    typedef std::shared_ptr<logistics> logisticsx_ptr;
    typedef std::vector<logisticsx_ptr> list_logisticsx;

    typedef std::shared_ptr<locations> locationsx_ptr;
    typedef std::vector<locationsx_ptr> list_locationsx;

    typedef std::shared_ptr<metrics> metricsx_ptr;
    typedef std::vector<metricsx_ptr> list_metricsx;

// -- enums

enum enum_roles { normaluser,guest,webmaster,admin,superpower };
enum enum_responsibiities { nill,agent,marketer };
enum enum_verification { waiting,valid,integrious,unethical,invalid };
enum enum_policy { save,retain };
enum enum_account { individual,logistics,businesses,authorities };
enum enum_sub { trial,active,inactive };
enum enum_blacklist { none,temporary,permanently };


   // -- properties

      QString     myids_id;
      QString     myids_username;
      QString     myids_phone;
      QString     myids_avatarname;
      QString     myids_idcardfront;
      QString     myids_idcardback;
      QString     myids_adname;
      QString     myids_email;
      QString     myids_mainurl;
      QString     myids_websiteurl;
      QString     myids_helpurl;
      QString  myids_adminnames;
      enum_sub    myids_subscription;
      enum_blacklist     myids_blacklist;
      enum_policy myids_policy;
      enum_roles  myids_roles;
      enum_responsibiities  myids_responsibilities;
      enum_verification  myids_verification;
      QString myids_businesstype;
      QString myids_authoritytype;
      QString myids_logisticstype;
      enum_account  myids_account;
      QDateTime   myids_created_at;
      list_catx   myids_list_catx;
      list_itemsx  myids_list_itemsx;
      list_ordersx  myids_list_ordersx;
      list_deliveryx  myids_list_deliveryx;
      list_logisticsx  myids_list_logisticsx;
      list_locationsx  myids_list_locationsx;
      list_metricsx  myids_list_metricsx;

     static QJsonValue deletemediaids(const QJsonValue & request);

// -- contructor, virtual destructor
//   ids() { ; }
 //  ids() : qx::IxPersistable(), myids_id("0") { ; }
  ids() : qx::IxPersistable() { ; }
   virtual ~ids() { ; }
};

QX_REGISTER_PRIMARY_KEY(ids, QString)
QX_REGISTER_HPP_QX_DIGITALBLOOMORDERS(ids, qx::trait::no_base_class_defined, 0)

typedef std::shared_ptr<ids> ids_ptr;
//typedef QList<ids_ptr> list_ids;
//typedef std::vector<ids_ptr> list_ids;

typedef qx::QxCollection<QString, ids_ptr> list_ids;


#endif // _QX_DIGITALBLOOMORDERS_IDS_H_
