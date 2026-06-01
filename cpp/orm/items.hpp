#ifndef _QX_DIGITALBLOOMORDERS_ITEMS_H_
#define _QX_DIGITALBLOOMORDERS_ITEMS_H_


class ids;
class orders;
class delivery;




class QX_DIGITALBLOOMORDERS_DLL_EXPORT items : public qx::IxPersistable
{

QX_PERSISTABLE_HPP(items)
QX_REGISTER_FRIEND_CLASS(items)

public:
// -- properties
  // -- typedef

typedef std::shared_ptr<ids> idsy_ptr;





   QString        ite_id;
   QString        ite_username;
   QString           ite_itemprice;
   QString        ite_itemname;
   QString        ite_mediafile;
   QString        ite_itemcategory;
   QString        ite_itemsizeorquantity;
   QString            ite_standardized;
   QString        ite_standardizedavatar;
   QString          ite_iteer_uuid;
   QString        ite_itemmanufacturer;
   QString        ite_itemdescription;
   QDateTime      ite_created_at;
   idsy_ptr       ite_myidsz;


   static QJsonValue deletemediaite(const QJsonValue & request);


// -- contructor, virtual destructor
//   items() { ; }
// items() : qx::IxPersistable(), ite_id("0") { ; }
   items() : qx::IxPersistable() { ; }
   virtual ~items() { ; }
};

QX_REGISTER_PRIMARY_KEY(items, QString)
QX_REGISTER_HPP_QX_DIGITALBLOOMORDERS(items, qx::trait::no_base_class_defined, 0)

typedef std::shared_ptr<items> items_ptr;
//typedef std::vector<items_ptr> list_items;
typedef qx::QxCollection<QString, items_ptr> list_items;


#endif // _QX_DIGITALBLOOMORDERS_ITEMS_H_
