
#include "MyJsonTools.hpp"


    MyJsonTools::MyJsonTools() {
      // ensureDirectoriesExist();
    }

   MyJsonTools::~MyJsonTools() {}



    // Helper function to create directories
    bool MyJsonTools::ensureDirectoriesExist() {
        bool allCreated = true;

        // Create UPLOAD_DIRIDS
        if (!QDir().mkpath(QString::fromStdString(UPLOAD_DIRIDS))) {
            qDebug() << "Failed to create directory:" << QString::fromStdString(UPLOAD_DIRIDS);
            allCreated = false;
        }

        // Create LOGS_ROOTIDS
        if (!QDir().mkpath(QString::fromStdString(LOGS_ROOTIDS))) {
            qDebug() << "Failed to create directory:" << QString::fromStdString(LOGS_ROOTIDS);
            allCreated = false;
        }

        // Create UPLOAD_TEMPDIRIDS
        if (!QDir().mkpath(QString::fromStdString(UPLOAD_TEMPDIRIDS))) {
            qDebug() << "Failed to create directory:" << QString::fromStdString(UPLOAD_TEMPDIRIDS);
            allCreated = false;
        }

        if (allCreated) {
            qDebug() << "All required directories are ready.";
        }

        return allCreated;
    }





    void MyJsonTools::buildError(int errCode, const QString & errDesc)
    {
    #if (QT_VERSION >= 0x050300)
       m_error = QSqlError(errDesc, "", QSqlError::UnknownError, QString::number(errCode));
    #else // (QT_VERSION >= 0x050300)
       m_error = QSqlError(errDesc, "", QSqlError::UnknownError, errCode);
    #endif // (QT_VERSION >= 0x050300)
       QJsonObject errJson; QJsonObject errDetail;
       errDetail.insert("code", errCode);
       errDetail.insert("desc", errDesc);
       errJson.insert("error", errDetail);
       if (! m_requestId.isEmpty()) { errJson.insert("request_id", m_requestId); }
       m_errorJson = errJson;
    }

    void MyJsonTools::buildError(const QSqlError & error)
    {
       if (! error.isValid()) { return; }
       m_error = error;
       QJsonObject errJson; QJsonObject errDetail;
    #if (QT_VERSION >= 0x050300)
       errDetail.insert("code", error.nativeErrorCode());
    #else // (QT_VERSION >= 0x050300)
       errDetail.insert("code", error.number());
    #endif // (QT_VERSION >= 0x050300)
       QString errMsg = (error.driverText() + "\n" + error.databaseText());
       errDetail.insert("desc", errMsg);
       errJson.insert("error", errDetail);
       if (! m_requestId.isEmpty()) { errJson.insert("request_id", m_requestId); }
       m_errorJson = errJson;
    }



    QString MyJsonTools::processRequest(const QString & request)
    {
    #ifdef _QX_NO_JSON
       QString msg = "QxOrm library must be built without _QX_NO_JSON compilation option to be able to call REST API module";
       m_error = QSqlError(msg, "Cannot process request with _QX_NO_JSON compilation option", QSqlError::UnknownError);
       return "{ \"error\": \"" + msg + "\" }";
    #else // _QX_NO_JSON
       clear();
       if (parseRequest(request)) { processRequest(m_requestJson); }
       QJsonValue result = (m_errorJson.isNull() ? m_responseJson : m_errorJson);
       return qx::cvt::to_string(result);
    #endif // _QX_NO_JSON
    }


    QString MyJsonTools::processRequest(const QByteArray & request)
    {
    #ifdef _QX_NO_JSON
       QString msg = "QxOrm library must be built without _QX_NO_JSON compilation option to be able to call REST API module";
       m_error = QSqlError(msg, "Cannot process request with _QX_NO_JSON compilation option", QSqlError::UnknownError);
       return "{ \"error\": \"" + msg + "\" }";
    #else // _QX_NO_JSON
       clear();
       if (parseRequest(request)) { processRequest(m_requestJson); }
       QJsonValue result = (m_errorJson.isNull() ? m_responseJson : m_errorJson);
       return qx::cvt::to_string(result);
    #endif // _QX_NO_JSON
    }



    void MyJsonTools::setData(const QJsonValue & data) { m_dataJson = data; }

    QJsonValue MyJsonTools::processRequest(const QJsonValue & request)
    {
       clear();
       if (m_error.isValid()) { buildError(m_error); return m_errorJson; }
       if (request.isArray()) { return processRequestAsArray(request); }
       m_requestJson = request;
       if (! doRequest()) { return m_errorJson; }
       return m_responseJson;
    }

    QJsonValue MyJsonTools::processRequestAsArray(const QJsonValue & request)
    {
       QJsonArray responseArray;
       QJsonArray requestArray = request.toArray();
       if (requestArray.count() <= 0) { buildError(9999, "Request array is empty"); return m_errorJson; }


       for (int i = 0; i < requestArray.count(); i++)
       {
          resetRequest();
          m_requestJson = requestArray.at(i);
          if (! doRequest()) { break; }
          responseArray.append(m_responseJson);
       }


       if (! m_errorJson.isNull()) { return m_errorJson; }
       m_responseJson = responseArray;
       return m_responseJson;
    }




    void MyJsonTools::clear()
    {
       // Reset error and instance
       m_responseJson = QJsonValue();
       m_errorJson = QJsonValue();

    }


    void MyJsonTools::resetRequest()
    {

       m_dataJson = QJsonValue();
       m_requestJson = QJsonValue();
       m_responseJson = QJsonValue();

    }


    bool MyJsonTools::parseRequest(const QString & request)
    {
       // Parse request as JSON format
       QJsonParseError jsonError;
       QByteArray requestAsByteArray = request.toUtf8();
       QJsonDocument doc = QJsonDocument::fromJson(requestAsByteArray, (& jsonError));
       if (jsonError.error == QJsonParseError::NoError)
       {

       // Request can be an array or a single object

       m_requestJson = (doc.isArray() ? QJsonValue(doc.array()) : QJsonValue(doc.object()));

       return true;

       }

       else {

          // qDebug() << "json error is: " << jsonError.error << Qt::endl;
           return false;
       }


    }


    bool MyJsonTools::parseRequest(const QByteArray & request)
    {
       // Parse request as JSON format
       QJsonParseError jsonError;
       QByteArray requestAsByteArray = request;
       QJsonDocument doc = QJsonDocument::fromJson(requestAsByteArray, (& jsonError));

       if (jsonError.error == QJsonParseError::NoError) {

       // Request can be an array or a single object
       m_requestJson = (doc.isArray() ? QJsonValue(doc.array()) : QJsonValue(doc.object()));
       return true;

           }

       else {

          // qDebug() << "json error is: " << jsonError.error << Qt::endl;
           return false;
       }


     }

    bool MyJsonTools::decodeRequest()
    {
       // Check if request is a valid JSON object
       if (m_requestJson.isNull()) {



           return false;

       }
       if (! m_requestJson.isObject()) {



           return false;

       }
       QJsonObject request = m_requestJson.toObject();

       // Extract request identifier
       if (request.contains("request_id")) { m_requestId = request.value("request_id").toString(); }

       // Extract action
       if (request.contains("action")) {

           m_action = request.value("action").toString();

       }
       else {  return false; }
       if (m_action.isEmpty()) {  return false; }

       // Extract entity
       if (request.contains("entity")) { m_entity = request.value("entity").toString(); }

       // Extract data
       if (request.contains("data")) { m_dataJson = request.value("data"); }
       else if (! m_data.isEmpty())
       {
          qx_bool bParseOk = qx::cvt::from_string(m_data, m_dataJson);
          if (! bParseOk) {  return false; }
       }

       // Extract function
       if (request.contains("fct")) { qx::cvt::from_json(request.value("fct"), m_function); }

       // Extract columns
       if (request.contains("columns")) { qx::cvt::from_json(request.value("columns"), m_columns); }

       // Extract relations
       if (request.contains("relations")) { qx::cvt::from_json(request.value("relations"), m_relations); }

       // Extract output format
       if (request.contains("output_format")) { qx::cvt::from_json(request.value("output_format"), m_outputFormat); }

       // Extract database key
          if (request.contains("database")) { m_database = request.value("database").toString(); }


       // Extract save mode for 'save' action
       if (request.contains("save_mode"))
       {
          QString sSaveMode = request.value("save_mode").toString();
          bool bSaveModeAsInt = false; int iSaveMode = sSaveMode.toInt(& bSaveModeAsInt);
          if (bSaveModeAsInt) { m_eSaveMode = static_cast<qx::dao::save_mode::e_save_mode>(iSaveMode); }
          else { m_eSaveMode = ((sSaveMode == "check_insert_or_update") ? qx::dao::save_mode::e_check_insert_or_update : ((sSaveMode == "insert_only") ? qx::dao::save_mode::e_insert_only : ((sSaveMode == "update_only") ? qx::dao::save_mode::e_update_only : qx::dao::save_mode::e_none))); }
       }


       // Extract query
       if (request.contains("query")) { qx::cvt::from_json(request.value("query"), m_qxQuery); }
       else if (! m_query.isEmpty()) { m_qxQuery = qx_query(m_query); }

       return true;
    }

    bool MyJsonTools::checkRequest()
    {
       bool isEntityRequired = ((m_action == "get_meta_data") || (m_action == "call_entity_function"));
       isEntityRequired = (isEntityRequired || (m_action == "fetch_by_id") || (m_action == "fetch_all") || (m_action == "fetch_by_query"));
       isEntityRequired = (isEntityRequired || (m_action == "insert") || (m_action == "update") || (m_action == "save"));
       isEntityRequired = (isEntityRequired || (m_action == "delete_by_id") || (m_action == "delete_all") || (m_action == "delete_by_query"));
       isEntityRequired = (isEntityRequired || (m_action == "destroy_by_id") || (m_action == "destroy_all") || (m_action == "destroy_by_query"));
       isEntityRequired = (isEntityRequired || (m_action == "exec_custom_query") || (m_action == "exist") || (m_action == "validate") || (m_action == "count"));
       if (isEntityRequired && (m_entity.isEmpty())) {  return false; }

       bool isDataRequired = ((m_action == "fetch_by_id") || (m_action == "delete_by_id") || (m_action == "destroy_by_id"));
       isDataRequired = (isDataRequired || (m_action == "insert") || (m_action == "update") || (m_action == "save"));
       isDataRequired = (isDataRequired || (m_action == "exist") || (m_action == "validate"));
       if (isDataRequired && (m_dataJson.isNull())) { return false; }

       bool isQueryRequired = ((m_action == "fetch_by_query") || (m_action == "delete_by_query") || (m_action == "destroy_by_query"));
       isQueryRequired = (isQueryRequired || (m_action == "exec_custom_query") || (m_action == "call_custom_query"));
       if (isQueryRequired && (m_qxQuery.query().isEmpty())) { return false; }

       bool isFunctionRequired = (m_action == "call_entity_function");
       if (isFunctionRequired && (m_function.isEmpty())) {  return false; }

       return true;
    }


    bool MyJsonTools::createInstance()
    {
       // Some actions doesn't require any instance
       if (m_action == "get_meta_data") { return true; }
       if (m_action == "get_databases") { return true; }
       if (m_action == "call_custom_query") { return true; }
       if (m_action == "call_entity_function") { return true; }


      // create an instance of database persistence
       // Check if action requires some input data
       if (m_action == "count") { return true; }
       if (m_action == "delete_all") { return true; }
       if (m_action == "destroy_all") { return true; }
       if (m_action == "delete_by_query") { return true; }
       if (m_action == "destroy_by_query") { return true; }

       // Check if some input data has been provided by caller
       if (m_dataJson.isNull() && (m_action != "fetch_all") && (m_action != "fetch_by_query"))
       {

          return false;
       }

      /* if (!m_dataJson.isArray())
       {
        QJsonObject myrequest = m_dataJson.toObject();
        qDebug() <<"Value of username from object is: " << myrequest.value("username").toString() << Qt::endl;

       }

       else {

           QJsonArray dataArray = m_dataJson.toArray(); if (dataArray.count() <= 0) {  return false; }
           QJsonValue dataFirst = dataArray.first(); if (dataFirst.isNull()) {  return false; }
           if (! dataFirst.isObject()) {  return false; }
           QJsonObject dataFirstObject = dataFirst.toObject();

           qDebug() <<"Value of username from array is: " << dataFirstObject.value("username").toString() << Qt::endl;


       } */



       // Check data format : single instance, or array of instances, or collection of instances (hash-map key/value)
       if (m_dataJson.isArray())
       {

          QJsonArray dataArray = m_dataJson.toArray(); if (dataArray.count() <= 0) {  return false; }
          QJsonValue dataFirst = dataArray.first(); if (dataFirst.isNull()) {  return false; }
          if (! dataFirst.isObject()) {  return false; }
          QJsonObject dataFirstObject = dataFirst.toObject();

       //   qDebug() <<"Value of username from array is: " << dataFirstObject.value("username").toString() << Qt::endl;

          if (dataFirstObject.isEmpty()) {  return false; }
          if ((dataFirstObject.count() == 2) && dataFirstObject.contains("key") && dataFirstObject.contains("value")) {
             // create a collection
          }
          else { // create a collection
          }
         // create a collection of pointers
       }
       else if ((m_action == "fetch_all") || (m_action == "fetch_by_query"))
       {
          // create a collection of pointers
       }

       // Fill data in Persistable instance

       return true;
    }


    bool MyJsonTools::buildResponse()
    {
       if (m_action == "get_meta_data") { return m_errorJson.isNull(); }
       if (m_action == "get_databases") { return m_errorJson.isNull(); }

       QJsonObject response;
       if (m_action == "count") { response.insert("count", static_cast<double>(m_countResult)); m_responseJson = response; return true; }
       else if (m_action == "exist") { response.insert("exist", (m_existResult ? true : false)); m_responseJson = response; return true; }
       else if (m_action == "validate") { response.insert("invalid_values", qx::cvt::to_json(m_validateResult)); m_responseJson = response; return true; }
       else if (m_action == "call_custom_query") { response.insert("query_output", qx::cvt::to_json(m_qxQuery)); m_responseJson = response; return true; }
       else if ((m_action == "delete_all") || (m_action == "delete_by_query")) { response.insert("deleted", true); m_responseJson = response; return true; }
       else if ((m_action == "destroy_all") || (m_action == "destroy_by_query")) { response.insert("destroyed", true); m_responseJson = response; return true; }
       else if (m_action == "call_entity_function") { return true; }

       QString outputFormat = (m_outputFormat.isEmpty() ? QString() : QString("filter: " + m_outputFormat.join(" | ")));
       if (outputFormat.isEmpty() && ((m_action == "insert") || (m_action == "update") || (m_action == "save"))) { outputFormat = QX_JSON_SERIALIZE_ONLY_ID; }
       else if (outputFormat.isEmpty() && ((m_action == "delete_by_id") || (m_action == "destroy_by_id"))) { outputFormat = QX_JSON_SERIALIZE_ONLY_ID; }
     // Assign to  m_responseJson =
       return true;
    }

    bool MyJsonTools::formatResponse()
    {
       QJsonObject response;
       if (! m_requestId.isEmpty()) { response.insert("request_id", m_requestId); }
       response.insert("data", m_responseJson);
       m_responseJson = response;
       return true;
    }


    bool MyJsonTools::executeAction()
    {
       m_error = QSqlError();
       m_errorJson = QJsonValue();

       try
       {
          if (m_action == "count")                     {

          }
          else if (m_action == "fetch_by_id")          {
          }
          else if (m_action == "fetch_all")            {
          }
          else if (m_action == "fetch_by_query")       {

          }
          else if (m_action == "insert")               {

              // m_responseJson =

              QJsonObject response;
              response.insert("execution", "okay");
              m_responseJson = response;

          }



          else if (m_action == "update")               {  }
          else if (m_action == "save")                 {  }
          else if (m_action == "delete_by_id")         {  }
          else if (m_action == "delete_all")           {  }
          else if (m_action == "delete_by_query")      {  }
          else if (m_action == "destroy_by_id")        {  }
          else if (m_action == "destroy_all")          {  }
          else if (m_action == "destroy_by_query")     {  }
          else if (m_action == "exec_custom_query")    {  }
          else if (m_action == "exist")                {  }
          else if (m_action == "validate")             {  }
          else if (m_action == "call_custom_query")    {  }
          else if (m_action == "call_entity_function") {  }
          else if (m_action == "get_meta_data")        {  }
          else if (m_action == "get_databases")        {  }
          else                                         {  }
       }
       catch (const qx::exception & x) {  return false; }
       catch (const std::exception & e) {  return false; }
       catch (...) {  return false; }

       if (m_error.isValid()) {  }
       return (! m_error.isValid());
    }

    bool MyJsonTools::doRequest()
    {
       if (! decodeRequest()) { return false; }
       if (! checkRequest()) { return false; }
       if (! createInstance()) { return false; }
       if (! executeAction()) { return false; }
       if (! buildResponse()) { return false; }
       if (! formatResponse()) { return false; }
       return true;
    }



