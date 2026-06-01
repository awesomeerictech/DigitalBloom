#ifndef JSONTOOLS_HPP
#define JSONTOOLS_HPP
// JsonTools.cpp
#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QUrl>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QStandardPaths>
#include <QRandomGenerator>
#include <QElapsedTimer>
#include <QCryptographicHash>
#include <QDateTime>
#include <QTimeZone>
#include <QTextStream>
#include <QRegularExpression>
#include <QVariantMap>
#include <QJsonValue>
#include <QJsonArray>
#include <QCoreApplication>

#include "qjsonwebtoken.h"
#include "Net/imageloader.hpp" // <-- ensure this path matches your project
#include <QByteArray>

// Single-file JsonTools definition + implementation
class JsonTools : public QObject
{
    Q_OBJECT

public:
    explicit JsonTools(QObject *parent = nullptr)
        : QObject(parent)
        , mImageLoader(nullptr)
    {
    }

    ~JsonTools() override
    {
        if (mImageLoader) {
            mImageLoader->deleteLater();
            mImageLoader = nullptr;
        }
    }

public Q_SLOTS:
    void onImageUpdateLoaded(QObject * /*dataObject*/, int width, int height)
    {
        if (mImageLoader) {
            mImageLoader->deleteLater();
            mImageLoader = nullptr;
        }
        qDebug() << "width of image is:" << width << "and the Height of image is:" << height;
    }

    void onImageUpdateFailed(QObject * /*dataObject*/, const QString &message)
    {
        if (mImageLoader) {
            mImageLoader->deleteLater();
            mImageLoader = nullptr;
        }
        qDebug() << "error in downloading image is:" << message;
    }

public: // Q_INVOKABLE API (you can still register the object to QML)
    Q_INVOKABLE void downloadimage(const QString &imageUrl, const QString &pathtoimage)
    {
        // protect from double-creation / leaks
        if (mImageLoader) {
            mImageLoader->deleteLater();
            mImageLoader = nullptr;
        }

        mImageLoader = new ImageLoader(imageUrl, pathtoimage, this);

        // connect using pointer-to-member syntax (works with QT_NO_KEYWORDS)
        bool ok = QObject::connect(mImageLoader,
                                   &ImageLoader::loaded,
                                   this,
                                   &JsonTools::onImageUpdateLoaded);
        Q_ASSERT(ok);

        ok = QObject::connect(mImageLoader,
                              &ImageLoader::loadingFailed,
                              this,
                              &JsonTools::onImageUpdateFailed);
        Q_ASSERT(ok);

        // start loading (preserve original API)
        mImageLoader->loadImage(this);
    }

    Q_INVOKABLE QString queryparam() { return QStringLiteral("reason=testapi"); }

    Q_INVOKABLE qint64 rng()
    {
        auto gen = QRandomGenerator::system();
        return static_cast<qint64>(gen->bounded(10000));
    }

    Q_INVOKABLE QString roles(int role)
    {
        switch (role) {
        case 0: return QStringLiteral("onlyuser");
        case 1: return QStringLiteral("admin");
        case 2: return QStringLiteral("responsible");
        default: return QString();
        }
    }

    Q_INVOKABLE QString roleone() { return QStringLiteral("admin"); }
    Q_INVOKABLE QString roletwo() { return QStringLiteral("responsible"); }

    Q_INVOKABLE QString qrcriteria()
    {
        QString mycrit = QStringLiteral("attenddosaievents");
        QString base64 = base64encode(mycrit);
        qDebug() << "base 64 is:" << base64;
        return base64;
    }

    Q_INVOKABLE bool writeJsonFile(const QVariantMap &point_map, const QString &file_path)
    {
        QJsonObject json_obj = QJsonObject::fromVariantMap(point_map);
        QJsonDocument json_doc(json_obj);
        QByteArray json_bytes = json_doc.toJson();

        QFile save_file(file_path);
        if (!save_file.open(QIODevice::WriteOnly)) {
            qDebug() << "failed to open save file" << file_path;
            return false;
        }
        save_file.write(json_bytes);
        save_file.close();
        return true;
    }

    Q_INVOKABLE QVariantMap readJsonFile(const QString &file_path)
    {
        QVariantMap results;
        QFile file_obj(file_path);
        if (!file_obj.open(QIODevice::ReadOnly)) {
            qDebug() << "Failed to open:" << file_path;
            return results;
        }
        QTextStream file_text(&file_obj);
        QString json_string = file_text.readAll();
        file_obj.close();

        QByteArray json_bytes = json_string.toUtf8();
        QJsonDocument json_doc = QJsonDocument::fromJson(json_bytes);
        if (json_doc.isNull() || !json_doc.isObject()) {
            qDebug() << "Invalid or non-object JSON in" << file_path;
            return results;
        }
        QJsonObject json_obj = json_doc.object();
        results = json_obj.toVariantMap();
        return results;
    }

