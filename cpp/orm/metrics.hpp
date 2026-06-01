#ifndef _QX_DIGITALBLOOMORDERS_METRICS_H_
#define _QX_DIGITALBLOOMORDERS_METRICS_H_

class ids;

class QX_DIGITALBLOOMORDERS_DLL_EXPORT metrics : public qx::IxPersistable
{

QX_PERSISTABLE_HPP(metrics)
public:

 // -- typedef

 typedef std::shared_ptr<ids> idsmet_ptr;

// -- properties
   QString     met_id;
   QString        met_uuid;
   QString       met_sessionid;
   long        met_frequency;
   QString     met_agent;
   QString     met_locationinfo;
   QString        met_ipaddress;
   QString  met_metadata;
   QDateTime   met_created_at;
   idsmet_ptr    met_myidsmet;

// -- contructor, virtual destructor
//   metrics() { ; }
 //  metrics() : qx::IxPersistable(), met_id("0") { ; }
  metrics() : qx::IxPersistable() { ; }
   virtual ~metrics() { ; }
};

QX_REGISTER_PRIMARY_KEY(metrics, QString)
QX_REGISTER_HPP_QX_DIGITALBLOOMORDERS(metrics, qx::trait::no_base_class_defined, 0)

typedef std::shared_ptr<metrics> metrics_ptr;
//typedef QList<metrics_ptr> list_metrics;
typedef std::vector<metrics_ptr> list_metrics;

//typedef qx::QxCollection<QString, metrics_ptr> list_metrics;


#endif // _QX_DIGITALBLOOMORDERS_METRICS_H_
