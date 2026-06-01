
#include "qjsonwebtoken.h"

#include <QDebug>

QJsonWebToken::QJsonWebToken()
{
	// create the header with default algorithm
	setAlgorithmStr("HS256");
	m_jdocPayload = QJsonDocument::fromJson("{}");
    // default for random generation
    m_intRandLength   = 10;
    m_strRandAlphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
}

QJsonWebToken::QJsonWebToken(const QJsonWebToken &other)
{
	this->m_jdocHeader    = other.m_jdocHeader;
	this->m_jdocPayload   = other.m_jdocPayload;
	this->m_byteSignature = other.m_byteSignature;
	this->m_strSecret     = other.m_strSecret;
	this->m_strAlgorithm  = other.m_strAlgorithm;
}

QJsonDocument QJsonWebToken::getHeaderJDoc() const
{
	return m_jdocHeader;
}

QString QJsonWebToken::getHeaderQStr(const QJsonDocument::JsonFormat &format/* = QJsonDocument::JsonFormat::Indented*/) const
{
	return m_jdocHeader.toJson(format);
}

bool QJsonWebToken::setHeaderJDoc(const QJsonDocument &jdocHeader)
{
	if (jdocHeader.isEmpty() || jdocHeader.isNull() || !jdocHeader.isObject())
	{
		return false;
	}

	// check if supported algorithm
	QString strAlgorithm = jdocHeader.object().value("alg").toString("");
	if (!isAlgorithmSupported(strAlgorithm))
	{
		return false;
	}

	m_jdocHeader = jdocHeader;

	// set also new algorithm
	m_strAlgorithm = strAlgorithm;

	return true;
}

bool QJsonWebToken::setHeaderQStr(const QString &strHeader)
{
	QJsonParseError error;
	QJsonDocument tmpHeader = QJsonDocument::fromJson(strHeader.toUtf8(), &error);

	// validate and set header
	if (error.error != QJsonParseError::NoError || !setHeaderJDoc(tmpHeader))
	{
		return false;
	}

	return true;
}

QJsonDocument QJsonWebToken::getPayloadJDoc() const
{
	return m_jdocPayload;
}

QString QJsonWebToken::getPayloadQStr(const QJsonDocument::JsonFormat &format/* = QJsonDocument::JsonFormat::Indented*/) const
{
	return m_jdocPayload.toJson(format);
}

bool QJsonWebToken::setPayloadJDoc(const QJsonDocument &jdocPayload)
{
	if (jdocPayload.isEmpty() || jdocPayload.isNull() || !jdocPayload.isObject())
	{
		return false;
	}

	m_jdocPayload = jdocPayload;

	return true;
}

bool QJsonWebToken::setPayloadQStr(const QString &strPayload)
{
	QJsonParseError error;
	QJsonDocument tmpPayload = QJsonDocument::fromJson(strPayload.toUtf8(), &error);

	// validate and set payload
	if (error.error != QJsonParseError::NoError || !setPayloadJDoc(tmpPayload))
	{
		return false;
	}

	return true;
}

// // helper: determine block size (in bytes) for HMAC for a given algorithm
// static int hmacBlockSize(QCryptographicHash::Algorithm alg)
// {
//     switch (alg) {
//     case QCryptographicHash::Sha256:    return 64;   // 512-bit block
//     case QCryptographicHash::Sha384:    // SHA-384 is part of SHA-512 family
//     case QCryptographicHash::Sha512:    return 128;  // 1024-bit block
//     case QCryptographicHash::Sha3_256:  return 136;  // Keccak rate = 200 - 2*digest (200 - 64)
//     case QCryptographicHash::Sha3_384:  return 104;  // 200 - 2*48 = 104
//     case QCryptographicHash::Sha3_512:  return 72;   // 200 - 2*64 = 72
//     case QCryptographicHash::Keccak_256: return 136;
//     case QCryptographicHash::Keccak_384: return 104;
//     case QCryptographicHash::Keccak_512: return 72;
//     default: return 64; // conservative default
//     }
// }

// // helper: compute HMAC(key, data, alg) using QCryptographicHash
// static QByteArray computeHmacWithQCryptographicHash(const QByteArray &key, const QByteArray &data, QCryptographicHash::Algorithm alg)
// {
//     // 1) Normalize key to block size
//     int blockSize = hmacBlockSize(alg);
//     QByteArray k = key;
//     if (k.size() > blockSize) {
//         k = QCryptographicHash::hash(k, alg); // reduce to digest size
//     }
//     if (k.size() < blockSize) {
//         k.append(QByteArray(blockSize - k.size(), '\0'));
//     }

