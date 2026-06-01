#include "DataNet.h"
#include "MapperSoftware.h"

#include "orderserver/QmlBackend.h"




extern QmlBackend globalBackendHandler;


DataNet *DataNet::instance = nullptr;

DataNet *DataNet::getInstance()
{
    if (instance == nullptr)
    {
        instance = new DataNet();
    }

    return instance;
}



DataNet::DataNet(MapperSoftware *mappersoftware,Wt::Http::Client *client,std::shared_ptr<MyJsonTools> mypayload_,std::shared_ptr<eric::JsonUtils> myjsonutils_)
: mappersoftware_(mappersoftware),client_(client),mypayload(mypayload_),myjsonutils(myjsonutils_)
{

 m_notificationManager = NotificationManager::getInstance();

}


DataNet::~DataNet() {


}



void DataNet::requestDone(Wt::AsioWrapper::error_code ec, const Wt::Http::Message &msg)
{
  if (!ec) {
  /*  Wt::WStringStream ss;
    ss << "Status code " << msg.status() << "\n\n"
       << msg.body(); */
    //qDebug() <<"Status Code is: " << msg.status() << Qt::endl;
    //qDebug() << "Message Body is: " << QString::fromStdString(msg.body()) << Qt::endl;
    //qDebug() << "Results are: " << QString::fromStdString(Wt::utf8(ss.str()).toUTF8()) << Qt::endl;
      if(msg.status()==200) {

      myrestdata_.emit(msg.body());

      }

      else {

      serverErrormyrest_.emit(msg.body());


      }

  } else {

     qDebug() << "Errors are: " << QString::fromStdString(Wt::utf8("{1}").arg(ec.message()).toUTF8()) << Qt::endl;
     networkErrormyrest_.emit(Wt::utf8("{1}").arg(ec.message()).toUTF8());
  }


  mappersoftware_->triggerUpdate();
  mappersoftware_->enableUpdates(false);
}




void DataNet::requestDoneWithin(Wt::AsioWrapper::error_code ec, const Wt::Http::Message &msg)
{
  if (!ec) {


  } else {

   //  qDebug() <<"Status Code is: " << msg.status() << Qt::endl;
     std::string errorsme = Wt::utf8("{1}").arg(ec.message()).toUTF8();
     if(msg.status()==200 && errorsme=="stream truncated") {



     }

     else if(errorsme=="Connection refused" && msg.status()==-1 ) {

  //  qDebug() << "Errors are: " << QString::fromStdString(Wt::utf8("{1}").arg(ec.message()).toUTF8()) << Qt::endl;
    networkErrormyrest_.emit(Wt::utf8("{1}").arg(ec.message()).toUTF8());

     }

     else {

     serverErrormyrest_.emit(msg.body());


     }


  }


  mappersoftware_->triggerUpdate();
  mappersoftware_->enableUpdates(false);
}


void DataNet::mybodyDataReceived(std::string bodydata) {


// qDebug() << "Message Body mybodyDataReceived is: " << QString::fromStdString(bodydata) << Qt::endl;

 if(start) {

 combimsg = bodydata+combimsg;
 start=false;

 }

 else {

 combimsg = combimsg+bodydata;

 }



 if(mypayload->parseRequest(QString::fromStdString(combimsg))) {

 // qDebug() << "Json is well constructed. " << Qt::endl;
 // qDebug() << "Message Body combimsg is: " << QString::fromStdString(combimsg) << Qt::endl;
  myrestdata_.emit(combimsg);


 }

 else {

//qDebug() << "Json is being constructed! " << Qt::endl;

 }






}






std::string DataNet::fromIstream(std::istream &stream) {

 std::istreambuf_iterator<char> eos;
 return std::string(std::istreambuf_iterator<char>(stream), eos);

 }



void DataNet::starttimer1() {


return;

}

void DataNet::starttimer2() {

return;

}

void DataNet::starttimer3() {

return;

}

void DataNet::starttimer4() {

return;

}

std::wstring DataNet::myurldue(AppLinks links) {

AppLinks mylinkapi = (AppLinks) links;
std::wstring dat = RandomLinks(mylinkapi);
return dat;


}

std::wstring DataNet::myroledue(AppRoles roles) {

AppRoles mylinkrole = (AppRoles) roles;
std::wstring dat = RandomRole(mylinkrole);
return dat;


}


void  DataNet::uservalidity(QVariantMap info) {

combimsg ="";
//client_->abort();

std::string userdata = R"(
{
    "action": "fetch_by_query",
    "entity": "cryptocenter",
    "data": {
      "username": "new1"
    },
    "query": {
        "sql": "username = 'ericm' AND token = 18435055"
    },
    "request_id": "c8e42836-b04b-40dc-81da-919bbd00d5f8"
}
)";

QString username = info.value("user").toString();
QString token = "";
std::string tokenfile;
tokenfile = mypayload->TOKEN_DIRIDS;
tokenfile.append(username.toStdString());
tokenfile.append("/reason.txt");

/*

std::string idsfile;
idsfile = mypayload->TOKEN_DIRIDS;
idsfile.append(username.toStdString());
idsfile.append("/ids.txt");

*/

if(std::ifstream is{tokenfile, std::ios_base::in}) {

    auto size = is.tellg();
    char * str;
    std::string content = mypayload->fromIstream(is.read(str,size));
    token = QString::fromStdString(content);
     // construct string to stream size
    //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
     is.close();

  }

long long token1 = token.toLongLong();

QMap<QString, QVariant> params;
params["username"] = username;
params["token"]    = token1;

QSet<QString> numericKeys;
numericKeys << "token";

QString whereClause = buildWhereClause(params, numericKeys);
// Example: "username = 'bob' AND token = 12345"

QString strPayload = QString::fromStdString(userdata);
QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
mypayload->modifyJsonValue(userdoc, "query.sql",  whereClause);
mypayload->modifyJsonValue(userdoc, "data.username", username);





}

void  DataNet::categoryexistence(QVariantMap info) {

combimsg ="";
//client_->abort();

std::string userdata = R"(

{
    "action": "fetch_by_query",
    "entity": "categories",
    "data": {
      "username": "new1"
    },
    "query": {
        "sql": "name = 'cattestexist1' AND username = 'ericm'"
    },
    "request_id": "ecf0b632-a0a3-44d0-adc7-446923159153"
}

)";


QString username = info.value("user").toString();
QString name = info.value("name").toString();
QString token = "";
std::string tokenfile;
tokenfile = mypayload->TOKEN_DIRIDS;
tokenfile.append(username.toStdString());
tokenfile.append("/reason.txt");

/*

std::string idsfile;
idsfile = mypayload->TOKEN_DIRIDS;
idsfile.append(username.toStdString());
idsfile.append("/ids.txt");

*/

if(std::ifstream is{tokenfile, std::ios_base::in}) {

    auto size = is.tellg();
    char * str;
    std::string content = mypayload->fromIstream(is.read(str,size));
    token = QString::fromStdString(content);
     // construct string to stream size
     //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
     is.close();

  }


  QMap<QString, QVariant> params;
  params["name"]     = name;
  params["username"] = username;

  //QString whereClause = buildWhereClause(params);


  QString whereClause = buildWhereClause(params,QSet<QString>(),"categories");

  // Example:  "name = 'eric' AND username = 'admin'"