QString MyJsonTools::queryparam() {


    QString param = "reason=testapi";
    return param;


}


 qint64 MyJsonTools::rng() {


    QRandomGenerator *gen = QRandomGenerator::system();
    gen->securelySeeded();
    gen->generate64();
    qint64 discard = gen->bounded(10000);

    return discard;


}


std::string  MyJsonTools::doupload(QByteArray data,std::string tmpdir,std::string uploaddir,long size,std::string type
                      ,bool save,bool update,std::string previousfile) {

 std::string jsontext = "null";
 std::string multipartContent="";
 long mylen=0;
 QByteArray utfString = data;
 const long mysize  =  size;
// qDebug() << " Content-Length is: " << mysize << Qt::endl;
 multipartContent = type;
// qDebug() << " multipartContent is: " << QString::fromStdString(multipartContent)<< Qt::endl;
 const char* ericdata = utfString.constData();
// qDebug() << "ericdata" << qPrintable(ericdata) << Qt::endl;

  try { // start try block

  //MPFD::Parser *parser = new MPFD::Parser();

 std::shared_ptr<MPFD::Parser> parser { new MPFD::Parser };
 parser->SetMaxCollectedDataLength(6000000000);
 parser->SetTempDirForFileUpload(tmpdir);
 parser->SetContentType(multipartContent);
 parser->AcceptSomeData(ericdata, mysize);
 std::map<std::string,MPFD::Field *> fields=parser->GetFieldsMap();
 std::map<std::string,MPFD::Field *>::iterator it;


 for (it=fields.begin();it!=fields.end();it++) { // start for loop

  if (fields[it->first]->GetType()==MPFD::Field::TextType) { // if text type

 // BOOST_LOG_TRIVIAL(info) << "Got text field=" +it->first << "\tvalue of it is\n" << fields[it->first]->
                      //GetTextTypeContent();
 std::string jsondata = fields[it->first]->GetTextTypeContent();

   jsontext.clear();
   jsontext = jsondata;




           }  // end if text type


  else {  // start else for binary part

  if(save) {


//BOOST_LOG_TRIVIAL(info) << "Got file field=" +it->first << "\tFilename is="+fields[it->first]->GetFileName()
                      //  << "\tTempFilename=" +fields[it->first]->GetTempFileNameMPFD() ;
std::ifstream  src( fields[it->first]->GetTempFileNameMPFD().c_str(), std::ios::binary);
std::string dstFilename= std::string(uploaddir)+'/'+fields[it->first]->GetFileName();
std::ofstream  dst( dstFilename.c_str(), std::ofstream::binary);

       if (!src || !dst) { // start binary copy error

         BOOST_LOG_TRIVIAL(info) <<  "Copy error: check read/write permissions";


          } // end binary copy error

       else { // start binary copy success

       if(update) {

       std::string oldavatarfile = std::string(uploaddir)+'/'+previousfile;

       if( remove(oldavatarfile.c_str()) != 0 ) {

           perror( "Error deleting file:" );

                   }
          else {

            puts( "File successfully deleted" );

             }

         }

         dst << src.rdbuf();

         } // end binary copy success

         src.close();
         dst.close();


   }






  } // end else for binary part


        } // end for loop




             } // end try block


  catch (MPFD::Exception & e) {

       std::cout << e.GetError() << std::endl;
       // Parsing input error


       //  exit(EXIT_SUCCESS);

     }


 return jsontext;




 }

 QVariantMap MyJsonTools::encryptpass(std::string mypass) {

   //  SettingsManagerEric mysettings;
     AutoSeededRandomPool prng;
     QVariantMap encryptioninfo;
     encryptioninfo.clear();
     std::string token;
   //  HexEncoder encoder(new FileSink(std::cout));
     HexEncoder encoder(new StringSink(token));
     std::string plain(mypass), cipher, recover;

     SecByteBlock iv(12);
     QRandomGenerator64 *gen = QRandomGenerator64::system();
     gen->securelySeeded();
     qint64 discard1 = gen->bounded(1000000000);
     qint64 discard2 = gen->bounded(1000000000);
     qint64 discard3 = gen->bounded(1000000000);
     qint64 discard4 = gen->bounded(100000);

     QRandomGenerator64 *genextra = QRandomGenerator64::system();
     genextra->securelySeeded();
     qint64 extra1 = gen->bounded(10);
     qint64 extra2 = gen->bounded(100);
     qint64 extra3 = gen->bounded(1000);


     std::string sKey = QString::number(discard1).toStdString()
             +QString::number(discard2).toStdString()+
             QString::number(discard3).toStdString() + QString::number(discard4).toStdString();
     if(sKey.size()==29) {

       //  std::cout << "skey size is: " << sKey.size() << std::endl;
         sKey = sKey+QString::number(extra3).toStdString();

     }

     else if(sKey.size()==30) {

       //  std::cout << "skey size is: " << sKey.size() << std::endl;
          sKey = sKey+QString::number(extra2).toStdString();
     }

     else if(sKey.size()==31) {

        //  std::cout << "skey size is: " << sKey.size() << std::endl;
          sKey = sKey+QString::number(extra1).toStdString();
     }

     else if(sKey.size()==32){

       //  std::cout << "skey size is: " << sKey.size() << std::endl;
     }

  //   std::cout << "skey is: " << sKey << std::endl;
  //   mysettings.write("key",QString::fromStdString(sKey));

     SecByteBlock key((const unsigned char*)(sKey.data()), sKey.size());
     prng.GenerateBlock(iv, iv.size());


     encoder.Put((const byte*)key.data(), key.size());
     encoder.MessageEnd();

     encryptioninfo.insert("key",QString::fromStdString(sKey));



     token.clear();
     encoder.Put((const byte*)iv.data(), iv.size());
     encoder.MessageEnd();



  //   std::cout << "IV is: " << token << std::endl;

 //   mysettings.write("iv",QString::fromStdString(token));
     encryptioninfo.insert("iv",QString::fromStdString(token));





     // Encryption object

      try
         {


     ChaChaTLS::Encryption enc;

     enc.SetKeyWithIV(key, key.size(), iv, iv.size());

     // Perform the encryption
     cipher.resize(plain.size());
     enc.ProcessData((byte*)&cipher[0], (const byte*)plain.data(), plain.size());

   //  std::cout << "Plain: " << plain << std::endl;

     token.clear();
     encoder.Put((const byte*)cipher.data(), cipher.size());
     encoder.MessageEnd();

     encryptioninfo.insert("cipher",QString::fromStdString(token));



  //   mysettings.write("cipher",QString::fromStdString(token));

 //   std::cout << "cipher text is: " << token << std::endl;

      }

      catch (const Exception& e)
        {
            //  std::cerr <<"exception at encryption is: " << e.what() << std::endl;

       }




    return  encryptioninfo;




 }

 bool MyJsonTools::decryptpass(std::string mypass,QVariantMap crypto) {

     SettingsManagerEric mysettings;
     bool verified = false;
     std::string keytext = crypto.value("key").toString().toStdString();;
     std::string ivtext = crypto.value("iv").toString().toStdString();
    std::string ciphertext = crypto.value("cipher").toString().toStdString();
     std::string plain(mypass), recover;


     SecByteBlock key((const unsigned char*)( keytext.data()),  keytext.size());

     std::string iv, encMessage;
     HexDecoder decoder;

     std::string sIv = ivtext;
     std::string encMessageHex = ciphertext;
     std::string recovered;

   //  std::cout << "key text is: " << keytext << std::endl;
   //  std::cout << "key size is: " << key.size() << std::endl;


     decoder.Attach(new StringSink(iv));
     decoder.Put((byte*)sIv.data(), sIv.size());
     decoder.MessageEnd();
     // Forgot to decode encrypted message hex. added here
     decoder.Attach(new StringSink(encMessage));
     decoder.Put((byte*)encMessageHex.data(), encMessageHex.size());
     decoder.MessageEnd();
      try
         {

      ChaChaTLS::Decryption d;
      d.SetKeyWithIV(key.data(), key.size(), (byte *)iv.data(), 12);

      StringSource s(encMessage, true,
          new StreamTransformationFilter(d,
              new StringSink(recovered)
              )
        );

      }

     catch (const Exception& e)
       {


       // std::cerr <<"exception at decryption is: " << e.what() << std::endl;

      }



  //  std::cout << "recovered is: " << recovered << std::endl;






     if(recovered==plain) {

         verified=true;
     }

     else {

         verified=false;

     }


     return verified;


 }


 std::string MyJsonTools::recoverpass(QVariantMap crypto) {

     SettingsManagerEric mysettings;
     std::string keytext = crypto.value("key").toString().toStdString();;
     std::string ivtext = crypto.value("iv").toString().toStdString();
     std::string ciphertext = crypto.value("cipher").toString().toStdString();
     std::string  recover;


     SecByteBlock key((const unsigned char*)( keytext.data()),  keytext.size());

     std::string iv, encMessage;
     HexDecoder decoder;

     std::string sIv = ivtext;
     std::string encMessageHex = ciphertext;
     std::string recovered;

   //  std::cout << "key text is: " << keytext << std::endl;
   //  std::cout << "key size is: " << key.size() << std::endl;


     decoder.Attach(new StringSink(iv));
     decoder.Put((byte*)sIv.data(), sIv.size());
     decoder.MessageEnd();
     // Forgot to decode encrypted message hex. added here
     decoder.Attach(new StringSink(encMessage));
     decoder.Put((byte*)encMessageHex.data(), encMessageHex.size());
     decoder.MessageEnd();
      try
         {

      ChaChaTLS::Decryption d;
      d.SetKeyWithIV(key.data(), key.size(), (byte *)iv.data(), 12);

      StringSource s(encMessage, true,
          new StreamTransformationFilter(d,
              new StringSink(recovered)
              )
        );

      }

     catch (const Exception& e)
       {


       // std::cerr <<"exception at decryption is: " << e.what() << std::endl;

      }



  //  std::cout << "recovered is: " << recovered << std::endl;








     return recovered;


 }



   QDateTime MyJsonTools::duedate()  {

     /*  QDate cd = QDate::currentDate();
      QTime ct = QTime::currentTime();
      QTimeZone tz = QTimeZone("Africa/Nairobi");
      qDebug()<<"Timezone is: " << local.timeZone() << Qt::endl;
      qDebug()<<"Time is: " << local.toString() << Qt::endl;
      qDebug() << "Current date is: " << cd.toString() << Qt::endl;
      qDebug() << "Current time is: " << ct.toString() << Qt::endl;
      qDebug() << "my local time" << mylocaltime("2022-06-21T10:35:43.408Z", "yyyy-MM-ddTHH:mm:ss.zzzZ") << Qt::endl; */
      QDateTime local = QDateTime::currentDateTime();
      return local;

    }

  QString MyJsonTools::utctime() {

  QDateTime local = QDateTime::currentDateTime();
  QDateTime UTC(local.toUTC());
  QString thedatetime = UTC.toString(Qt::ISODate);
  return thedatetime;


    }

     QString MyJsonTools::mylocaltime(QString utc,QString fmt) {


        QDateTime date = QDateTime::fromString(utc,fmt);
        date.setTimeSpec(Qt::UTC);
        QDateTime localtm = date.toLocalTime();
      /*  qDebug() << "utc: " << date;
        qDebug() << "local: " << localtm.toString();
        qDebug() << "hax: " << localtm.toString(Qt::SystemLocaleLongDate); */
        return localtm.toString();


    }






   bool MyJsonTools::writeJsonFile(QVariantMap point_map, QString file_path)
    {
        QJsonObject json_obj = QJsonObject::fromVariantMap(point_map);
        QJsonDocument json_doc(json_obj);
        QString json_string = json_doc.toJson();

   /*

        QVariantList myjsonmapdat = eric::JsonUtils::toVariantList(json_string);
        QJsonArray arr= QJsonArray::fromVariantList(myjsonmapdat);
        QJsonObject obj;
        obj [ "eric" ] = arr;
        const auto docjson = QJsonDocument{ obj };
        const auto jsonstr = QString::fromLatin1( docjson.toJson() );
        qDebug() << "custom document is:\n" << qPrintable( jsonstr );

    */

      // if (!save_file.open(QFile::WriteOnly|QFile::Truncate)) {
        QFile save_file(file_path);
        if (!save_file.open(QIODevice::WriteOnly)) {
            qDebug() << "failed to open save file" ;
            return false;
        }

        save_file.write(json_string.toLocal8Bit());
       // save_file.write(jsonstr.toLocal8Bit());
     //   save_file.resize(0);
        save_file.resize(save_file.pos());
        save_file.close();
        return true;
    }

   QVariantMap MyJsonTools::readJsonFile(QString file_path)
    {
        // step 1
        QVariantMap results;
        QFile file_obj(file_path);
        if (!file_obj.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open: " << file_path ;
        exit(1);
        }

        // step 2
        QTextStream file_text(&file_obj);
        QString json_string;
        json_string = file_text.readAll();
        file_obj.close();
        QByteArray json_bytes = json_string.toLocal8Bit();

        // step 3
        auto json_doc = QJsonDocument::fromJson(json_bytes);

        if (json_doc.isNull()) {
            qDebug() << "Failed to create JSON doc." ;

        }
        if (!json_doc.isObject()) {
            qDebug() << "JSON is not an object." ;

        }

        QJsonObject json_obj = json_doc.object();

        if (json_obj.isEmpty()) {
            qDebug()<< "JSON object is empty." ;

        }

        // step 4
        results = json_obj.toVariantMap();
        return results;
    }

     QString MyJsonTools::datapath(){

     QString mDataRoot = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0);
     return mDataRoot;

       }

     QString MyJsonTools::convertUrlToFilePath(const QString & filePath) {
      // This function convert the url passed from qml in a valid resource path.
      QUrl url(filePath);
      if (url.scheme().compare(QLatin1String("qrc"), Qt::CaseInsensitive) == 0) {
        if (url.authority().isEmpty()) {
          return QLatin1Char(':') + url.path();
        }
      }
      return QString();
    }

     QString MyJsonTools::base64encode(QString content) {
         return content.toUtf8().toBase64();
     }

    QString MyJsonTools::base64decode(QString content) {
         QByteArray arr = content.toUtf8();
         QByteArray dec = QByteArray::fromBase64(arr);

         return QString(dec);
     }







    QString MyJsonTools::jsondatapath(){

    QString mDataRoot = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0);
    QString jsonpath = mDataRoot+"/json/";
    return jsonpath;

      }

     QString MyJsonTools::jsonqrcpath(){

      QString mConfigqrc =":/data-assets/json/";
     return mConfigqrc;

       }

    bool MyJsonTools::copyjsonassets(QString mjsonPathfile,QString mjsonqrcfile )
    {




            QFile readFile(mjsonPathfile);
            if(!readFile.exists()) {
                qDebug() << "config data path does not exist: " << mjsonPathfile;
                QFile qrcFile( mjsonqrcfile);
                if(qrcFile.exists()) {
                    // copy file from qrc to data
                    bool copyOk = qrcFile.copy(mjsonPathfile);
                    if (!copyOk) {
                        qDebug() << "cannot copy data assets from qrc to data path";
                        return false;
                    }
                    // IMPORTANT !!! copying from RESOURCES ":/data-assets/" to AppDataLocation
                    // makes the target file READ ONLY - you must set PERMISSIONS
                    // copying from RESOURCES ":/config/" to GenericDataLocation the target is READ-WRITE
                    copyOk = readFile.setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser);
                    if (!copyOk) {
                        qDebug() << "cannot set Permissions to read / write settings";
                        return false;
                    }
                } else {
                    qDebug() << "nothing in qrc " <<  mjsonqrcfile;
                    return false;
                }
            }
            if (!readFile.open(QIODevice::ReadOnly)) {
                qWarning() << "Couldn't open file: " << mjsonPathfile; 
                return false;
            }

            return true;
    }

    QString MyJsonTools::fluidiconpath() const {
             return QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0)+"/assets/fluidicons";
           }
      QString MyJsonTools::extractword(QString word,QString criteria) {


         QRegularExpression eric;
         QString results="";
         eric.setPattern("\\s*"+criteria+"\\s*");
          QRegularExpressionMatch match = eric.match(word);
          if (match.hasMatch()) {
                QString matched = match.captured(0);
                qDebug()<<"match is:" << matched;
                results=matched;
            }


         return results;



     }
     QString MyJsonTools::jsonlink(){

      // QString myjsonlink ="https://deliva.stratetactical.com/";
        QString myjsonlink ="http://127.0.0.1:8180/";
        return myjsonlink;

          }

 QString MyJsonTools::medialink(){

        // QString myjsonlink ="https://deliva.stratetactical.com/deliva/media/";
        QString myjsonlink ="http://127.0.0.1:8180/deliva/media/";
           return myjsonlink;

      }

 bool MyJsonTools::hasexpired(QString startdate) {


    QDateTime mycurrent = QDateTime::currentDateTime();
    QString thedate = mycurrent.toString(Qt::ISODate);
    QDateTime raw = QDateTime::fromString(startdate,Qt::ISODate);
    QDateTime rawdatetime = raw.toTimeZone(mycurrent.timeZone());
    QString isodate = rawdatetime.toString(Qt::ISODate);
    QDateTime beginDate =   QDateTime::fromString(isodate,Qt::ISODate);
    QDateTime expireDate =  QDateTime::fromString(thedate,Qt::ISODate);
    int y=0, m=0, d=0;
    bool isit = false;
    if(beginDate.daysTo(expireDate) >= 0)
    {
        y=expireDate.date().year()-beginDate.date().year();
        if((m=expireDate.date().month()-beginDate.date().month())<0)
        {
            y--;
            m+=12;
        }
        if((d=expireDate.date().day()-beginDate.date().day())<0)
        {
            if(--m < 0)
            {
                y--;
                m+=12;
            }
            d+=beginDate.date().daysInMonth();
        }

        if(y >0 ) {

            isit=true;
            qDebug() << "one year and later" <<Qt::endl;

        }

        else if(y<1 && m ==1 && d<1) {

            isit=false;
             qDebug() << "exactly a month" <<Qt::endl;

        }



        else if(y<1 && m <1 ) {

             isit=false;
              qDebug() << "less than a month so subscription active" <<Qt::endl;

        }



        else {

            isit=true;
        }


        qDebug()<<y<<m<<d;

      }


    return isit;



}



 QString MyJsonTools::tolocaltime(QString isodate) {

     QDateTime mycurrent = QDateTime::currentDateTime();
     QDateTime raw = QDateTime::fromString(isodate,Qt::ISODate);
     QDateTime rawdatetime = raw.toTimeZone(mycurrent.timeZone());
     QString localtime = rawdatetime.toString();
     return localtime;



}


 QString MyJsonTools::dayselapsed(QString startdate) {


    QDateTime mycurrent = QDateTime::currentDateTime();
    QString thedate = mycurrent.toString(Qt::ISODate);
    QDateTime raw = QDateTime::fromString(startdate,Qt::ISODate);
    QDateTime rawdatetime = raw.toTimeZone(mycurrent.timeZone());
    QString isodate = rawdatetime.toString(Qt::ISODate);
    QDateTime beginDate =   QDateTime::fromString(isodate,Qt::ISODate);
    QDateTime expireDate =  QDateTime::fromString(thedate,Qt::ISODate);
    int y=0, m=0, d=0;
    if(beginDate.daysTo(expireDate) >= 0)
    {
        y=expireDate.date().year()-beginDate.date().year();
        if((m=expireDate.date().month()-beginDate.date().month())<0)
        {
            y--;
            m+=12;
        }
        if((d=expireDate.date().day()-beginDate.date().day())<0)
        {
            if(--m < 0)
            {
                y--;
                m+=12;
            }
            d+=beginDate.date().daysInMonth();
        }




        qDebug()<<y<<m<<d;

      }


    QString year = QString::number(y);
    QString month = QString::number(m);
    QString days = QString::number(d);
    QString result = year +" years, " +month +" months, "+days+ " days " +" has elapsed since you joined us, thankyou.";
    return result;





}