    Q_INVOKABLE QString datapath()
    {
        return QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0);
    }

    Q_INVOKABLE QString convertUrlToFilePath(const QString &filePath)
    {
        QUrl url(filePath);
        if (url.scheme().compare(QLatin1String("qrc"), Qt::CaseInsensitive) == 0) {
            if (url.authority().isEmpty()) {
                return QLatin1Char(':') + url.path();
            }
        }
        return QString();
    }

    Q_INVOKABLE QString base64encode(const QString &content)
    {
        return QString::fromUtf8(content.toUtf8().toBase64());
    }

    Q_INVOKABLE QString base64decode(const QString &content)
    {
        QByteArray arr = content.toUtf8();
        QByteArray dec = QByteArray::fromBase64(arr);
        return QString::fromUtf8(dec);
    }

    Q_INVOKABLE QString jsondatapath()
    {
        QString mDataRoot = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0);
        return mDataRoot + QDir::separator() + QStringLiteral("json") + QDir::separator();
    }

    Q_INVOKABLE QString jsonqrcpath() { return QStringLiteral(":/data-assets/json/"); }

    Q_INVOKABLE bool copyjsonassets(const QString &mjsonPathfile, const QString &mjsonqrcfile)
    {
        QFile readFile(mjsonPathfile);
        if (!readFile.exists()) {
            qDebug() << "config data path does not exist:" << mjsonPathfile;
            QFile qrcFile(mjsonqrcfile);
            if (qrcFile.exists()) {
                bool copyOk = qrcFile.copy(mjsonPathfile);
                if (!copyOk) {
                    qDebug() << "cannot copy data assets from qrc to data path";
                    return false;
                }
                bool permOk = QFile::setPermissions(mjsonPathfile, QFileDevice::ReadUser | QFileDevice::WriteUser);
                if (!permOk) {
                    qDebug() << "cannot set Permissions to read / write settings";
                    return false;
                }
            } else {
                qDebug() << "nothing in qrc" << mjsonqrcfile;
                return false;
            }
        }
        if (!readFile.open(QIODevice::ReadOnly)) {
            qWarning() << "Couldn't open file:" << mjsonPathfile;
            return false;
        }
        readFile.close();
        return true;
    }

    Q_INVOKABLE QString fluidiconpath() const
    {
        return QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0)
        + QDir::separator() + QStringLiteral("assets/fluidicons");
    }

    Q_INVOKABLE QString extractword(const QString &word, const QString &criteria)
    {
        QRegularExpression eric(QRegularExpression::escape(criteria));
        QRegularExpressionMatch match = eric.match(word);
        return match.hasMatch() ? match.captured(0) : QString();
    }

    Q_INVOKABLE QString jsonlink() { return QStringLiteral("https://www.awesomeerictech.com/"); }
    Q_INVOKABLE QString medialink() { return QStringLiteral("https://www.awesomeerictech.com/digitalmenu/media/"); }

    Q_INVOKABLE bool hasexpired(const QString &startdate)
    {
        QDateTime mycurrent = QDateTime::currentDateTime();
        QDateTime raw = QDateTime::fromString(startdate, Qt::ISODate);
        if (!raw.isValid()) return true; // treat invalid as expired
        QDateTime rawdatetime = raw.toTimeZone(mycurrent.timeZone());
        QDate beginDate = rawdatetime.date();
        QDate expireDate = mycurrent.date();

        // simple "expired if more than 0 years" logic per your original
        const int years = expireDate.year() - beginDate.year();
        if (years > 0) return true;

        // if months >= 1 treat as not expired (following original logic)
        int months = 0;
        QDate tmp = beginDate;
        while (tmp.addMonths(1) <= expireDate) {
            tmp = tmp.addMonths(1);
            months++;
            if (months >= 1) return false;
        }
        return false;
    }

    Q_INVOKABLE QString tolocaltime(const QString &isodate)
    {
        QDateTime mycurrent = QDateTime::currentDateTime();
        QDateTime raw = QDateTime::fromString(isodate, Qt::ISODate);
        if (!raw.isValid()) return QString();
        QDateTime rawdatetime = raw.toTimeZone(mycurrent.timeZone());
        return rawdatetime.toString();
    }

    Q_INVOKABLE QString dayselapsed(const QString &startdate)
    {
        QDateTime mycurrent = QDateTime::currentDateTime();
        QDateTime raw = QDateTime::fromString(startdate, Qt::ISODate);
        if (!raw.isValid()) return QString();
        QDateTime rawdatetime = raw.toTimeZone(mycurrent.timeZone());
        QDate beginDate = rawdatetime.date();
        QDate expireDate = mycurrent.date();

        int y = expireDate.year() - beginDate.year();
        int m = expireDate.month() - beginDate.month();
        int d = expireDate.day() - beginDate.day();
        if (d < 0) { m--; d += beginDate.daysInMonth(); }
        if (m < 0) { y--; m += 12; }

        return QString("%1 years, %2 months, %3 days has elapsed since you joined us, thankyou.")
            .arg(y).arg(m).arg(d);
    }

    Q_INVOKABLE QString authsub() { return QStringLiteral("adminqrmenulokomokosub"); }
    Q_INVOKABLE QString authurl() { return QStringLiteral("adminqrmenulokomokourl"); }
    Q_INVOKABLE QString authphone() { return QStringLiteral("adminqrmenulokomokophone"); }
    Q_INVOKABLE QString authemail() { return QStringLiteral("adminqrmenulokomokoemail"); }
    Q_INVOKABLE QString authpass() { return QStringLiteral("adminqrmenulokomokopass"); }
    Q_INVOKABLE QString authblacklist() { return QStringLiteral("adminqrmenulokomokoblacklistuser"); }
    Q_INVOKABLE QString authadmin() { return QStringLiteral("adminqrmenulokomokoadmincreation"); }
    Q_INVOKABLE QString authdeletebiz() { return QStringLiteral("adminqrmenulokomokodeleteuserbiz"); }

    Q_INVOKABLE QString jwtregtoken(const QVariantMap &ericmap)
    {
        QJsonWebToken m_jwtObj;
        QString user = ericmap.value("user").toString();
        QString hint = ericmap.value("hint").toString();
        QString password = ericmap.value("password").toString();
        QString email = ericmap.value("email").toString();
        QString phone = ericmap.value("phone").toString();
        m_jwtObj.appendClaim("user", user);
        m_jwtObj.appendClaim("hint", hint);
        m_jwtObj.appendClaim("password", password);
        m_jwtObj.appendClaim("email", email);
        m_jwtObj.appendClaim("phone", phone);
        m_jwtObj.setSecret("registerericdigitalmenu");
        m_jwtObj.setAlgorithmStr("HSSha3_512");
        return m_jwtObj.getToken();
    }

    Q_INVOKABLE QString jwtlogintoken(const QVariantMap &ericmap)
    {
        QJsonWebToken m_jwtObj;
        QString user = ericmap.value("user").toString();
        QString password = ericmap.value("password").toString();
        m_jwtObj.appendClaim("user", user);
        m_jwtObj.appendClaim("password", password);
        m_jwtObj.setSecret("loginericdigitalmenu");
        m_jwtObj.setAlgorithmStr("HSSha3_512");
        return m_jwtObj.getToken();
    }

    Q_INVOKABLE QString extracttokenreg(const QString &jwt, const QString &user)
    {
        // expects 'jwt' to be a JSON array where elements contain {"token":"..."}
        QJsonDocument doc = QJsonDocument::fromJson(jwt.toUtf8());
        QJsonArray myarr = doc.array();
        QString singletoken;
        for (const QJsonValue &value : myarr) {
            QJsonObject obj = value.toObject();
            singletoken = obj.value("token").toString();
        }
        if (singletoken.isEmpty()) return QString();
        QString strSecret = QStringLiteral("registerericdigitalmenu");
        QJsonWebToken token = QJsonWebToken::fromTokenAndSecret(singletoken, strSecret);
        if (!token.isValid()) return QString();
        QString strPayload = token.getPayloadQStr();
        QJsonObject myobj = QJsonDocument::fromJson(strPayload.toUtf8()).object();
        return myobj.value(user).toString();
    }

    Q_INVOKABLE QString extracttokenlogin(const QString &jwt, const QString &user)
    {
        QJsonDocument doc = QJsonDocument::fromJson(jwt.toUtf8());
        QJsonArray myarr = doc.array();
        QString singletoken;
        for (const QJsonValue &value : myarr) {
            QJsonObject obj = value.toObject();
            singletoken = obj.value("token").toString();
        }
        if (singletoken.isEmpty()) return QString();
        QString strSecret = QStringLiteral("loginericdigitalmenu");
        QJsonWebToken token = QJsonWebToken::fromTokenAndSecret(singletoken, strSecret);
        if (!token.isValid()) return QString();
        QString strPayload = token.getPayloadQStr();
        QJsonObject myobj = QJsonDocument::fromJson(strPayload.toUtf8()).object();
        return myobj.value(user).toString();
    }

private:
    ImageLoader *mImageLoader = nullptr;
};

// If you want to instantiate this class and register to QML:
// JsonTools *tools = new JsonTools(appEngine.rootContext());
// engine.rootContext()->setContextProperty(\"JsonTools\", tools);


#endif // JSONTOOLS_HPP