QString strPayload = QString::fromStdString(userdata);
QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
mypayload->modifyJsonValue(userdoc, "query.sql", whereClause);
mypayload->modifyJsonValue(userdoc, "data.username", username);
const QString usagejson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);

std::string myapiurl = QString::fromStdWString(myurldue(APIURL)).toStdString();
Wt::Http::Message message;
client_->setSslCertificateVerificationEnabled(false);
message.addBodyText(usagejson.toStdString());

if (client_->post(myapiurl+"/deliva/universal/basicupdates"+"?reason="+token.toStdString(),message)) {

   Wt::WApplication::instance()->enableUpdates(true);

    }

else {


   }




}

void  DataNet::itemexistence(QVariantMap info) {

combimsg ="";
//client_->abort();

std::string jsondata = R"(

{
    "action": "fetch_by_query",
    "entity": "items",
    "data": {
      "username": "new1"
    },
    "query": {
        "sql": "name = 'itemtestexist' AND username = 'ericm'"
    },
    "request_id": "c8e42836-b04b-40dc-81da-919bbd00d5f8"
}

)";


std::string userdata = R"(

{
    "action": "fetch_by_query",
    "entity": "items",
    "data": {
      "username": "new1"
    },
    "query": {
        "sql": "name = 'itemtestexist' AND username = 'ericm'"
    },
    "request_id": "c8e42836-b04b-40dc-81da-919bbd00d5f8"
}

)";


QString username = info.value("user").toString();
QString name = info.value("name").toString();
QString itemuuid = info.value("itemsid").toString();
QString token = "";

qInfo() << "itemuuid: " << itemuuid << Qt::endl;
qInfo() << "name: " << name << Qt::endl;
qInfo() << "username: " << username << Qt::endl;

std::string tokenfile;
tokenfile = mypayload->TOKEN_DIRIDS;
tokenfile.append(username.toStdString());
tokenfile.append("/reason.txt");

/*

std::string idsfile;
idsfile = mypayload->TOKEN_DIRIDS;
idsfile.append(username.toStdString());
idsfile.append("/ids.txt");

*/

if(std::ifstream is{tokenfile, std::ios_base::in}) {

    auto size = is.tellg();
    char * str;
    std::string content = mypayload->fromIstream(is.read(str,size));
    token = QString::fromStdString(content);
     // construct string to stream size
     //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
     is.close();

  }


  QString usagequery = "";
  QString strPayload = QString::fromStdString(userdata);
  QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());

  mypayload->modifyJsonValue(userdoc, "data.username", username);

  if (itemuuid == "empty") {

      // Build SQLite WHERE: name = '...' AND username = '...'
      QMap<QString, QVariant> params;
      params["name"]     = name;
      params["username"] = username;

      usagequery = buildWhereClause(params,QSet<QString>(),"items");
      // e.g. "name = 'eric' AND username = 'admin'"

      qInfo() << "usagequery: " << usagequery << Qt::endl;


      mypayload->modifyJsonValue(userdoc, "query.sql", usagequery);
  }
  else {

      // Build SQLite WHERE: ids_id = '...' AND username = '...'
      QMap<QString, QVariant> params;
      params["items_id"]      = itemuuid;
      params["username"] = username;

      usagequery  = buildWhereClause(params,QSet<QString>(),"items");
      // e.g. "items_id = '3fa85f64' AND username = 'admin'"

      mypayload->modifyJsonValue(userdoc, "query.sql", usagequery);

      //mypayload->modifyJsonValue(userdoc, "data.items_id", itemuuid);
      //mypayload->modifyJsonValue(userdoc, "action", QJsonValue("fetch_by_id"));
  }






const QString usagejson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);

qInfo() << "usagejson itemexistence is: " << qPrintable(usagejson) << Qt::endl;
std::string myapiurl = QString::fromStdWString(myurldue(APIURL)).toStdString();
Wt::Http::Message message;
client_->setSslCertificateVerificationEnabled(false);
message.addBodyText(usagejson.toStdString());

if (client_->post(myapiurl+"/deliva/universal/basicupdates"+"?reason="+token.toStdString(),message)) {

   Wt::WApplication::instance()->enableUpdates(true);

    }

else {


   }




}


void DataNet::loginuser(QVariantMap info) {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";

std::string userdata = R"(

{
   "request_id": "7fcae1bd-f256-43f8-b67b-861b36fefd5e",
   "action": "update",
   "entity": "ids",
   "data": {
      "username": "new1",
      "key": "policydictates",
      "password": "11nill66",
      "subscription": 2,
      "created_at": "2022-09-25T15:00:54.740Z"
   },
   "columns": [
      "subscription"
   ],
   "query": {
      "sql": "username = 'new1'"
   }
}

)";


std::string jwtdata = R"(

{
   "token": "mytoken"
}


)";

QString strPayload = QString::fromStdString(userdata);
QString jwtPayload = QString::fromStdString(jwtdata);
QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
QString myrequestid = mypayload->newuuid();
QString username = info.value("user").toString();
QString password = info.value("pass").toString();
//QString reason = info.value("reason").toString();
//QString key = info.value("key").toString();
QString datedue = mypayload->utctime();
//qDebug() << "utc datedue is: " << datedue << Qt::endl;
mypayload->modifyJsonValue(userdoc, "request_id", myrequestid);
mypayload->modifyJsonValue(userdoc, "data.username", username);
mypayload->modifyJsonValue(userdoc, "data.password", password);
mypayload->modifyJsonValue(userdoc, "data.created_at", datedue);
//qDebug() << "new userdoc is: " << qPrintable(userdoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
QJsonDocument jwtdoc = QJsonDocument::fromJson(jwtPayload.toUtf8());
QString tokenlogin = QString::fromStdWString(RandomLinks(LOGINTOKEN));
QString jwt = mypayload->jwttoken(userdoc.object().toVariantMap(),tokenlogin);
//qDebug() << "jwt payload is: " << jwt << Qt::endl;
mypayload->modifyJsonValue(jwtdoc, "token", jwt);
const QString usagejson = jwtdoc.toJson(QJsonDocument::JsonFormat::Indented);
std::string myapiurl = QString::fromStdWString(myurldue(APIURL)).toStdString();

Wt::Http::Message message;
client_->setSslCertificateVerificationEnabled(false);
message.addBodyText(usagejson.toStdString());

if (client_->post(myapiurl+"/deliva/ids/login"+"?reason="+username.toStdString(),message)) {

   mappersoftware_->enableUpdates(true);

    }

else {


   }






return;




}


void DataNet::registeruser(QVariantMap info) {
combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";

std::string userdata = R"(

{
   "request_id": "b64859e7-7b59-499a-bc37-35c141dcdff3",
   "action": "insert",
   "entity": "ids",
   "query": {
      "sql": "username = 'null'"
   },
   "data": {
      "ids_id": "a618adeb-afab-48fc-a027-faa89bada08b",
      "cryptocenter_id": "a618adeb-afab-48fc-a027-faa89bada08b",
      "username": "new2",
      "agentid": "8db3d546-c0b6-4770-bf24-ecfe9e668b9c",
      "password": "11nill66",
      "hint": "new",
      "phone": "0742954736",
      "email": "later",
      "uuid": "8db3d546-c0b6-4770-bf24-ecfe9e668b9c",
      "avatarname": "later",
      "idfront": "later",
      "idback": "later",
      "adminnames": "later",
      "adname": "later",
      "key": "later",
      "iv": "later",
      "cipher": "later",
      "token": "later",
      "mainurl": "later",
      "websiteurl": "later",
      "helpurl": "later",
      "subscription": 1,
      "responsibilities": 1,
      "verification": 1,
      "blacklist": 1,
      "account": 1,
      "business": "later",
      "authority": "later",
      "logistics": "later",
      "policy": 0,
      "roles": 1,
      "created_at": "2022-09-25T14:46:44.909Z"
   }
}

)";