std::string MyJsonTools::date_time() {


       QDateTime mycurrent = QDateTime::currentDateTime();
       QString thedate = mycurrent.toString(Qt::ISODate);
       return thedate.toStdString();

   }

void MyJsonTools::requestDone(Wt::AsioWrapper::error_code ec, const Wt::Http::Message &msg)
{


    if (!ec) {
        Wt::WStringStream ss;
        ss << "Status code " << msg.status() << "\n\n"
           << msg.body();
        qDebug() <<"status code is: " << QString::fromStdString(ss.str()) << Qt::endl;
      } else {

  qDebug() <<"error is: " << QString::fromStdString(Wt::utf8("Error: {1}").arg(ec.message()).toUTF8()) << Qt::endl;


      }



}


std::string MyJsonTools::fromIstream(std::istream &stream)
  {
      std::istreambuf_iterator<char> eos;
      return std::string(std::istreambuf_iterator<char>(stream), eos);
  }


QJsonObject MyJsonTools::ObjectFromString(const QString& in)
   
   {
       QJsonObject obj;

       QJsonDocument doc = QJsonDocument::fromJson(in.toUtf8());

       // check validity of the document
       if(!doc.isNull())
       {
           if(doc.isObject())
           {
               obj = doc.object();
             //  qDebug() << "Valid object" << Qt::endl;
           }
           else
           {
               qDebug() << "Document is not an object" << Qt::endl;
           }
       }
       else
       {
           qDebug() << "Invalid JSON...\n" << in << Qt::endl;
       }

       return obj;
   }


 QString MyJsonTools::jwttoken(QVariantMap ericmap,QString secret) {

    eric::JsonUtils m;
    QString jsonstr = m.toJsonString(ericmap);
 //   qDebug() << "Json String is: " << jsonstr << Qt::endl;
    QJsonWebToken m_jwtObj;
    m_jwtObj.setPayloadQStr(jsonstr);
    m_jwtObj.setSecret(secret);
    m_jwtObj.setAlgorithmStr("HSSha3_512");
    QString mytoken = m_jwtObj.getToken();
  //  qDebug() <<"Token is: " << mytoken <<Qt::endl;




    return mytoken;


}





   QString  MyJsonTools::extracttoken(QString jwt,QString user,QString mysecret) {




       QJsonDocument doc = QJsonDocument::fromJson(jwt.toUtf8());
     //  qDebug() << "doc :" << doc;
       QJsonArray myarr = doc.array();
       QString singletoken;
       Q_FOREACH(const QJsonValue & value, myarr) {
           QJsonObject obj = value.toObject();
           singletoken =obj["token"].toString();


       }
       QString thetoken = singletoken;
       QString strToken = thetoken;
       QStringList listJwtParts = strToken.split(".");
       bool tokenformatcorrect = true;
       bool isvalidtoken = true;
       if (listJwtParts.count() != 3)
          {
          // token format error
          tokenformatcorrect = false;



               }

       QString strSecret =mysecret;
       if (strSecret.isEmpty())
                {

           isvalidtoken=false;


                 }

    QJsonWebToken token = QJsonWebToken::fromTokenAndSecret(strToken, strSecret);
    // get decoded header and payload
    // QString strHeader = token.getHeaderQStr();
     QString strPayload = "";
   //  QString jsonobj ="";
     if (token.isValid() && tokenformatcorrect && isvalidtoken)
          {

         strPayload = token.getPayloadQStr();
       //  isvalidtoken=true;


           }

  //   qDebug() <<"payload is: " << strPayload << Qt::endl;

     QJsonObject myobj = ObjectFromString(strPayload);
     QString finaltoken = myobj.value(user).toString();
   //  qDebug() <<"Token is: " << finaltoken << Qt::endl;


     return finaltoken;








   }




   void MyJsonTools::modifyJsonValue(QJsonValue &destValue, const QString& path, const QJsonValue& newValue)
   {
       const int indexOfDot = path.indexOf('.');
       const QString dotPropertyName = path.left(indexOfDot);
       const QString dotSubPath = indexOfDot > 0 ? path.mid(indexOfDot + 1) : QString();

       const int indexOfSquareBracketOpen = path.indexOf('[');
       const int indexOfSquareBracketClose = path.indexOf(']');

       const int arrayIndex = path.mid(indexOfSquareBracketOpen + 1, indexOfSquareBracketClose - indexOfSquareBracketOpen - 1).toInt();

       const QString squareBracketPropertyName = path.left(indexOfSquareBracketOpen);
       const QString squareBracketSubPath = indexOfSquareBracketClose > 0 ? (path.mid(indexOfSquareBracketClose + 1)[0] == '.' ? path.mid(indexOfSquareBracketClose + 2) : path.mid(indexOfSquareBracketClose + 1)) : QString();

       // determine what is first in path. dot or bracket
       bool useDot = true;
       if (indexOfDot >= 0) // there is a dot in path
       {
           if (indexOfSquareBracketOpen >= 0) // there is squarebracket in path
           {
               if (indexOfDot > indexOfSquareBracketOpen)
                   useDot = false;
               else
                   useDot = true;
           }
           else
               useDot = true;
       }
       else
       {
           if (indexOfSquareBracketOpen >= 0)
               useDot = false;
           else
               useDot = true; // actually, id doesn't matter, both dot and square bracket don't exist
       }

       QString usedPropertyName = useDot ? dotPropertyName : squareBracketPropertyName;
       QString usedSubPath = useDot ? dotSubPath : squareBracketSubPath;

       QJsonValue subValue;

       if (destValue.isArray()) {

           subValue = destValue.toArray()[usedPropertyName.toInt()];
        //    qDebug() << "Json Array is " << destValue;

       }

       else if (destValue.isObject()) {

       subValue = destValue.toObject()[usedPropertyName];
      /* QStringList m_list;
       QJsonObject ericrequest = destValue.toObject();
       qx::cvt::from_json(ericrequest.value("columns"), m_list);
       for (int i = 0; i < m_list.size(); ++i) {
              std::cout <<"m_list: " << m_list.at(i).toLocal8Bit().constData() << std::endl;

       }  */

      // qDebug() << "Json Object is " << destValue;

       }

       else {


          qDebug() << "oh, what should i do now with the following value? " << destValue;
       }

       if(usedSubPath.isEmpty())
       {
           subValue = newValue;
       }
       else
       {
           if (subValue.isArray())
           {
               QJsonArray arr = subValue.toArray();
               QJsonValue arrEntry = arr[arrayIndex];
               modifyJsonValue(arrEntry,usedSubPath,newValue);
               arr[arrayIndex] = arrEntry;
               subValue = arr;
           }
           else if (subValue.isObject())
               modifyJsonValue(subValue,usedSubPath,newValue);
           else
               subValue = newValue;
       }

       if (destValue.isArray())
       {
           QJsonArray arr = destValue.toArray();
           if (subValue.isNull())
               arr.removeAt(arrayIndex);
           else
               arr[arrayIndex] = subValue;
           destValue = arr;
       }
       else if (destValue.isObject())
       {
           QJsonObject obj = destValue.toObject();
           if (subValue.isNull())
               obj.remove(usedPropertyName);
           else
               obj[usedPropertyName] = subValue;
           destValue = obj;
       }
       else
           destValue = newValue;
   }

   void MyJsonTools::modifyJsonValue(QJsonDocument& doc, const QString& path, const QJsonValue& newValue)
   {
       QJsonValue val;
       if (doc.isArray())
           val = doc.array();
       else
           val = doc.object();

       modifyJsonValue(val,path,newValue);

       if (val.isArray())
           doc = QJsonDocument(val.toArray());
       else
           doc = QJsonDocument(val.toObject());
   }


   QString  MyJsonTools::extractsinglevalue(QString json,QString key) {




       QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
     //  qDebug() << "doc :" << doc;
       QJsonArray myarr = doc.array();
       QString singleval="none";
       Q_FOREACH (const QJsonValue & value, myarr) {
           QJsonObject obj = value.toObject();
           singleval =obj[key].toString();


       }

       return singleval;

   }



   void MyJsonTools::resetadmin( QJsonDocument &modifymydoc,std::shared_ptr<MyJsonTools> modifymyMyJsonTools,int role,QString exclusion) {



       if(role<=2) {

       if(exclusion=="business") {


       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.account", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.roles", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.blacklist", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.verification", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.responsibilities", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.subscription", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.authority", "later");
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.logistics", "later");

       }

       else if(exclusion=="account") {

      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.business","later");
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.roles", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.blacklist", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.verification", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.responsibilities", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.subscription", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.authority", "later");
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.logistics", "later");


       }

       else if(exclusion=="roles") {

      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.business","later");
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.account", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.blacklist", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.verification", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.responsibilities", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.subscription", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.authority", "later");
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.logistics", "later");

       }

       else if(exclusion =="blacklist") {

       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.business","later");
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.account", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.roles", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.verification", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.responsibilities", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.subscription", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.authority", "later");
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.logistics", "later");

       }

       else if(exclusion=="verification") {

       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.business","later");
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.account", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.roles", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.blacklist", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.responsibilities", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.subscription", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.authority", "later");
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.logistics", "later");

       }

       else if(exclusion=="responsibilities") {

       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.business","later");
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.account", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.roles", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.blacklist", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.verification", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.subscription", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.authority", "later");
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.logistics", "later");


       }

       else if(exclusion=="subscription") {

       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.business","later");
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.account", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.roles", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.blacklist", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.verification", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.responsibilities", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.authority", "later");
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.logistics", "later");

       }

       else if(exclusion=="authority") {

      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.business","later");
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.account", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.roles", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.blacklist", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.verification", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.responsibilities", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.subscription", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.logistics", "later");

       }

       else if(exclusion=="logistics") {

      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.business","later");
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.account", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.roles", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.blacklist", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.verification", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.responsibilities", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.subscription", 0);
      modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.authority", "later");



       }

       else {


       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.business","later");
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.account", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.roles", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.blacklist", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.verification", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.responsibilities", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.subscription", 0);
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.authority", "later");
       modifymyMyJsonTools->modifyJsonValue(modifymydoc, "data.logistics", "later");

       }

       }

       else if(role==3) {


        }


       else if(role ==4) {

       }

       else {

       }


   }


   QString MyJsonTools::fileNameFromPath(const QString & filePath) const {
      return QFileInfo(filePath).fileName();

    }
      QString MyJsonTools::fileSuffixFromPath(const QString & filePath) const {
        return QFileInfo(filePath).suffix();
      }
     qint64  MyJsonTools::fileSizeFromPath(const QString & filePath) const {
          qint64 size = 0;
          if(QFileInfo(filePath).size()==0){

            //  QUrl eric = QUrl::fromLocalFile(filePath);
              QUrl eric(filePath);
              QString relative = eric.toString(QUrl::PreferLocalFile);
              qDebug()<< "relative is: " << relative;
              size = QFileInfo(relative).size();


             // size =
          }

          else{


              size = QFileInfo(filePath).size();
          }
        return size;
      }

      QString  MyJsonTools::filePath(const QString & filePath) const {
          QUrl eric(filePath);
          QString relative = eric.toString(QUrl::PreferLocalFile);
          qDebug()<< "relative is: " << relative;
          return relative;
       }


       QString MyJsonTools::sanitizeFileName(const QString &name, const QString &replacement)
       {
           if (name.isEmpty())
               return name;

           QString s = name;

           // Replace forbidden characters (Windows) and also clean cross-platform ugly chars
           static const QRegularExpression forbiddenPattern(R"([<>:"/\\|?*\x00-\x1F])");
           s.replace(forbiddenPattern, replacement);

           // Collapse whitespace to a single underscore
           static const QRegularExpression spaceRe(R"(\s+)");
           s.replace(spaceRe, "_");

           // Collapse multiple replacement characters (e.g. "----") to single replacement
           QString repEsc = QRegularExpression::escape(replacement);
           s.replace(QRegularExpression(repEsc + "{2,}"), replacement);

           // Trim leading/trailing dots and spaces (Windows hates them)
           static const QRegularExpression leadTrail(R"(^[\. ]+|[\. ]+$)");
           s.remove(leadTrail);

           if (s.isEmpty())
               s = "file";

           return s;
       }

       bool MyJsonTools::normalizeSpooledPath(const std::string &raw, QString &outPath)
       {
           outPath.clear();

           // Remove surrounding quotes
           std::string s = raw;
           if (!s.empty() && s.front() == '"' && s.back() == '"') {
               s = s.substr(1, s.size() - 2);
           }

           // Percent-decode
           QByteArray encoded = QByteArray::fromStdString(s);
           QString decoded = QUrl::fromPercentEncoding(encoded);

           // Normalize slashes & clean path
           decoded.replace('\\', '/');
           decoded = QDir::cleanPath(decoded);

           if (decoded.isEmpty())
               return false;

           outPath = decoded;
           return true;
       }


       // bool MyJsonTools::moveOrCopySpooledFile(const std::string &rawSpool,
       //                            const QString &destDir,
       //                            const QString &destFilenameRaw,
       //                            QString &outFinalPath,
       //                            QString &outError)
       // {
       //     outFinalPath.clear();
       //     outError.clear();

       //     // normalize spool path (your existing function)
       //     QString spoolPath;
       //     if (!normalizeSpooledPath(rawSpool, spoolPath)) {
       //         outError = "Failed to normalize spool path";
       //         return false;
       //     }

       //     // sanitize filename (important for Windows)
       //     QString destFilename = sanitizeFileName(destFilenameRaw, "-");

       //     // ensure dest dir exists
       //     QDir().mkpath(destDir);
       //     QString destPath = QDir(destDir).filePath(destFilename);

       //     qInfo() << "Normalized spool:" << spoolPath;
       //     qInfo() << "Destination (sanitized):" << destPath;

       //     if (!QFile::exists(spoolPath)) {
       //         outError = QString("Spooled file does not exist: %1").arg(spoolPath);
       //         return false;
       //     }

       //     // If destination already exists, try to remove it first (or you can choose to fail)
       //     if (QFile::exists(destPath)) {
       //         qWarning() << "Destination file already exists, attempting to remove:" << destPath;
       //         if (!QFile::remove(destPath)) {
       //             qWarning() << "Failed to remove existing destination file. QFile::error()=" << QFile(destPath).error();
       //             // Not fatal — we can still try rename/copy which will likely fail, but return helpful error later.
       //         }
       //     }

       //     // Attempt rename (fast, atomic when same FS)
       //     if (QFile::rename(spoolPath, destPath)) {
       //         outFinalPath = destPath;
       //         return true;
       //     }

       //     // If rename failed, try std::rename to get errno for diagnostics
       //     {
       //         int r = std::rename(spoolPath.toStdString().c_str(), destPath.toStdString().c_str());
       //         if (r == 0) {
       //             outFinalPath = destPath;
       //             return true;
       //         } else {
       //             int err = errno;
       //             qWarning() << "std::rename failed with errno =" << err << " (" << QString::fromUtf8(strerror(err)) << ")";
       //         }
       //     }

       //     // Fallback: copy + remove original
       //     if (QFile::copy(spoolPath, destPath)) {
       //         if (!QFile::remove(spoolPath)) {
       //             qWarning() << "Copied but failed to remove original spool file:" << spoolPath << "error =" << QFile(spoolPath).error();
       //             // not fatal, destination exists so treat as success
       //         }
       //         outFinalPath = destPath;
       //         return true;
       //     }

       //     // Final attempt: stream copy (binary-safe)
       //     {
       //         std::ifstream in(spoolPath.toStdString(), std::ios::binary);
       //         if (!in) {
       //             outError = QString("Failed to open spool file for reading: %1").arg(spoolPath);
       //             return false;
       //         }
       //         std::ofstream out(destPath.toStdString(), std::ios::binary);
       //         if (!out) {
       //             outError = QString("Failed to open destination for writing: %1").arg(destPath);
       //             return false;
       //         }
       //         out << in.rdbuf();
       //         out.close();
       //         in.close();
       //         if (!QFile::remove(spoolPath)) {
       //             qWarning() << "Stream-copied but could not remove original spool file:" << spoolPath;
       //         }
       //         outFinalPath = destPath;
       //         return true;
       //     }

       //     // unreachable, but keep for clarity
       //     outError = "Unknown error moving/copying spool file";
       //     return false;
       // }



       bool MyJsonTools::moveOrCopySpooledFile(const std::string &rawSpool,
                                               const QString &destDir,
                                               const QString &destFilenameRaw,
                                               QString &outFinalPath,
                                               QString &outError)
       {
           outFinalPath.clear();
           outError.clear();

           // 1) Normalize spool path (must be implemented elsewhere)
           QString spoolPath;

           if (!normalizeSpooledPath(rawSpool, spoolPath)) {
               outError = "Failed to normalize spool path from raw: " + QString::fromStdString(rawSpool);
               qWarning() << outError;
               return false;
           }

           // 2) Sanitize destination filename (must be implemented elsewhere)
          QString destFilename = sanitizeFileName(destFilenameRaw, "-");

           // 3) Ensure destination directory exists
           if (!QDir(destDir).exists()) {
               if (!QDir().mkpath(destDir)) {
                   outError = QString("Failed to create destination directory: %1").arg(destDir);
                   qWarning() << outError;
                   return false;
               }
           }

           QString destPath = QDir(destDir).filePath(destFilenameRaw);

           qInfo() << "Attempting to save spooled file:";
           qInfo() << "  raw spool   :" << QString::fromStdString(rawSpool);
           qInfo() << "  spoolNorm   :" << spoolPath;
           qInfo() << "  dest dir    :" << destDir;
           qInfo() << "  dest path   :" << destPath;

           // 4) Source existence and readability check
           if (!QFile::exists(spoolPath)) {
               outError = QString("Spooled file does not exist at '%1'").arg(spoolPath);
               qWarning() << outError;
               return false;
           }

           QFile srcTest(spoolPath);
           if (!srcTest.open(QIODevice::ReadOnly)) {
               outError = QString("Cannot open spool file for reading: %1 (QFile error=%2, errstr=%3)")
               .arg(spoolPath)
                   .arg(srcTest.error())
                   .arg(srcTest.errorString());
               qWarning() << outError;
               qWarning() << "errno:" << errno << "(" << (errno ? QString::fromUtf8(strerror(errno)) : "") << ")";
               return false;
           }
           qint64 srcSize = srcTest.size();
           srcTest.close();

           qInfo() << "Source exists and is readable. size=" << srcSize;

           // 5) Destination writability test (scratch file)
           QDir d(destDir);
           QString scratch = d.filePath(".qx_write_test");
           {
               QFile s(scratch);
               bool wrote = false;
               if (s.open(QIODevice::WriteOnly)) {
                   s.write("x");
                   s.close();
                   // try remove the scratch file
                   wrote = s.remove();
               } else {
                   qWarning() << "Cannot write scratch file to dest dir:" << destDir << "err:" << s.error() << s.errorString();
               }
               if (!wrote) {
                   outError = QString("Destination directory not writable: %1").arg(destDir);
                   qWarning() << outError;
                   return false;
               }
           }

           // 6) If destination exists, attempt to remove it (overwrite behavior)
           if (QFile::exists(destPath)) {
               if (!QFile::remove(destPath)) {
                   qWarning() << "Could not remove existing destination file:" << destPath
                              << "error=" << QFile(destPath).error()
                              << QFile(destPath).errorString();
                   // continue and attempt rename/copy — but warn
               } else {
                   qInfo() << "Existing destination removed:" << destPath;
               }
           }

           // 7) Try atomic rename (fast on same FS)
           if (QFile::rename(spoolPath, destPath)) {
               outFinalPath = destPath;
               qInfo() << "QFile::rename succeeded";
               return true;
           }
           qWarning() << "QFile::rename failed. QFile::error()=" << QFile(spoolPath).error()
                      << QFile(spoolPath).errorString();

           // 8) Try std::rename to get errno details
           {
               int ren = std::rename(spoolPath.toLocal8Bit().constData(), destPath.toLocal8Bit().constData());
               if (ren == 0) {
                   outFinalPath = destPath;
                   qInfo() << "std::rename succeeded";
                   return true;
               } else {
                   qWarning() << "std::rename failed errno=" << errno << "(" << QString::fromUtf8(strerror(errno)) << ")";
               }
           }

           // 9) Try QFile::copy
           if (QFile::copy(spoolPath, destPath)) {
               qInfo() << "QFile::copy succeeded";
               if (!QFile::remove(spoolPath)) {
                   qWarning() << "Copied but could not remove original spool file:" << spoolPath
                              << "error=" << QFile(spoolPath).error()
                              << QFile(spoolPath).errorString();
                   // not fatal
               } else {
                   qInfo() << "Original spool file removed after copy.";
               }
               outFinalPath = destPath;
               return true;
           } else {
               qWarning() << "QFile::copy failed. QFile::error()=" << QFile(spoolPath).error()
               << QFile(spoolPath).errorString();
           }

           // 10) Streaming fallback using QFile (chunked copy)
           {
               QFile inFile(spoolPath);
               if (!inFile.open(QIODevice::ReadOnly)) {
                   outError = QString("Stream fallback: cannot open spool for reading: %1 (err=%2)").arg(spoolPath).arg(inFile.error());
                   qWarning() << outError << inFile.errorString();
                   return false;
               }
               QFile outFile(destPath);
               if (!outFile.open(QIODevice::WriteOnly)) {
                   outError = QString("Stream fallback: cannot open destination for writing: %1 (err=%2)").arg(destPath).arg(outFile.error());
                   qWarning() << outError << outFile.errorString();
                   inFile.close();
                   return false;
               }

               constexpr qint64 bufSize = 64 * 1024;
               QByteArray buffer;
               buffer.resize(bufSize);
               qint64 total = 0;
               bool streamError = false;

               while (!inFile.atEnd()) {
                   qint64 read = inFile.read(buffer.data(), buffer.size());
                   if (read <= 0) {
                       qWarning() << "Read error while streaming spool. read=" << read << " error=" << inFile.error();
                       streamError = true;
                       break;
                   }
                   qint64 written = outFile.write(buffer.constData(), read);
                   if (written != read) {
                       qWarning() << "Write error while streaming spool. written=" << written << " expected=" << read << " error=" << outFile.error();
                       streamError = true;
                       break;
                   }
                   total += written;
               }

               outFile.flush();
               outFile.close();
               inFile.close();

               if (!streamError && QFile::exists(destPath) && QFileInfo(destPath).size() == total) {
                   // Remove original if possible
                   if (!QFile::remove(spoolPath)) {
                       qWarning() << "Stream-copied but could not remove original spool file:" << spoolPath
                                  << "error=" << QFile(spoolPath).error()
                                  << QFile(spoolPath).errorString();
                   } else {
                       qInfo() << "Stream-copied and removed original spool file.";
                   }
                   outFinalPath = destPath;
                   qInfo() << "Stream copy saved bytes =" << total;
                   return true;
               } else {
                   outError = QString("Stream fallback failed; dst exists? %1 size=%2 expected=%3")
                   .arg(QFile::exists(destPath))
                       .arg(QFile::exists(destPath) ? QString::number(QFileInfo(destPath).size()) : "-")
                       .arg(total);
                   qWarning() << outError;
                   if (QFile::exists(destPath))
                       QFile::remove(destPath);
                   return false;
               }
           }

           // 11) Unreachable, but keep final failure
           outError = "Unknown failure saving spooled file";
           return false;
       }


  bool MyJsonTools::renamefile(QString pathtooldfilename, QString pathtonewfilename) {


          bool results = false;
          QFile readFile(pathtonewfilename);
          bool ispresent = readFile.exists();
          if(ispresent) {

             bool ok =readFile.remove();
             if(ok){

                 qDebug()<<"deleted existing file" ;
             }
             else {

                 qDebug()<<"unable to delete existing file" ;

             }

           }

          else {

              qDebug()<<"proceed to rename";

          }

          if(QFile::rename(pathtooldfilename, pathtonewfilename)) {

              qDebug()<<"file was renamed successfully" << Qt::endl;
              results = true;

          }

          else {

               qDebug()<<"file was not renamed" << Qt::endl;
               results = false;
          }

          return results;

      }


 QString MyJsonTools::filelocalPath(const QString & filePath) const {

         return QFileInfo(filePath).path();

       }

  QString MyJsonTools::dirname(const QString & filePath) const {

         return QFileInfo(filePath).filePath();

       }



  QString MyJsonTools::convertUrlToFilePath(const QUrl &url) {

        // This function convert the url passed from qml in a valid resource path.
        if (url.scheme().compare(QLatin1String("qrc"), Qt::CaseInsensitive) == 0) {
          if (url.authority().isEmpty()) {
            return QLatin1Char(':') + url.path();
          }
        }
        return QString();

      }

  QString MyJsonTools::extractstring(QString firstword,QString criteria) {


          QRegularExpression eric;
          QString results="";
          eric.setPattern("(?<="+firstword+").*");
           QRegularExpressionMatch match = eric.match(criteria);
           if (match.hasMatch()) {
                 QString matched = match.captured(0);
                 qDebug()<<"match is:" << matched;
                 results=matched;
             }


          return results;



      }

    bool MyJsonTools::testextractstring(QString firstword,QString criteria) {


           QRegularExpression eric;
           bool results=false;
           eric.setPattern("(?<="+firstword+").*");
            QRegularExpressionMatch match = eric.match(criteria);
            if (match.hasMatch()) {

                  results=true;
              }


           return results;



       }


    bool MyJsonTools::copyanyfile(QString copydstfile,QString copysrcfile )
     {




             QFile readFile(copydstfile);
             bool ispresent = readFile.exists();
             if(ispresent) {

                bool ok =readFile.remove();
                if(ok){

                    qDebug()<<"deleted existing file" ;
                }
                else {

                    qDebug()<<"unable to delete existing file" ;
                    return false;
                }

              }

             else {

                 qDebug()<<"proceed to copy";

             }

             if(!readFile.exists()) {
                 qDebug() << "image data path does not exist: " << copydstfile;
                 QFile srcFile( copysrcfile);
                 if(srcFile.exists()) {
                     // copy file from src to data path
                     bool copyOk = srcFile.copy(copydstfile);
                     if (!copyOk) {
                         qDebug() << "cannot copy file from src to data path";
                         return false;
                     }

                     copyOk = readFile.setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser);
                     if (!copyOk) {
                         qDebug() << "cannot set Permissions to read / write settings";
                         return false;
                     }
                 } else {
                     qDebug() << "nothing in src " << copysrcfile;
                     return false;
                 }
             }
             if (!readFile.open(QIODevice::ReadOnly)) {
                 qWarning() << "Couldn't open file: " << copydstfile;
                 return false;
             }

             return true;
     }

  bool MyJsonTools::rmDir(const QString &dirPath)
    {
        QDir dir(dirPath);
        if (!dir.exists())
            return true;
       Q_FOREACH(const QFileInfo &info, dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
            if (info.isDir()) {
                if (!rmDir(info.filePath()))
                    return false;
            } else {
                if (!dir.remove(info.fileName()))
                    return false;
            }
        }
        QDir parentDir(QFileInfo(dirPath).path());
        return parentDir.rmdir(QFileInfo(dirPath).fileName());
    }

 void MyJsonTools::cleardir(QString dirname) {


        rmDir(dirname);

    }

    bool MyJsonTools::checkSomeDirs(QString folder)
       {
           QDir myDir;
           bool existsimgs;
           bool existsvids;
           QString mDataImagesPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0)+"/images/photos";
           QString mDataVideosPath = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0)+"/videos/movies";

           existsimgs = myDir.exists(mDataImagesPath);
           if (!existsimgs) {
               bool ok = myDir.mkpath(mDataImagesPath);
               if(!ok) {
                   qWarning() << "Couldn't create mDataImagesPath " <<mDataImagesPath;
                   return false;
               }
               qDebug() << "created directory mDataImagesPath " << mDataImagesPath;
           }

           existsvids = myDir.exists(mDataVideosPath);
           if (!existsvids) {
               bool ok = myDir.mkpath(mDataVideosPath);
               if(!ok) {
                   qWarning() << "Couldn't create mDataVideosPath " << mDataVideosPath;
                   return false;
               }
               qDebug() << "created directory mDataVideosPath " << mDataVideosPath;
           }

           if(!(folder=="")){

               bool existsfolder;
               QString somePath=QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0)+"/"+folder;
               existsfolder = myDir.exists(somePath);
               if (!existsfolder) {
                   bool ok = myDir.mkpath(somePath);
                   if(!ok) {
                       qWarning() << "Couldn't create somePath " <<somePath;
                       return false;
                   }
                   qDebug() << "created directory somePath " << somePath;
               }



           }

           return true;
       }


    bool MyJsonTools::checkAnyDir(QString folder,QString dir)
       {
           QDir myDir;
           bool existsdir;
           existsdir = myDir.exists(dir);
           if (!existsdir) {
               bool ok = myDir.mkpath(dir);
               if(!ok) {
                   qWarning() << "Couldn't create main directory " <<dir;
                   return false;
               }
               qDebug() << "created main directory " << dir;
           }



           if(!(folder=="")){

               bool existsfolder;
               QString somePath=dir+folder;
               existsfolder = myDir.exists(somePath);
               if (!existsfolder) {
                   bool ok = myDir.mkpath(somePath);
                   if(!ok) {
                       qWarning() << "Couldn't create somePath " <<somePath;
                       return false;
                   }
                   qDebug() << "created directory somePath " << somePath;
               }



           }

           return true;
       }

    void MyJsonTools::createanydir(QString folder,QString dir){


          bool okdir = checkAnyDir(folder,dir);
                      if(!okdir) {
                          qFatal("Directory could not be created for some reasons");
                      }


      }

  void MyJsonTools::createdir(QString folder){


        bool okdir = checkSomeDirs(folder);
                    if(!okdir) {
                        qFatal("Directory could not be created for some reasons");
                    }


    }



  QString MyJsonTools::newuuid() {

    QUuid myuuid = QUuid::createUuid();

    return myuuid.toString(QUuid::WithoutBraces);



   }

  QString MyJsonTools::longdatetime() {

  QDateTime mydatetime = QDateTime::currentDateTime();
  QDateTime UTC(mydatetime.toUTC());
  QString thedate = UTC.toString(Qt::ISODate);

  return mydatetime.toString("dd.MM.yyyy-hh:mm:ss.zzz-")+thedate;

 // return thedate;


  }


  QString MyJsonTools::newfilename() {

   // return newuuid()+"-"+longdatetime();
   return newuuid();


  }


  bool MyJsonTools::deleteanyfile(QString targetfile)
   {


      bool result = false;

           QFile readFile(targetfile);
           bool ispresent = readFile.exists();
           if(ispresent) {

              bool ok =readFile.remove();
              if(ok){

                  qDebug()<<"deleted existing file" ;
                  result = true;
              }
              else {

                  qDebug()<<"unable to delete existing file" ;
                  result = false;
              }

            }

           else {

               qDebug()<<"no such file or directory!";
               result = false;

           }


           return result;


     }

// 1 minute = 60,000 millisecond




