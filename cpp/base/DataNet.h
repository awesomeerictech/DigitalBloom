#ifndef DATANET_H
#define DATANET_H





#include "JsonUtils.h"
#include "views/Module.h"
#include "views/MainTemplate.h"
#include "views/AppLinks.h"
#include "views/AppRoles.h"
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv.hpp>
#include <json/json.h>
#include "CsvUtil.h"
#include "sqliteone.hpp"
#include "tools/NotificationManager.h"




class MapperSoftware;
class NotificationManager;



class DataNet :public Wt::WObject
{

public:

DataNet(MapperSoftware *mappersoftware=nullptr,Wt::Http::Client *client=nullptr,std::shared_ptr<MyJsonTools> mypayload_=nullptr ,std::shared_ptr<eric::JsonUtils> myjsonutils_=nullptr);
~DataNet();
  std::wstring myurldue(AppLinks links);
  std::wstring myroledue(AppRoles roles);
  void uservalidity(QVariantMap info);
  void categoryexistence(QVariantMap info);
  void itemexistence(QVariantMap info);
  void loginuser(QVariantMap info);
  void registeruser(QVariantMap info);
  void retrievepassword(QVariantMap info);
  void newcategory(QVariantMap info);
  void newitem(QVariantMap info);
  void queryitems(QVariantMap info);
  void userbyid(QVariantMap info);
  void userdata(QVariantMap info);
  void usersubscription(QVariantMap info);
  void updateitem(QVariantMap info);
  void deleteitem(QVariantMap info);
  void deletecategory(QVariantMap info);
  void newrequest(QVariantMap info);
  void updaterequest(QVariantMap info);
  void deleterequest(QVariantMap info);
  void queryrequest(QVariantMap info);
  void deleteitemcategory(QVariantMap info);
  void fetchspecific(QVariantMap info);
  void logout(QVariantMap info);
  void passwordupdate(QVariantMap info);
  void phonenumberupdate(QVariantMap info);
  void getallitems();
  void getuseritems(QString user);
  void starttimer1();
  void starttimer2();
  void starttimer3();
  void starttimer4();
  Wt::Signal<std::string>& myrestdata() { return myrestdata_; }
  Wt::Signal<std::string>& myrestnetworkError() { return networkErrormyrest_; }
  Wt::Signal<std::string>& myrestserverError() { return serverErrormyrest_; }
  Wt::Signal<bool>& timer1() { return mytimer1_; }
  Wt::Signal<bool>& timer2() { return mytimer2_; }
  Wt::Signal<bool>& timer3() { return mytimer3_; }
  Wt::Signal<std::string> myrestdata_;
  Wt::Signal<std::string> networkErrormyrest_;
  Wt::Signal<std::string> serverErrormyrest_;
  Wt::Signal<bool> mytimer1_;
  Wt::Signal<bool> mytimer2_;
  Wt::Signal<bool> mytimer3_;
  Wt::Signal<std::string>& usernameresourceready() { return usernameresource_; }
  std::string fromIstream(std::istream &stream);
  Wt::Http::Client *client_;
  void requestDone(Wt::AsioWrapper::error_code ec, const Wt::Http::Message &msg);
  void requestDoneWithin(Wt::AsioWrapper::error_code ec, const Wt::Http::Message &msg);
  void mybodyDataReceived(std::string bodydata);
  std::string combimsg ="";
  bool start = true;
  NotificationManager *m_notificationManager = nullptr;
  static DataNet *getInstance();
  static DataNet *instance;






protected:



private:


  std::shared_ptr<MyJsonTools> mypayload;
  std::shared_ptr<eric::JsonUtils> myjsonutils;
  Wt::Signal<std::string> usernameresource_;
  MapperSoftware *mappersoftware_;




};

//Q_DECLARE_METATYPE(DataNet)



#endif // DATANET_H