std::string jwtdata = R"(

{
   "token": "mytoken"
}


)";

QString strPayload = QString::fromStdString(userdata);
QString jwtPayload = QString::fromStdString(jwtdata);
QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
QString myrequestid = mypayload->newuuid();
QString myid = mypayload->newuuid();
QString myids = mypayload->newuuid();
QString username = info.value("user").toString();
QString password = info.value("pass").toString();
QString hint = info.value("hint").toString();
QString phone = info.value("phone").toString();
QString email = info.value("email").toString();
//QString reason = info.value("reason").toString();
//QString key = info.value("key").toString();
QString datedue = mypayload->utctime();
//qDebug() << "utc datedue is: " << datedue << Qt::endl;
mypayload->modifyJsonValue(userdoc, "request_id", myrequestid);
mypayload->modifyJsonValue(userdoc, "data.username", username);
mypayload->modifyJsonValue(userdoc, "data.password", password);
mypayload->modifyJsonValue(userdoc, "data.ids_id", myid);
mypayload->modifyJsonValue(userdoc, "data.cryptocenter_id", myid);
mypayload->modifyJsonValue(userdoc, "data.agentid", myids);
mypayload->modifyJsonValue(userdoc, "data.hint", hint);
mypayload->modifyJsonValue(userdoc, "data.phone", phone);
mypayload->modifyJsonValue(userdoc, "data.email", email);
mypayload->modifyJsonValue(userdoc, "data.uuid", myids);
mypayload->modifyJsonValue(userdoc, "data.created_at", datedue);
//qDebug() << "new userdoc is: " << qPrintable(userdoc.toJson(QJsonDocument::JsonFormat::Indented)) << Qt::endl;
QJsonDocument jwtdoc = QJsonDocument::fromJson(jwtPayload.toUtf8());
QString tokenlogin = QString::fromStdWString(RandomLinks(REGISTERTOKEN));
QString jwt = mypayload->jwttoken(userdoc.object().toVariantMap(),tokenlogin);
//qDebug() << "jwt payload is: " << jwt << Qt::endl;
mypayload->modifyJsonValue(jwtdoc, "token", jwt);
const QString usagejson = jwtdoc.toJson(QJsonDocument::JsonFormat::Indented);

std::string myapiurl = QString::fromStdWString(myurldue(APIURL)).toStdString();
Wt::Http::Message message;
client_->setSslCertificateVerificationEnabled(false);
message.addBodyText(usagejson.toStdString());

if (client_->post(myapiurl+"/deliva/ids/new"+"?reason=testapi",message)) {

   mappersoftware_->enableUpdates(true);

    }

else {



   }







return;


}




void DataNet::retrievepassword(QVariantMap info) {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";


std::string userdata = R"(

{
   "request_id": "ba5ad981-1239-48fe-bfd9-e96a745b3fa3",
   "action": "update",
   "entity": "ids",
   "relations": [
      "*"
   ],
   "data": {
      "username": "new6",
      "hint": "name",
      "key": "policydictates",
      "subscription": 2,
      "created_at": "2022-09-25T17:05:03.929Z"
   },
   "columns": [
      "subscription"
   ],
   "query": {
      "sql": "username = 'new6'"
   }
}

)";


QString strPayload = QString::fromStdString(userdata);
QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
QString myrequestid = mypayload->newuuid();
QString username = info.value("user").toString();
QString hint = info.value("hint").toString();
QString datedue = mypayload->utctime();
QString token = "";

std::string tokenfile;
tokenfile = mypayload->TOKEN_DIRIDS;
tokenfile.append(username.toStdString());
tokenfile.append("/reason.txt");

/*

std::string idsfile;
idsfile = mypayload->TOKEN_DIRIDS;
idsfile.append(username.toStdString());
idsfile.append("/ids.txt");

*/

if(std::ifstream is{tokenfile, std::ios_base::in}) {

    auto size = is.tellg();
    char * str;
    std::string content = mypayload->fromIstream(is.read(str,size));
    token = QString::fromStdString(content);
     // construct string to stream size
   //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
     is.close();

  }

mypayload->modifyJsonValue(userdoc, "request_id", myrequestid);
mypayload->modifyJsonValue(userdoc, "data.username", username);
mypayload->modifyJsonValue(userdoc, "data.hint", hint);
mypayload->modifyJsonValue(userdoc, "data.created_at", datedue);
const QString usagejson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);

std::string myapiurl = QString::fromStdWString(myurldue(APIURL)).toStdString();
Wt::Http::Message message;
client_->setSslCertificateVerificationEnabled(false);
message.addBodyText(usagejson.toStdString());

if (client_->post(myapiurl+"/deliva/ids/hint"+"?reason="+token.toStdString(),message)) {

   mappersoftware_->enableUpdates(true);

    }

else {


   }





return;


}




void DataNet::userdata(QVariantMap info) {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";



std::string userdata = R"(

{
   "action": "fetch_by_query",
   "entity": "ids",
   "relations": [
      "*"
   ],
   "data": {
      "token": "732417298",
      "username": "new1",
      "created_at": "2022-09-25T15:07:28.702Z"
   },
   "query": {
      "sql": "username = 'new1'"
   }
}

)";


return;


}

void DataNet::usersubscription(QVariantMap info) {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";

std::string userdata = R"(

{
   "request_id": "648ea97f-f610-4660-bfab-82426215c0e9",
   "action": "update",
   "entity": "ids",
   "data": {
      "ids_id": "48334cea-1e65-4332-80e1-54a600ffc102",
      "username": "new6",
      "subscription": 1,
      "auth": "adminqrmenulokomokosub",
      "created_at": "2022-09-25T15:22:28.982Z"
   },
   "columns": [
      "subscription"
   ],
   "query": {
      "sql": "username = 'new6'"
   }
}

)";



return;


}


void DataNet::newcategory(QVariantMap info) {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";

std::string userdata = R"(

{
   "request_id": "3f8b5140-488a-4aa2-819c-6865e5bd7cee",
   "action": "insert",
   "entity": "categories",
   "query": {
      "request_id": "a67d991c-d31b-4a06-a6bf-4837250f0ab0",
      "action": "fetch_by_query",
      "entity": "ids",
      "query": {
         "sql": "username = 'new6'"
      }
   },
   "data": {
      "categories_id": "e55e3707-9cda-44f8-952f-33c8b57af0bd",
      "ids_id": "48334cea-1e65-4332-80e1-54a600ffc102",
      "username": "new6",
      "name": "catnew61",
      "created_at": "2022-09-25T15:38:58.063Z"
   }
}

)";


QString strPayload = QString::fromStdString(userdata);
QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
QString categoriesid = mypayload->newuuid();
QString myrequestid = mypayload->newuuid();
QString username = info.value("user").toString();
QString name = info.value("name").toString();
QString ids_id = info.value("ids_id").toString();
QString datedue = mypayload->utctime();
QString token = "";


std::string tokenfile;
tokenfile = mypayload->TOKEN_DIRIDS;
tokenfile.append(username.toStdString());
tokenfile.append("/reason.txt");