//     // 2) compute ipad and opad
//     QByteArray ipad(blockSize, 0x36);
//     QByteArray opad(blockSize, 0x5c);
//     for (int i = 0; i < blockSize; ++i) {
//         ipad[i] = ipad[i] ^ static_cast<char>(k.at(i));
//         opad[i] = opad[i] ^ static_cast<char>(k.at(i));
//     }

//     // 3) inner hash = hash(ipad || data)
//     QByteArray inner;
//     inner.reserve(ipad.size() + data.size());
//     inner.append(ipad);
//     inner.append(data);
//     QByteArray innerHash = QCryptographicHash::hash(inner, alg);

//     // 4) outer hash = hash(opad || innerHash)
//     QByteArray outer;
//     outer.reserve(opad.size() + innerHash.size());
//     outer.append(opad);
//     outer.append(innerHash);
//     QByteArray result = QCryptographicHash::hash(outer, alg);
//     return result;
// }

// QByteArray QJsonWebToken::getSignature()
// {
//     // get header/payload base64 as owned QByteArray
//     QByteArray byteHeaderBase64  = getHeaderQStr(QJsonDocument::JsonFormat::Compact).toUtf8().toBase64();
//     QByteArray bytePayloadBase64 = getPayloadQStr(QJsonDocument::JsonFormat::Compact).toUtf8().toBase64();

//     // Build owned contiguous buffers
//     QByteArray allData;
//     allData.reserve(byteHeaderBase64.size() + 1 + bytePayloadBase64.size());
//     allData.append(byteHeaderBase64);
//     allData.append('.');
//     allData.append(bytePayloadBase64);

//     QByteArray secret = m_strSecret.toUtf8();

//     // keep original visible state
//     m_byteAllData = allData;

//     // map string algorithm to QCryptographicHash::Algorithm where possible
//     QCryptographicHash::Algorithm alg = QCryptographicHash::Md5; // sentinel
//     if (m_strAlgorithm.compare("HS256", Qt::CaseSensitive) == 0) {
//         alg = QCryptographicHash::Sha256;
//     } else if (m_strAlgorithm.compare("HS384", Qt::CaseSensitive) == 0) {
//         alg = QCryptographicHash::Sha384;
//     } else if (m_strAlgorithm.compare("HS512", Qt::CaseSensitive) == 0) {
//         alg = QCryptographicHash::Sha512;
//     } else if (m_strAlgorithm.compare("HSSha3_256", Qt::CaseSensitive) == 0) {
//         alg = QCryptographicHash::Sha3_256;
//     } else if (m_strAlgorithm.compare("HSSha3_384", Qt::CaseSensitive) == 0) {
//         alg = QCryptographicHash::Sha3_384;
//     } else if (m_strAlgorithm.compare("HSSha3_512", Qt::CaseSensitive) == 0) {
//         alg = QCryptographicHash::Sha3_512;
//     } else if (m_strAlgorithm.compare("HSKeccak_256", Qt::CaseSensitive) == 0) {
//         alg = QCryptographicHash::Keccak_256;
//     } else if (m_strAlgorithm.compare("HSKeccak_384", Qt::CaseSensitive) == 0) {
//         alg = QCryptographicHash::Keccak_384;
//     } else if (m_strAlgorithm.compare("HSKeccak_512", Qt::CaseSensitive) == 0) {
//         alg = QCryptographicHash::Keccak_512;
//     } else {
//         // unsupported algorithm
//         m_byteSignature.clear();
//         return m_byteSignature;
//     }

//     // Compute HMAC manually using QCryptographicHash (avoids QMessageAuthenticationCode internals)
//     // But first check QCryptographicHash supports this algorithm on this Qt build
//     // QCryptographicHash::isSupported is not available, so we'll attempt and fallback if needed.
//     QByteArray hmac;
//     bool hmacSucceeded = true;
//     try {
//         hmac = computeHmacWithQCryptographicHash(secret, allData, alg);
//     } catch (...) {
//         hmacSucceeded = false;
//     }

