#ifndef _QX_DIGITALBLOOMORDERS_MYSECRETS_H_
#define _QX_DIGITALBLOOMORDERS_MYSECRETS_H_


class QX_DIGITALBLOOMORDERS_DLL_EXPORT mysecrets : public qx::IxPersistable
{

QX_PERSISTABLE_HPP(mysecrets)
public:




// -- variables

   QString     mysec_id;
   QString     mysec_username;
   QString     mysec_secretregister;
   QString     mysec_secretlogin;
   QDateTime   mysec_created_at;

   QDateTime datecreated() const;

// -- contructor, virtual destructor
//   mysecrets() { ; }
 //  mysecrets() : qx::IxPersistable(), mysec_id("0") { ; }
  mysecrets() : qx::IxPersistable() { ; }
   virtual ~mysecrets() { ; }
};

QX_REGISTER_PRIMARY_KEY(mysecrets, QString)
QX_REGISTER_HPP_QX_DIGITALBLOOMORDERS(mysecrets, qx::trait::no_base_class_defined, 0)

typedef std::shared_ptr<mysecrets> mysecrets_ptr;
//typedef QList<mysecrets_ptr> list_mysecrets;
typedef std::vector<mysecrets_ptr> list_mysecrets;

//typedef qx::QxCollection<QString, mysecrets_ptr> list_mysecrets;


#endif // _QX_DIGITALBLOOMORDERS_MYSECRETS_H_