if(std::ifstream is{tokenfile, std::ios_base::in}) {

    auto size = is.tellg();
    char * str;
    std::string content = mypayload->fromIstream(is.read(str,size));
    token = QString::fromStdString(content);
     // construct string to stream size
   //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
     is.close();

  }

mypayload->modifyJsonValue(userdoc, "request_id", myrequestid);
mypayload->modifyJsonValue(userdoc, "data.categories_id", categoriesid);
mypayload->modifyJsonValue(userdoc, "data.ids_id", ids_id);
mypayload->modifyJsonValue(userdoc, "data.username", username);
mypayload->modifyJsonValue(userdoc, "data.name", name);
mypayload->modifyJsonValue(userdoc, "data.created_at", datedue);
const QString usagejson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//qDebug()<<"Usage Json for new category is: " << qPrintable(usagejson) << Qt::endl;

std::string myapiurl = QString::fromStdWString(myurldue(APIURL)).toStdString();
Wt::Http::Message message;
client_->setSslCertificateVerificationEnabled(false);
message.addBodyText(usagejson.toStdString());

if(client_->post(myapiurl+"/deliva/category/new"+"?reason="+token.toStdString(),message)) {

   mappersoftware_->enableUpdates(true);

    }

else {


   }




return;




}


void DataNet::deletecategory(QVariantMap info) {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";

std::string userdata = R"(

{
   "request_id": "fa014c8b-1e67-4cb8-8de6-7f1124c01982",
   "action": "delete_by_id",
   "entity": "categories",
   "query": {
      "request_id": "95c97c2a-6d81-4590-ada5-4a88a67f5001",
      "action": "fetch_by_query",
      "entity": "ids",
      "data": {
         "media": [
            "value1",
            "value2",
            "value3"
         ]
      },
      "query": {
         "sql": "username = 'new6'"
      }
   },
   "data": {
      "categories_id": "a944688d-8733-4b98-aa2c-54787e78e1b6"
   }
}

)";


QString strPayload = QString::fromStdString(userdata);
QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
QString categoriesid = info.value("categoriesid").toString();
QString myrequestid = mypayload->newuuid();
QString username = info.value("user").toString();
QString token = "";


std::string tokenfile;
tokenfile = mypayload->TOKEN_DIRIDS;
tokenfile.append(username.toStdString());
tokenfile.append("/reason.txt");


if(std::ifstream is{tokenfile, std::ios_base::in}) {

    auto size = is.tellg();
    char * str;
    std::string content = mypayload->fromIstream(is.read(str,size));
    token = QString::fromStdString(content);
     // construct string to stream size
   //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
     is.close();

  }

mypayload->modifyJsonValue(userdoc, "request_id", myrequestid);
mypayload->modifyJsonValue(userdoc, "data.categories_id", categoriesid);
mypayload->modifyJsonValue(userdoc, "data.username", username);
const QString usagejson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//qDebug()<<"Usage Json for delete category is: " << qPrintable(usagejson) << Qt::endl;

std::string myapiurl = QString::fromStdWString(myurldue(APIURL)).toStdString();
Wt::Http::Message message;
client_->setSslCertificateVerificationEnabled(false);
message.addBodyText(usagejson.toStdString());

if(client_->post(myapiurl+"/deliva/universal/basicupdates"+"?reason="+token.toStdString(),message)) {

   mappersoftware_->enableUpdates(true);

    }

else {


   }






return;

}


void DataNet::newitem(QVariantMap info) {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";

std::string userdata = R"(

{
   "request_id": "bbc6da59-15a7-4564-b599-05fc4ae15ca3",
   "action": "insert",
   "entity": "items",
   "query": {
      "request_id": "3b7e975c-2b2b-4d5f-9e97-fb45b21ef3db",
      "action": "fetch_by_query",
      "entity": "ids",
      "query": {
         "sql": "username = 'new6'"
      }
   },
   "data": {
      "items_id": "a944688d-8733-4b98-aa2c-54787e78e1b6",
      "ids_id": "48334cea-1e65-4332-80e1-54a600ffc102",
      "username": "new6",
      "name": "itemnew61",
      "uuid": "91dafa3a-47a9-4164-8a87-8061bab8de9d",
      "category": "catnew61",
      "mediafile": "new625-09-202218-38-55Skincare(dark spots)-2500-item.jpg",
      "sizeorquantity": "2kg",
      "standardized": "later",
      "standardizedavatar": "later",
      "manufacturer": "later",
      "price": "3000sh",
      "description": "my new6 item",
      "created_at": "2022-09-25T15:39:00.428Z"
   }
}

)";



QString strPayload = QString::fromStdString(userdata);
QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
QString itemsid = mypayload->newuuid();
QString myrequestid = mypayload->newuuid();
QString username = info.value("user").toString();
QString name = info.value("name").toString();
QString category = info.value("category").toString();
QString ids_id = info.value("ids_id").toString();
QString mediafile = info.value("mediafile").toString();
QString price = info.value("price").toString();
QString description = info.value("description").toString();
QString sizeorquantity = info.value("sizeorquantity").toString();
QString uuid = info.value("uuid").toString();
QString standardized = info.value("standardized").toString();
QString standardizedavatar = info.value("standardizedavatar").toString();
QString manufacturer = info.value("manufacturer").toString();
QString datedue = mypayload->utctime();
QString token = "";


std::string tokenfile;
tokenfile = mypayload->TOKEN_DIRIDS;
tokenfile.append(username.toStdString());
tokenfile.append("/reason.txt");


if(std::ifstream is{tokenfile, std::ios_base::in}) {

    auto size = is.tellg();
    char * str;
    std::string content = mypayload->fromIstream(is.read(str,size));
    token = QString::fromStdString(content);
     // construct string to stream size
   //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
     is.close();

  }

mypayload->modifyJsonValue(userdoc, "request_id", myrequestid);
mypayload->modifyJsonValue(userdoc, "data.items_id", itemsid);
mypayload->modifyJsonValue(userdoc, "data.ids_id", ids_id);
mypayload->modifyJsonValue(userdoc, "data.username", username);
mypayload->modifyJsonValue(userdoc, "data.name", name);
mypayload->modifyJsonValue(userdoc, "data.uuid", uuid);
mypayload->modifyJsonValue(userdoc, "data.category", category);
mypayload->modifyJsonValue(userdoc, "data.mediafile", mediafile);
mypayload->modifyJsonValue(userdoc, "data.sizeorquantity", sizeorquantity);
mypayload->modifyJsonValue(userdoc, "data.standardized", standardized);
mypayload->modifyJsonValue(userdoc, "data.standardizedavatar", standardizedavatar);
mypayload->modifyJsonValue(userdoc, "data.manufacturer", manufacturer);
mypayload->modifyJsonValue(userdoc, "data.price", price);
mypayload->modifyJsonValue(userdoc, "data.description", description);
mypayload->modifyJsonValue(userdoc, "data.created_at", datedue);
const QString usagejson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//qDebug()<<"Usage Json for new item is: " << usagejson << Qt::endl;

std::string myapiurl = QString::fromStdWString(myurldue(APIURL)).toStdString();
Wt::Http::Message message;
client_->setSslCertificateVerificationEnabled(false);
message.addBodyText(usagejson.toStdString());

