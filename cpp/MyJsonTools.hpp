#ifndef MYJSONTOOLS_HPP
#define MYJSONTOOLS_HPP

#include "precompiled.h"

using namespace CryptoPP;

class MyJsonTools : public QObject
{
    Q_OBJECT

public:
    MyJsonTools();
    ~MyJsonTools();

QJsonValue m_requestJson;              //!< Request which contains all parameters (as JSON format)
QJsonValue m_responseJson;             //!< Response after executing the request (as JSON format)
QJsonValue m_dataJson;                 //!< Data used to process request (as JSON format)
QJsonValue m_errorJson;
QString m_requestId;                            //!< Request identifier (GUID for example)
QString m_entity;                               //!< Entity where to process request
QString m_action;                               //!< Action to execute (count, fetch_by_id, fetch_all, fetch_by_query, insert, update, etc...)
QString m_function;  //!< Entity static function to call with action 'call_entity_function' and signature : static QJsonValue myEntity::myFct(const QJsonValue & request)
QStringList m_columns;                          //!< Columns to fetch
QStringList m_relations;                        //!< Relations to fetch
QStringList m_outputFormat;                     //!< JSON output format (same syntax as relations)
QString m_database;                             //!< Database to use to process request
QString m_query;                                //!< SQL query to execute
QString m_data;                                 //!< Data used to process request
QSqlError m_error;                              //!< Error after executing the request
//qx::IxPersistable_ptr m_instance;               //!< Current instance to execute request
QSqlDatabase m_db;                              //!< Current database to execute request
qx_query m_qxQuery;                             //!< Query used by some actions
long m_countResult;                             //!< Result after a count query
qx_bool m_existResult;                          //!< Result after a exist query
qx::QxInvalidValueX m_validateResult;
qx::dao::save_mode::e_save_mode m_eSaveMode;    //!< Save mode for 'save' action
bool m_bUseExecBatch;

// Use temporary QStrings to build the paths correctly
const QString baseDownloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

// Build the full paths using QString and assign directly to QString
const QString uploadPath = baseDownloadPath + "/Orders_files/App/media";
const QString logsPath = baseDownloadPath + "/Orders_files/App/logs";
const QString uploadTempPath = baseDownloadPath + "/Orders_files/App/tempuploads";
const QString tokenPath = baseDownloadPath + "/Orders_files/App/approot/";

// Convert to std::string when necessary
const std::string UPLOAD_DIRIDS = uploadPath.toStdString();
const std::string LOGS_ROOTIDS = logsPath.toStdString();
const std::string UPLOAD_TEMPDIRIDS = uploadTempPath.toStdString();
const std::string TOKEN_DIRIDS = tokenPath.toStdString();
bool ensureDirectoriesExist();
void buildError(int errCode, const QString & errDesc);
void buildError(const QSqlError & error);
QString processRequest(const QString & request);
QString processRequest(const QByteArray & request);
void setData(const QJsonValue & data);
QJsonValue processRequest(const QJsonValue & request);
QJsonValue processRequestAsArray(const QJsonValue & request);
void clear();
void resetRequest();
bool parseRequest(const QString & request);
bool parseRequest(const QByteArray & request);
bool decodeRequest();
bool checkRequest();
bool createInstance();
bool buildResponse();
bool formatResponse();
bool executeAction();
bool doRequest();
QString queryparam();
qint64 rng();
std::string  doupload(QByteArray data,std::string tmpdir,std::string uploaddir,long size,std::string type
                      ,bool save,bool update = false,std::string previousfile="");
QVariantMap encryptpass(std::string mypass);
bool decryptpass(std::string mypass,QVariantMap crypto);
std::string recoverpass(QVariantMap crypto);
QDateTime duedate();
QString utctime();
QString mylocaltime(QString utc,QString fmt);
bool writeJsonFile(QVariantMap point_map, QString file_path);
QVariantMap readJsonFile(QString file_path);
QString datapath();
QString convertUrlToFilePath(const QString & filePath);
QString base64encode(QString content);
QString base64decode(QString content);
QString jsondatapath();
QString jsonqrcpath();
bool copyjsonassets(QString mjsonPathfile,QString mjsonqrcfile );
QString fluidiconpath() const;
QString extractword(QString word,QString criteria);
QString jsonlink();
QString medialink();
bool hasexpired(QString startdate);
QString tolocaltime(QString isodate);
QString dayselapsed(QString startdate);
std::string date_time();
void requestDone(Wt::AsioWrapper::error_code ec, const Wt::Http::Message &msg);
std::string fromIstream(std::istream &stream);
QJsonObject ObjectFromString(const QString& in);
QString jwttoken(QVariantMap ericmap,QString secret);
QString  extracttoken(QString jwt="",QString user="",QString mysecret="");
void modifyJsonValue(QJsonValue &destValue, const QString& path, const QJsonValue& newValue = QJsonValue());
void modifyJsonValue(QJsonDocument& doc, const QString& path, const QJsonValue& newValue = QJsonValue());
QString  extractsinglevalue(QString json="",QString key="");
void resetadmin( QJsonDocument &modifymydoc,std::shared_ptr<MyJsonTools> modifymyMyJsonTools,int role=2,QString exclusion="none");
QString fileNameFromPath(const QString & filePath) const;
QString fileSuffixFromPath(const QString & filePath) const;
qint64  fileSizeFromPath(const QString & filePath) const;
QString  filePath(const QString & filePath) const;
QString sanitizeFileName(const QString &name, const QString &replacement = "-");
bool normalizeSpooledPath(const std::string &raw, QString &outPath);
bool moveOrCopySpooledFile(const std::string &rawSpool,
                                  const QString &destDir,
                                  const QString &destFilenameRaw,
                                  QString &outFinalPath,
                                  QString &outError);
bool renamefile(QString pathtooldfilename, QString pathtonewfilename);
QString filelocalPath(const QString & filePath) const;
QString dirname(const QString & filePath) const;
QString convertUrlToFilePath(const QUrl &url);
QString extractstring(QString firstword,QString criteria);
bool testextractstring(QString firstword,QString criteria);
bool copyanyfile(QString copydstfile,QString copysrcfile );
bool rmDir(const QString &dirPath);
void cleardir(QString dirname);
bool checkSomeDirs(QString folder);
bool checkAnyDir(QString folder,QString dir);
void createanydir(QString folder,QString dir);
void createdir(QString folder);
QString newuuid();
QString longdatetime();
QString newfilename();
bool deleteanyfile(QString targetfile);

};

#endif // MYJSONTOOLS_HPP
