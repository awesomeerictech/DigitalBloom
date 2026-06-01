#ifndef _QX_DIGITALBLOOMORDERS_LOCATIONS_H_
#define _QX_DIGITALBLOOMORDERS_LOCATIONS_H_
#include "delivery.h"

class orders;
class ids;

class QX_DIGITALBLOOMORDERS_DLL_EXPORT locations : public qx::IxPersistable
{

QX_PERSISTABLE_HPP(locations)

public:
// -- typedef

  typedef std::shared_ptr<ids> idsloc_ptr;

// -- properties
   QString     loc_id;
   QString     loc_name;
   QString     loc_latitude;
   QString     loc_longitude;
   QString  loc_locationjsonmaps;
   QString        loc_uuid;
   QDateTime   loc_created_at;
   idsloc_ptr    loc_myidsloc;


// -- contructor, virtual destructor
 //  locations() :  { ; }
 //  locations() : qx::IxPersistable(), loc_id("0") { ; }
   locations() : qx::IxPersistable() { ; }
   virtual ~locations() { ; }

};

QX_REGISTER_PRIMARY_KEY(locations, QString)
QX_REGISTER_HPP_QX_DIGITALBLOOMORDERS(locations, qx::trait::no_base_class_defined, 0)

typedef std::shared_ptr<locations> locations_ptr;
typedef qx::QxCollection<QString, locations_ptr> list_locations;

#endif // _QX_DIGITALBLOOMORDERS_LOCATIONS_H_