if (client_->post(myapiurl+"/deliva/universal/basicupdates"+"?reason="+token.toStdString(),message)) {

   mappersoftware_->enableUpdates(true);

    }

else {


   }




return;


}


void DataNet::updateitem(QVariantMap info) {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";

std::string userdata = R"(

{
   "request_id": "0aa55965-3c91-4fd0-860c-e8cef203a1c5",
   "action": "update",
   "entity": "items",
   "query": {
      "request_id": "3b7e975c-2b2b-4d5f-9e97-fb45b21ef3db",
      "action": "fetch_by_query",
      "entity": "ids",
      "query": {
         "sql": "username = 'new6'"
      }
   },
   "columns": [
      "category",
      "mediafile",
      "uuid",
      "standardized",
      "standardizedavatar",
      "manufacturer",
      "sizeorquantity",
      "price",
      "description",
      "name"
   ],
   "data": {
      "items_id": "a944688d-8733-4b98-aa2c-54787e78e1b6",
      "ids_id": "48334cea-1e65-4332-80e1-54a600ffc102",
      "username": "new6",
      "name": "itemnew61",
      "uuid": "91dafa3a-47a9-4164-8a87-8061bab8de9d",
      "category": "catnew61",
      "mediafile": "new625-09-202218-38-55Skincare(dark spots)-2500-item.jpg",
      "sizeorquantity": "2kg",
      "standardized": "later",
      "standardizedavatar": "later",
      "manufacturer": "later",
      "price": "3000sh",
      "description": "my new6 item"
   }
}

)";



QString strPayload = QString::fromStdString(userdata);
QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
QString itemsid = info.value("itemsid").toString();
QString myrequestid = mypayload->newuuid();
QString username = info.value("user").toString();
QString name = info.value("name").toString();
QString category = info.value("category").toString();
QString ids_id = info.value("ids_id").toString();
QString mediafile = info.value("mediafile").toString();
QString price = info.value("price").toString();
QString description = info.value("description").toString();
QString sizeorquantity = info.value("sizeorquantity").toString();
QString uuid = info.value("uuid").toString();
QString standardized = info.value("standardized").toString();
QString standardizedavatar = info.value("standardizedavatar").toString();
QString manufacturer = info.value("manufacturer").toString();
//QString datedue = mypayload->utctime();
QString token = "";


std::string tokenfile;
tokenfile = mypayload->TOKEN_DIRIDS;
tokenfile.append(username.toStdString());
tokenfile.append("/reason.txt");


if(std::ifstream is{tokenfile, std::ios_base::in}) {

    auto size = is.tellg();
    char * str;
    std::string content = mypayload->fromIstream(is.read(str,size));
    token = QString::fromStdString(content);
     // construct string to stream size
   qInfo() << "content is: " << QString::fromStdString(content) << Qt::endl;
     is.close();

  }

mypayload->modifyJsonValue(userdoc, "request_id", myrequestid);
mypayload->modifyJsonValue(userdoc, "data.items_id", itemsid);
mypayload->modifyJsonValue(userdoc, "data.ids_id", ids_id);
mypayload->modifyJsonValue(userdoc, "data.username", username);
mypayload->modifyJsonValue(userdoc, "data.name", name);
mypayload->modifyJsonValue(userdoc, "data.uuid", uuid);
mypayload->modifyJsonValue(userdoc, "data.category", category);
mypayload->modifyJsonValue(userdoc, "data.mediafile", mediafile);
mypayload->modifyJsonValue(userdoc, "data.sizeorquantity", sizeorquantity);
mypayload->modifyJsonValue(userdoc, "data.standardized", standardized);
mypayload->modifyJsonValue(userdoc, "data.standardizedavatar", standardizedavatar);
mypayload->modifyJsonValue(userdoc, "data.manufacturer", manufacturer);
mypayload->modifyJsonValue(userdoc, "data.price", price);
mypayload->modifyJsonValue(userdoc, "data.description", description);
//mypayload->modifyJsonValue(userdoc, "data.created_at", datedue);
const QString usagejson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//qDebug()<<"Usage Json for update item is: " << qPrintable(usagejson) << Qt::endl;

std::string myapiurl = QString::fromStdWString(myurldue(APIURL)).toStdString();
Wt::Http::Message message;
client_->setSslCertificateVerificationEnabled(false);
message.addBodyText(usagejson.toStdString());

if (client_->post(myapiurl+"/deliva/universal/basicupdates"+"?reason="+token.toStdString(),message)) {

   mappersoftware_->enableUpdates(true);

    }

else {


   }





return;

}

void DataNet::queryitems(QVariantMap info) {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";
std::string userdata = R"(

{
  "action": "fetch_by_query",
  "entity": "items",
  "data": {
    "username": "new1"
  },
  "query": {
    "sql": "deliverer = 'uuid1' AND recipient = 'uuid2'"
  },
  "request_id": "c8e42836-b04b-40dc-81da-919bbd00d5f8"
}

)";



QString strPayload = QString::fromStdString(userdata);
QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
QString username = info.value("user").toString();
QString itemid = info.value("itemid").toString();
QString myrequestid = mypayload->newuuid();
QString token = "";


std::string tokenfile;
tokenfile = mypayload->TOKEN_DIRIDS;
tokenfile.append(username.toStdString());
tokenfile.append("/reason.txt");


if(std::ifstream is{tokenfile, std::ios_base::in}) {

auto size = is.tellg();
char * str;
std::string content = mypayload->fromIstream(is.read(str,size));
token = QString::fromStdString(content);
// construct string to stream size
//qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
is.close();

}

QMap<QString, QVariant> params;
params["items_id"] = itemid;

QString usagequery = buildWhereClause(params,QSet<QString>(),"items");
// => "items_id = '12345'"

mypayload->modifyJsonValue(userdoc, "query.sql", usagequery);
mypayload->modifyJsonValue(userdoc, "request_id", myrequestid);
mypayload->modifyJsonValue(userdoc, "data.username", username);
const QString usagejson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//qDebug()<<"Usage Json for query items is: " << qPrintable(usagejson) << Qt::endl;
std::string myapiurl = QString::fromStdWString(myurldue(APIURL)).toStdString();
Wt::Http::Message message;
client_->setSslCertificateVerificationEnabled(false);
message.addBodyText(usagejson.toStdString());

if(client_->post(myapiurl+"/deliva/universal/basicupdates"+"?reason="+token.toStdString(),message)) {

mappersoftware_->enableUpdates(true);

}

else {


}



return;




}


void DataNet::userbyid(QVariantMap info) {


combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";

std::string userdata = R"(

{
  "request_id": "fa014c8b-1e67-4cb8-8de6-7f1124c01982",
  "action": "fetch_by_id",
  "entity": "ids",
  "query": {
    "sql": "deliverer = 'uuid1' AND recipient = 'uuid2'"
  },
  "data": {
    "ids_id": "a944688d-8733-4b98-aa2c-54787e78e1b6",
    "username": "new1"
  }
}

)";




QString strPayload = QString::fromStdString(userdata);
QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
QString myrequestid = mypayload->newuuid();
QString username = info.value("user").toString();
QString ids_id = info.value("ids_id").toString();
QString token = "";

//qDebug() << "standardized is: " << standardized << Qt::endl;
//qDebug() << "standardizedavatar is: " << standardizedavatar << Qt::endl;

std::string tokenfile;
tokenfile = mypayload->TOKEN_DIRIDS;
tokenfile.append(username.toStdString());
tokenfile.append("/reason.txt");


