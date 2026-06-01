#ifndef _QX_DIGITALBLOOMORDERS_CATEGORIES_H_
#define _QX_DIGITALBLOOMORDERS_CATEGORIES_H_

class ids;
//class orders;
//class delivery;


class QX_DIGITALBLOOMORDERS_DLL_EXPORT categories : public qx::IxPersistable
{

QX_PERSISTABLE_HPP(categories)
public:

// -- typedef

typedef std::shared_ptr<ids> idsz_ptr;



// -- properties

   QString     cat_id;
   QString     cat_name;
   QString     cat_username;
   QDateTime   cat_created_at;
   idsz_ptr    cat_myidsz;

// -- contructor, virtual destructor
//   categories() { ; }
 //  categories() : qx::IxPersistable(), cat_id("0") { ; }

  categories() : qx::IxPersistable() { ; }
   virtual ~categories() { ; }

};

QX_REGISTER_PRIMARY_KEY(categories, QString)
QX_REGISTER_HPP_QX_DIGITALBLOOMORDERS(categories, qx::trait::no_base_class_defined, 0)

typedef std::shared_ptr<categories> categories_ptr;
//typedef QList<categories_ptr> list_categories;
typedef std::vector<categories_ptr> list_categories;

//typedef qx::QxCollection<QString, categories_ptr> list_categories;


#endif // _QX_DIGITALBLOOMORDERS_CATEGORIES_H_