//     if (!hmacSucceeded || hmac.isEmpty()) {
//         // As a fallback only (if compute failed), use QMessageAuthenticationCode with owned buffers
//         m_byteSignature = QMessageAuthenticationCode::hash(allData, secret,
//                                                            (alg == QCryptographicHash::Sha256 ? QCryptographicHash::Sha256 :
//                                                                 alg == QCryptographicHash::Sha384 ? QCryptographicHash::Sha384 :
//                                                                 alg == QCryptographicHash::Sha512 ? QCryptographicHash::Sha512 :
//                                                                 alg == QCryptographicHash::Sha3_256 ? QCryptographicHash::Sha3_256 :
//                                                                 alg == QCryptographicHash::Sha3_384 ? QCryptographicHash::Sha3_384 :
//                                                                 alg == QCryptographicHash::Sha3_512 ? QCryptographicHash::Sha3_512 :
//                                                                 alg == QCryptographicHash::Keccak_256 ? QCryptographicHash::Keccak_256 :
//                                                                 alg == QCryptographicHash::Keccak_384 ? QCryptographicHash::Keccak_384 :
//                                                                 alg == QCryptographicHash::Keccak_512 ? QCryptographicHash::Keccak_512 :
//                                                                 QCryptographicHash::Sha256));
//     } else {
//         m_byteSignature = hmac;
//     }

//     return m_byteSignature;
// }

static int hmacBlockSize(QCryptographicHash::Algorithm alg)
{
    switch (alg) {
    case QCryptographicHash::Sha256:    return 64;   // 512-bit block
    case QCryptographicHash::Sha384:    // SHA-384 is part of SHA-512 family
    case QCryptographicHash::Sha512:    return 128;  // 1024-bit block
    case QCryptographicHash::Sha3_256:  return 136;  // Keccak rate = 200 - 2*digest (200 - 64)
    case QCryptographicHash::Sha3_384:  return 104;  // 200 - 2*48 = 104
    case QCryptographicHash::Sha3_512:  return 72;   // 200 - 2*64 = 72
    case QCryptographicHash::Keccak_256: return 136;
    case QCryptographicHash::Keccak_384: return 104;
    case QCryptographicHash::Keccak_512: return 72;
    default: return 64; // conservative default
    }
}


static QByteArray computeHmacWithQCryptographicHash(const QByteArray &key, const QByteArray &data, QCryptographicHash::Algorithm alg)
{
    // 1) Normalize key to block size
    int blockSize = hmacBlockSize(alg);
    QByteArray k = key;
    if (k.size() > blockSize) {
        k = QCryptographicHash::hash(k, alg); // reduce to digest size
    }
    if (k.size() < blockSize) {
        k.append(QByteArray(blockSize - k.size(), '\0'));
    }

    // 2) compute ipad and opad
    QByteArray ipad(blockSize, 0x36);
    QByteArray opad(blockSize, 0x5c);
    for (int i = 0; i < blockSize; ++i) {
        ipad[i] = ipad[i] ^ static_cast<char>(k.at(i));
        opad[i] = opad[i] ^ static_cast<char>(k.at(i));
    }

    // 3) inner hash = hash(ipad || data)
    QByteArray inner;
    inner.reserve(ipad.size() + data.size());
    inner.append(ipad);
    inner.append(data);
    QByteArray innerHash = QCryptographicHash::hash(inner, alg);

    // 4) outer hash = hash(opad || innerHash)
    QByteArray outer;
    outer.reserve(opad.size() + innerHash.size());
    outer.append(opad);
    outer.append(innerHash);
    QByteArray result = QCryptographicHash::hash(outer, alg);
    return result;
}

QByteArray QJsonWebToken::getSignature()
{
    // get header/payload base64 as owned QByteArray
    QByteArray byteHeaderBase64  = getHeaderQStr(QJsonDocument::JsonFormat::Compact).toUtf8().toBase64();
    QByteArray bytePayloadBase64 = getPayloadQStr(QJsonDocument::JsonFormat::Compact).toUtf8().toBase64();

    // Build owned contiguous buffers
    QByteArray allData;
    allData.reserve(byteHeaderBase64.size() + 1 + bytePayloadBase64.size());
    allData.append(byteHeaderBase64);
    allData.append('.');
    allData.append(bytePayloadBase64);

    QByteArray secret = m_strSecret.toUtf8();

    // keep original visible state
    m_byteAllData = allData;

    // map string algorithm to QCryptographicHash::Algorithm where possible
    QCryptographicHash::Algorithm alg = QCryptographicHash::Md5; // sentinel
    if (m_strAlgorithm.compare("HS256", Qt::CaseSensitive) == 0) {
        alg = QCryptographicHash::Sha256;
    } else if (m_strAlgorithm.compare("HS384", Qt::CaseSensitive) == 0) {
        alg = QCryptographicHash::Sha384;
    } else if (m_strAlgorithm.compare("HS512", Qt::CaseSensitive) == 0) {
        alg = QCryptographicHash::Sha512;
    } else if (m_strAlgorithm.compare("HSSha3_256", Qt::CaseSensitive) == 0) {
        alg = QCryptographicHash::Sha3_256;
    } else if (m_strAlgorithm.compare("HSSha3_384", Qt::CaseSensitive) == 0) {
        alg = QCryptographicHash::Sha3_384;
    } else if (m_strAlgorithm.compare("HSSha3_512", Qt::CaseSensitive) == 0) {
        alg = QCryptographicHash::Sha3_512;
    } else if (m_strAlgorithm.compare("HSKeccak_256", Qt::CaseSensitive) == 0) {
        alg = QCryptographicHash::Keccak_256;
    } else if (m_strAlgorithm.compare("HSKeccak_384", Qt::CaseSensitive) == 0) {
        alg = QCryptographicHash::Keccak_384;
    } else if (m_strAlgorithm.compare("HSKeccak_512", Qt::CaseSensitive) == 0) {
        alg = QCryptographicHash::Keccak_512;
    } else {
        // unsupported algorithm
        m_byteSignature.clear();
        return m_byteSignature;
    }

    // Compute HMAC manually using QCryptographicHash
    QByteArray hmac = computeHmacWithQCryptographicHash(secret, allData, alg);

    // Set the signature from the computed HMAC result
    m_byteSignature = hmac;

    return m_byteSignature;
}