if(std::ifstream is{tokenfile, std::ios_base::in}) {

auto size = is.tellg();
char * str;
std::string content = mypayload->fromIstream(is.read(str,size));
token = QString::fromStdString(content);
// construct string to stream size
//qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
is.close();

}

mypayload->modifyJsonValue(userdoc, "request_id", myrequestid);
mypayload->modifyJsonValue(userdoc, "data.ids_id", ids_id);
mypayload->modifyJsonValue(userdoc, "data.username", username);
const QString usagejson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//qDebug()<<"Usage Json for userbyid is: " << qPrintable(usagejson) << Qt::endl;

std::string myapiurl = QString::fromStdWString(myurldue(APIURL)).toStdString();
Wt::Http::Message message;
client_->setSslCertificateVerificationEnabled(false);
message.addBodyText(usagejson.toStdString());

if (client_->post(myapiurl+"/deliva/universal/basicupdates"+"?reason="+token.toStdString(),message)) {

mappersoftware_->enableUpdates(true);

}

else {


}



return;


}

void DataNet::deleteitem(QVariantMap info) {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";

std::string userdata = R"(

{
   "request_id": "fa014c8b-1e67-4cb8-8de6-7f1124c01982",
   "field": "name",
   "avatarfield": "mediafile",
   "otherfield": "name",
   "action": "delete_by_id",
   "entity": "items",
   "deleteotherentity": "categories",
   "deleteotherfield": "category",
   "deleteother": false,
   "query": {
      "request_id": "95c97c2a-6d81-4590-ada5-4a88a67f5001",
      "action": "fetch_by_query",
      "entity": "ids",
      "data": {
         "media": [
            "value1",
            "value2",
            "value3"
         ]
      },
      "query": {
         "sql": "username = 'new6'"
      }
   },
   "info": {
      "items_id": "a944688d-8733-4b98-aa2c-54787e78e1b6",
      "ids_id": "48334cea-1e65-4332-80e1-54a600ffc102",
      "category": "catnew61",
      "name": "itemnew61",
      "username": "new6",
      "created_at": "2022-09-25T16:15:22.767Z"
   },
   "data": {
      "items_id": "a944688d-8733-4b98-aa2c-54787e78e1b6"
   }
}

)";



QString strPayload = QString::fromStdString(userdata);
QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
QString itemsid = info.value("itemsid").toString();
QString myrequestid = mypayload->newuuid();
QString username = info.value("user").toString();
QString name = info.value("name").toString();
QString category = info.value("category").toString();
QString ids_id = info.value("ids_id").toString();
QString datedue = mypayload->utctime();
QString token = "";

std::string tokenfile;
tokenfile = mypayload->TOKEN_DIRIDS;
tokenfile.append(username.toStdString());
tokenfile.append("/reason.txt");


if(std::ifstream is{tokenfile, std::ios_base::in}) {

    auto size = is.tellg();
    char * str;
    std::string content = mypayload->fromIstream(is.read(str,size));
    token = QString::fromStdString(content);
     // construct string to stream size
   //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
     is.close();

  }

mypayload->modifyJsonValue(userdoc, "request_id", myrequestid);
mypayload->modifyJsonValue(userdoc, "query.request_id", mypayload->newuuid());
mypayload->modifyJsonValue(userdoc, "data.items_id", itemsid);
mypayload->modifyJsonValue(userdoc, "info.items_id", itemsid);
mypayload->modifyJsonValue(userdoc, "info.ids_id", ids_id);
mypayload->modifyJsonValue(userdoc, "info.username", username);
mypayload->modifyJsonValue(userdoc, "info.name", name);
mypayload->modifyJsonValue(userdoc, "info.category", category);
mypayload->modifyJsonValue(userdoc, "info.created_at", datedue);
const QString usagejson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//qDebug()<<"Usage Json for delete item is: " << qPrintable(usagejson) << Qt::endl;

std::string myapiurl = QString::fromStdWString(myurldue(APIURL)).toStdString();
Wt::Http::Message message;
client_->setSslCertificateVerificationEnabled(false);
message.addBodyText(usagejson.toStdString());

if (client_->post(myapiurl+"/deliva/universal/otheroperations"+"?reason="+token.toStdString(),message)) {

   mappersoftware_->enableUpdates(true);

    }

else {


   }




return;


}


void DataNet::newrequest(QVariantMap info) {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";

std::string userdata = R"(

{
  "request_id": "bbc6da59-15a7-4564-b599-05fc4ae15ca3",
  "action": "insert",
  "entity": "orders",
  "query": {
    "request_id": "3b7e975c-2b2b-4d5f-9e97-fb45b21ef3db",
    "action": "fetch_by_query",
    "entity": "ids",
    "query": {
      "sql": "username = 'new6'"
    }
  },
  "data": {
    "orders_id": "a944688d-8733-4b98-aa2c-54787e78e1b6",
    "ids_id": "48334cea-1e65-4332-80e1-54a600ffc102",
    "username": "new6",
    "deliverer": "a1dafa3a-57a9-4164-8a87-9061bab8de7d",
    "recipient": "91dafa3a-47a9-4164-8a87-8061bab8de9d",
    "locationto": "later",
    "locationfrom": "later",
    "item": "82dafa3a-27a9-5164-9a87-1061bab4de6d",
    "created_at": "2022-09-25T15:39:00.428Z"
  }
}

)";




QString strPayload = QString::fromStdString(userdata);
QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
QString ordersid = mypayload->newuuid();
QString myrequestid = mypayload->newuuid();
QString username = info.value("user").toString();
QString deliverer = info.value("deliverer").toString();
QString recipient = info.value("recipient").toString();
QString ids_id = info.value("ids_id").toString();
QString locationto = info.value("locationto").toString();
QString locationfrom = info.value("locationfrom").toString();
QString item = info.value("item").toString();
QString datedue = mypayload->utctime();
QString token = "";

qInfo() << "Location to is: " << locationto << Qt::endl;
QString donotify = QString("A new order awaits your delivery: customer: %1, location: %2")
                       .arg(username)
                       .arg(locationto);

qInfo() << "Notification is: " << donotify << Qt::endl;

m_notificationManager->setNotification("New Order",donotify, 2,103);


globalBackendHandler.triggerInfoDialog(donotify);



std::string tokenfile;
tokenfile = mypayload->TOKEN_DIRIDS;
tokenfile.append(username.toStdString());
tokenfile.append("/reason.txt");


if(std::ifstream is{tokenfile, std::ios_base::in}) {

auto size = is.tellg();
char * str;
std::string content = mypayload->fromIstream(is.read(str,size));
token = QString::fromStdString(content);
// construct string to stream size
//qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
is.close();

}

mypayload->modifyJsonValue(userdoc, "request_id", myrequestid);
mypayload->modifyJsonValue(userdoc, "data.orders_id", ordersid);
mypayload->modifyJsonValue(userdoc, "data.ids_id", ids_id);
mypayload->modifyJsonValue(userdoc, "data.username", username);
mypayload->modifyJsonValue(userdoc, "data.deliverer", deliverer);
mypayload->modifyJsonValue(userdoc, "data.recipient", recipient);
mypayload->modifyJsonValue(userdoc, "data.locationto", locationto);
mypayload->modifyJsonValue(userdoc, "data.locationfrom", locationfrom);
mypayload->modifyJsonValue(userdoc, "data.item", item);
mypayload->modifyJsonValue(userdoc, "data.created_at", datedue);
const QString usagejson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//qDebug()<<"Usage Json for new request is: " << usagejson << Qt::endl;

std::string myapiurl = QString::fromStdWString(myurldue(APIURL)).toStdString();
Wt::Http::Message message;
client_->setSslCertificateVerificationEnabled(false);
message.addBodyText(usagejson.toStdString());

if (client_->post(myapiurl+"/deliva/universal/basicupdates"+"?reason="+token.toStdString(),message)) {

mappersoftware_->enableUpdates(true);

}

else {


}



return;

}

void DataNet::updaterequest(QVariantMap info) {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";



}

void DataNet::deleterequest(QVariantMap info) {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";

std::string userdata = R"(

{
  "action": "delete_by_query",
  "entity": "orders",
  "data": {
    "username": "new1"
  },
  "query": {
    "sql": "ids_id = 'uuid1' AND recipient = 'uuid2'"
  },
  "request_id": "c8e42836-b04b-40dc-81da-919bbd00d5f8"
}

)";



QString strPayload = QString::fromStdString(userdata);
QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
QString username = info.value("user").toString();
QString orderid = info.value("orderid").toString();
QString recipientid = info.value("recipientid").toString();
QString myrequestid = mypayload->newuuid();
QString token = "";


std::string tokenfile;
tokenfile = mypayload->TOKEN_DIRIDS;
tokenfile.append(username.toStdString());
tokenfile.append("/reason.txt");


if(std::ifstream is{tokenfile, std::ios_base::in}) {

    auto size = is.tellg();
    char * str;
    std::string content = mypayload->fromIstream(is.read(str,size));
    token = QString::fromStdString(content);
     // construct string to stream size
   //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
     is.close();

  }

QMap<QString, QVariant> params;
params["orders_id"]      = orderid;
params["recipient"] = recipientid;

QString usagequery = buildWhereClause(params,QSet<QString>(),"orders");
// => "ids_id = 'order123' AND recipient = 'user456'"

mypayload->modifyJsonValue(userdoc, "query.sql", usagequery);
mypayload->modifyJsonValue(userdoc, "request_id", myrequestid);
mypayload->modifyJsonValue(userdoc, "data.username", username);
const QString usagejson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//qDebug()<<"Usage Json for delete orders is: " << qPrintable(usagejson) << Qt::endl;

std::string myapiurl = QString::fromStdWString(myurldue(APIURL)).toStdString();
Wt::Http::Message message;
client_->setSslCertificateVerificationEnabled(false);
message.addBodyText(usagejson.toStdString());

if(client_->post(myapiurl+"/deliva/universal/basicupdates"+"?reason="+token.toStdString(),message)) {

   mappersoftware_->enableUpdates(true);

    }

else {


   }






return;



}

void DataNet::queryrequest(QVariantMap info) {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";
std::string userdata = R"(

{
   "action": "fetch_by_query",
   "entity": "orders",
   "data": {
      "username": "new1"
   },
   "query": {
      "sql": "deliverer = 'uuid1' AND recipient = 'uuid2'"
   },
   "request_id": "c8e42836-b04b-40dc-81da-919bbd00d5f8"
}

)";



QString strPayload = QString::fromStdString(userdata);
QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
QString username = info.value("user").toString();
QString delivererid = info.value("delivererid").toString();
QString recipientid = info.value("recipientid").toString();
QString myrequestid = mypayload->newuuid();
QString token = "";


std::string tokenfile;
tokenfile = mypayload->TOKEN_DIRIDS;
tokenfile.append(username.toStdString());
tokenfile.append("/reason.txt");


if(std::ifstream is{tokenfile, std::ios_base::in}) {

    auto size = is.tellg();
    char * str;
    std::string content = mypayload->fromIstream(is.read(str,size));
    token = QString::fromStdString(content);
     // construct string to stream size
   //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
     is.close();

  }

QMap<QString, QVariant> params;
params["deliverer"] = delivererid;
params["recipient"] = recipientid;

QString usagequery =buildWhereClause(params,QSet<QString>(),"orders");
// Example output: "deliverer = 'D123' AND recipient = 'R789'"

mypayload->modifyJsonValue(userdoc, "query.sql", usagequery);
mypayload->modifyJsonValue(userdoc, "request_id", myrequestid);
mypayload->modifyJsonValue(userdoc, "data.username", username);
const QString usagejson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//qDebug()<<"Usage Json for query orders is: " << qPrintable(usagejson) << Qt::endl;
std::string myapiurl = QString::fromStdWString(myurldue(APIURL)).toStdString();
Wt::Http::Message message;
client_->setSslCertificateVerificationEnabled(false);
message.addBodyText(usagejson.toStdString());

if(client_->post(myapiurl+"/deliva/universal/basicupdates"+"?reason="+token.toStdString(),message)) {

   mappersoftware_->enableUpdates(true);

    }

else {


   }






return;



}




void DataNet::deleteitemcategory(QVariantMap info) {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";
std::string userdata = R"(

{
   "request_id": "fa014c8b-1e67-4cb8-8de6-7f1124c01982",
   "field": "name",
   "avatarfield": "mediafile",
   "otherfield": "name",
   "action": "delete_by_id",
   "entity": "items",
   "deleteotherentity": "categories",
   "deleteotherfield": "category",
   "deleteother": true,
   "query": {
      "request_id": "95c97c2a-6d81-4590-ada5-4a88a67f5001",
      "action": "fetch_by_query",
      "entity": "ids",
      "data": {
         "media": [
            "value1",
            "value2",
            "value3"
         ]
      },
      "query": {
         "sql": "username = 'new6'"
      }
   },
   "info": {
      "items_id": "a944688d-8733-4b98-aa2c-54787e78e1b6",
      "ids_id": "48334cea-1e65-4332-80e1-54a600ffc102",
      "category": "catnew61",
      "name": "itemnew61",
      "username": "new6",
      "created_at": "2022-09-25T16:15:22.767Z"
   },
   "data": {
      "items_id": "a944688d-8733-4b98-aa2c-54787e78e1b6"
   }
}

)";


return;



}




void DataNet::fetchspecific(QVariantMap info) {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";

std::string userdata = R"(

{
   "action": "fetch_by_query",
   "entity": "ids",
   "target": "items",
   "relations": [
      "*"
   ],
   "data": {
      "token": "911779660",
      "username": "new6",
      "created_at": "2022-09-25T16:15:19.677Z"
   },
   "query": {
      "sql": "username = 'new6'"
   }
}

)";


return;



}




void DataNet::passwordupdate(QVariantMap info) {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";

std::string userdata = R"(

{
   "request_id": "78c82b28-4ffc-4a9d-9d2b-1780d36f4dab",
   "action": "update",
   "entity": "ids",
   "data": {
      "ids_id": "48334cea-1e65-4332-80e1-54a600ffc102",
      "password": "11nill66",
      "auth": "adminqrmenulokomokopass",
      "username": "new6",
      "created_at": "2022-09-25T16:37:53.084Z"
   },
   "columns": [
      "one",
      "two",
      "three"
   ],
   "query": {
      "sql": "username = 'new6'"
   }
}

)";

QString strPayload = QString::fromStdString(userdata);
QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
QString myrequestid = mypayload->newuuid();
QString username = info.value("user").toString();
QString password = info.value("pass").toString();
QString ids_id = info.value("ids_id").toString();
QString datedue = mypayload->utctime();
QString token = "";