QByteArray QJsonWebToken::getSignatureBase64()
{
	// note we return through getSignature() to force recalculation
	return getSignature().toBase64();
}

QString QJsonWebToken::getSecret() const
{
	return m_strSecret;
}

bool QJsonWebToken::setSecret(const QString &strSecret)
{
	if (strSecret.isEmpty() || strSecret.isNull())
	{
		return false;
	}

	m_strSecret = strSecret;

    return true;
}

void QJsonWebToken::setRandomSecret()
{
    m_strSecret.resize(m_intRandLength);
    for (int i = 0; i < m_intRandLength; ++i)
    {
        m_strSecret[i] = m_strRandAlphanum.at(rand() % (m_strRandAlphanum.length() - 1));
    }
}

QString QJsonWebToken::getAlgorithmStr() const
{
	return m_strAlgorithm;
}

bool QJsonWebToken::setAlgorithmStr(const QString &strAlgorithm)
{
	// check if supported algorithm
	if (!isAlgorithmSupported(strAlgorithm))
	{
		return false;
	}
	// set algorithm
	m_strAlgorithm = strAlgorithm;
	// modify header
    m_jdocHeader = QJsonDocument::fromJson(QObject::tr("{\"typ\": \"JWT\", \"alg\" : \"").toUtf8()
		                                 + m_strAlgorithm.toUtf8()
                                         + QObject::tr("\"}").toUtf8());

	return true;
}

QString QJsonWebToken::getToken()
{
	// important to execute first to update m_byteAllData which contains header + "." + payload in base64
	QByteArray byteSignatureBase64 = this->getSignatureBase64();
	// compose token and return it
	return m_byteAllData + "." + byteSignatureBase64;
}

bool QJsonWebToken::setToken(const QString &strToken)
{
	// assume base64 encoded at first, if not try decoding
	bool isBase64Encoded = true;
	QStringList listJwtParts = strToken.split(".");
	// check correct size
	if (listJwtParts.count() != 3)
	{
		return false;
	}
	// check all parts are valid using another instance,
	// so we dont overwrite this instance in case of error
	QJsonWebToken tempTokenObj;
	if ( !tempTokenObj.setHeaderQStr(QByteArray::fromBase64(listJwtParts.at(0).toUtf8())) ||
		 !tempTokenObj.setPayloadQStr(QByteArray::fromBase64(listJwtParts.at(1).toUtf8())) )
	{
		// try unencoded
		if (!tempTokenObj.setHeaderQStr(listJwtParts.at(0)) ||
			!tempTokenObj.setPayloadQStr(listJwtParts.at(1)))
		{
			return false;
		}
		else
		{
			isBase64Encoded = false;
		}
	}
	// set parts on this instance
	setHeaderQStr(tempTokenObj.getHeaderQStr());
	setPayloadQStr(tempTokenObj.getPayloadQStr());
	if (isBase64Encoded)
	{ // unencode
		m_byteSignature = QByteArray::fromBase64(listJwtParts.at(2).toUtf8());
	} 
	else
	{
		m_byteSignature = listJwtParts.at(2).toUtf8();
	}
	// allData not valid anymore
	m_byteAllData.clear();
	// success
    return true;
}

QString QJsonWebToken::getRandAlphanum() const
{
    return m_strRandAlphanum;
}

void QJsonWebToken::setRandAlphanum(const QString &strRandAlphanum)
{
    if(strRandAlphanum.isNull())
    {
        return;
    }
    m_strRandAlphanum = strRandAlphanum;
}