std::string tokenfile;
tokenfile = mypayload->TOKEN_DIRIDS;
tokenfile.append(username.toStdString());
tokenfile.append("/reason.txt");


if(std::ifstream is{tokenfile, std::ios_base::in}) {

    auto size = is.tellg();
    char * str;
    std::string content = mypayload->fromIstream(is.read(str,size));
    token = QString::fromStdString(content);
     // construct string to stream size
   //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
     is.close();

  }

mypayload->modifyJsonValue(userdoc, "request_id", myrequestid);
mypayload->modifyJsonValue(userdoc, "data.ids_id", ids_id);
mypayload->modifyJsonValue(userdoc, "data.username", username);
mypayload->modifyJsonValue(userdoc, "data.password", password);
mypayload->modifyJsonValue(userdoc, "data.created_at", datedue);
const QString usagejson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//qDebug()<<"Usage Json for password update is: " << qPrintable(usagejson) << Qt::endl;

std::string myapiurl = QString::fromStdWString(myurldue(APIURL)).toStdString();
Wt::Http::Message message;
client_->setSslCertificateVerificationEnabled(false);
message.addBodyText(usagejson.toStdString());

if (client_->post(myapiurl+"/deliva/ids/secure"+"?reason="+token.toStdString(),message)) {

   mappersoftware_->enableUpdates(true);

    }

else {


   }





return;

}

void DataNet::phonenumberupdate(QVariantMap info) {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";


std::string userdata = R"(

{
   "request_id": "d1450618-e175-4059-aa29-438aa54442bd",
   "action": "update",
   "entity": "ids",
   "data": {
      "ids_id": "48334cea-1e65-4332-80e1-54a600ffc102",
      "phone": "0742954736",
      "auth": "adminqrmenulokomokophone",
      "username": "new6",
      "created_at": "2022-09-25T16:41:04.079Z"
   },
   "columns": [
      "phone"
   ],
   "query": {
      "sql": "username = 'new6'"
   }
}

)";


QString strPayload = QString::fromStdString(userdata);
QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
QString myrequestid = mypayload->newuuid();
QString username = info.value("user").toString();
QString phone = info.value("phone").toString();
QString ids_id = info.value("ids_id").toString();
QString datedue = mypayload->utctime();
QString token = "";


std::string tokenfile;
tokenfile = mypayload->TOKEN_DIRIDS;
tokenfile.append(username.toStdString());
tokenfile.append("/reason.txt");


if(std::ifstream is{tokenfile, std::ios_base::in}) {

    auto size = is.tellg();
    char * str;
    std::string content = mypayload->fromIstream(is.read(str,size));
    token = QString::fromStdString(content);
     // construct string to stream size
   //qDebug() << "content is: " << QString::fromStdString(content) << Qt::endl;
     is.close();

  }

mypayload->modifyJsonValue(userdoc, "request_id", myrequestid);
mypayload->modifyJsonValue(userdoc, "data.ids_id", ids_id);
mypayload->modifyJsonValue(userdoc, "data.username", username);
mypayload->modifyJsonValue(userdoc, "data.phone", phone);
mypayload->modifyJsonValue(userdoc, "data.created_at", datedue);
const QString usagejson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
//qDebug()<<"Usage Json for phone number update is: " << qPrintable(usagejson) << Qt::endl;

std::string myapiurl = QString::fromStdWString(myurldue(APIURL)).toStdString();
Wt::Http::Message message;
client_->setSslCertificateVerificationEnabled(false);
message.addBodyText(usagejson.toStdString());

if (client_->post(myapiurl+"/deliva/universal/basicupdates"+"?reason="+token.toStdString(),message)) {

   mappersoftware_->enableUpdates(true);

    }

else {


   }





return;

}


void DataNet::logout(QVariantMap info) {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";

std::string userdata = R"(
{
   "request_id": "c15fc926-201b-4d60-93d1-2244c4aefef6",
   "action": "update",
   "entity": "ids",
   "data": {
      "ids_id": "48334cea-1e65-4332-80e1-54a600ffc102",
      "username": "new6",
      "logout": 23456979,
      "created_at": "2022-09-25T16:31:56.708Z"
   },
   "columns": [
      "logout"
   ],
   "query": {
      "sql": "username = 'new6'"
   }
}
)";


QString strPayload = QString::fromStdString(userdata);
QJsonDocument userdoc = QJsonDocument::fromJson(strPayload.toUtf8());
QString myrequestid = mypayload->newuuid();
QString username = info.value("user").toString();
QString ids_id = info.value("ids_id").toString();
QString datedue = mypayload->utctime();
QString token = "";


std::string tokenfile;
tokenfile = mypayload->TOKEN_DIRIDS;
tokenfile.append(username.toStdString());
tokenfile.append("/reason.txt");


if(std::ifstream is{tokenfile, std::ios_base::in}) {

    auto size = is.tellg();
    char * str;
    std::string content = mypayload->fromIstream(is.read(str,size));
    token = QString::fromStdString(content);
     // construct string to stream size
   //qInfo() << "content for token file is: " << QString::fromStdString(content) << Qt::endl;
     is.close();

  }

mypayload->modifyJsonValue(userdoc, "request_id", myrequestid);
mypayload->modifyJsonValue(userdoc, "data.ids_id", ids_id);
mypayload->modifyJsonValue(userdoc, "data.username", username);
mypayload->modifyJsonValue(userdoc, "data.created_at", datedue);
const QString usagejson = userdoc.toJson(QJsonDocument::JsonFormat::Indented);
qInfo()<<"Usage Json for logout is: " << qPrintable(usagejson) << Qt::endl;
qInfo()<<"Token for logout is: " << token << Qt::endl;

std::string myapiurl = QString::fromStdWString(myurldue(APIURL)).toStdString();
Wt::Http::Message message;
client_->setSslCertificateVerificationEnabled(false);
message.addBodyText(usagejson.toStdString());

if (client_->post(myapiurl+"/deliva/ids/logout"+"?reason="+token.toStdString(),message)) {

   mappersoftware_->enableUpdates(true);

    }

else {


   }





return;

}



void  DataNet::getallitems() {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";

std::string myapiurl = QString::fromStdWString(myurldue(APIURL)).toStdString();
std::string token = QString::fromStdWString(myroledue(SUPERPOWER)).toStdString();
client_->setSslCertificateVerificationEnabled(false);

if (client_->get(myapiurl+"/deliva/fetchidssuperpower/all"+"?reason="+token)) {

   mappersoftware_->enableUpdates(true);

   }

else {


}


}

void  DataNet::getuseritems(QString user) {

combimsg ="";
//client_->abort();
//std::string jsondata = R"( )";

std::string myapiurl = QString::fromStdWString(myurldue(APIURL)).toStdString();
std::string token = QString::fromStdWString(myroledue(SUPERPOWER)).toStdString();
//qInfo() << "My API URL IS: " << QString::fromStdString(myapiurl) << Qt::endl;
//qInfo() << "TOKEN IS: " << QString::fromStdString(token) << Qt::endl;
//qInfo() << "USER IS: " << user << Qt::endl;
client_->setSslCertificateVerificationEnabled(false);

if (client_->get(myapiurl+"/deliva/fetchids/"+user.toStdString()+"?reason="+token)) {

mappersoftware_->enableUpdates(true);

}

else {


}



}