int QJsonWebToken::getRandLength() const
{
    return m_intRandLength;
}

void QJsonWebToken::setRandLength(const int &intRandLength)
{
    if(intRandLength < 0 || intRandLength > 1e6)
    {
        return;
    }
    m_intRandLength = intRandLength;
}

bool QJsonWebToken::isValid() const
{
	// calculate token on other instance,
	// so we dont overwrite this instance's signature
	QJsonWebToken tempTokenObj = *this;
	if (m_byteSignature != tempTokenObj.getSignature())
	{
		return false;
	}
	return true;
}

QJsonWebToken QJsonWebToken::fromTokenAndSecret(const QString &strToken, const QString &srtSecret)
{
	QJsonWebToken tempTokenObj;
	// set Token
	tempTokenObj.setToken(strToken);
	// set Secret
	tempTokenObj.setSecret(srtSecret);
	// return
	return tempTokenObj;
}

void QJsonWebToken::appendClaim(const QString &strClaimType, const QString &strValue)
{
	// have to make a copy of the json object, modify the copy and then put it back, sigh
	QJsonObject jObj = m_jdocPayload.object();
	jObj.insert(strClaimType, strValue);
	m_jdocPayload = QJsonDocument(jObj);
}

void QJsonWebToken::removeClaim(const QString &strClaimType)
{
	// have to make a copy of the json object, modify the copy and then put it back, sigh
	QJsonObject jObj = m_jdocPayload.object();
	jObj.remove(strClaimType);
	m_jdocPayload = QJsonDocument(jObj);
}

void QJsonWebToken::removeAll() {


    QJsonObject obj = m_jdocPayload.object();

  /*

   for( QJsonObject::iterator iter = obj.begin(); iter != obj.end(); ++iter) {
      qDebug() << iter.key() << iter.value();


    }

 */



    obj = QJsonObject();
    m_jdocPayload = QJsonDocument(obj);






}


bool QJsonWebToken::isAlgorithmSupported(const QString &strAlgorithm)
{
	// TODO : support other algorithms
	if (strAlgorithm.compare("HS256", Qt::CaseSensitive) != 0 && // HMAC using SHA-256 hash algorithm
		strAlgorithm.compare("HS384", Qt::CaseSensitive) != 0 && // HMAC using SHA-384 hash algorithm
        strAlgorithm.compare("HS512", Qt::CaseSensitive) != 0 && // HMAC using SHA-512 hash algorithm
        strAlgorithm.compare("HSSha3_256", Qt::CaseSensitive) != 0 && // HMAC using Sha3_256 hash algorithm
        strAlgorithm.compare("HSSha3_384", Qt::CaseSensitive) != 0 && // HMAC using Sha3_384 hash algorithm
        strAlgorithm.compare("HSSha3_512", Qt::CaseSensitive) != 0 && // HMAC using Sha3_512 hash algorithm
        strAlgorithm.compare("HSKeccak_256", Qt::CaseSensitive) != 0 && // HMAC using Keccak_256 hash algorithm
        strAlgorithm.compare("HSKeccak_384", Qt::CaseSensitive) != 0 && // HMAC using Keccak_384 hash algorithm
        strAlgorithm.compare("HSKeccak_512", Qt::CaseSensitive) != 0)  // HMAC using Keccak_512 hash algorithm
	{
		return false;
	}
	return true;
}

QStringList QJsonWebToken::supportedAlgorithms()
{
	// TODO : support other algorithms
    return QStringList() << "HS256" << "HS384" << "HS512" << "HSSha3_256" << "HSSha3_384" << "HSSha3_512" << "HSKeccak_256" << "HSKeccak_384" << "HSKeccak_512";
}

/*

{
    "success": true,
    "properties": [
        {
            "ID": 1001,
            "PropertyName": "McDonalds",
            "key": "00112233445566778899aabbccddeeff"
        },
        {
            "ID": 1002,
            "PropertyName": "Burger King",
            "key": "10112233445566778899aabbccddeeff"
        },
        {
            "ID": 1003,
            "PropertyName": "Taco Bell",
            "key": "20112233445566778899aabbccddeeff"
        }
    ]
}

QStringList propertyNames;
QStringList propertyKeys;
QString strReply = (QString)reply->readAll();
QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
QJsonObject jsonObject = jsonResponse.object();
QJsonArray jsonArray = jsonObject["properties"].toArray();

foreach (const QJsonValue & value, jsonArray) {
    QJsonObject obj = value.toObject();
    propertyNames.append(obj["PropertyName"].toString());
    propertyKeys.append(obj["key"].toString());
}



*/
