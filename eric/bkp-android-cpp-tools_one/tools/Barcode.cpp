#include "Barcode.h"

/* ************************************************************************** */

Barcode::Barcode(const QString &data, const QString &format, const QString &enc, const QString &ecc,
                 const QDateTime &lastseen, const double lat, const double lon,
                 const bool starred,
                 QObject *parent) : QObject(parent)
{
    m_data = data;
    m_format = format;
    m_encoding = enc;
    m_ecc = ecc;

    m_date = lastseen;
    m_geo_lat = lat;
    m_geo_long = lon;

    m_starred = starred;

    m_isMatrix = (format == "QR_CODE" || format == "QRCode" || format == "MicroQRCode" ||
                  format == "DATA_MATRIX" || format == "DataMatrix" || format == "Aztec" ||
                  format == "PDF417" || format == "MaxiCode");

    if (m_data.startsWith("http://") || m_data.startsWith("https://")) m_content = "URL";
    else if (m_data.startsWith("WIFI:")) m_content = "WiFi";
    else if (m_data.startsWith("mailto:")) m_content = "Email";
    else if (m_data.startsWith("geo:")) m_content = "Geolocation";
    else if (m_data.startsWith("tel:")) m_content = "Phone";
    else if (m_data.startsWith("smsto:")) m_content = "SMS";
    else if (m_data.startsWith("BEGIN:VCARD") || m_data.startsWith("MECARD:")) m_content = "Contact";
    else if (m_data.startsWith("BEGIN:VEVENT")) m_content = "Calendar";
}

Barcode::Barcode(const QString &data, const QString &format, const QString &enc, const QString &ecc,
                 const QDateTime &lastseen, const QColor &color,
                 /*const QPointF &p1, const QPointF &p2, const QPointF &p3, const QPointF &p4,*/
                 QObject *parent) : QObject(parent)
{
    m_data = data;
    m_format = format;
    m_encoding = enc;
    m_ecc = ecc;

    m_isMatrix = (format == "QR_CODE" || format == "QRCode" || format == "MicroQRCode" ||
                  format == "DATA_MATRIX" || format == "DataMatrix" || format == "Aztec" ||
                  format == "PDF417" || format == "MaxiCode");

    if (m_data.startsWith("http://") || m_data.startsWith("https://")) m_content = "URL";
    else if (m_data.startsWith("WIFI:")) m_content = "WiFi";
    else if (m_data.startsWith("mailto:")) m_content = "Email";
    else if (m_data.startsWith("geo:")) m_content = "Geolocation";
    else if (m_data.startsWith("tel:")) m_content = "Phone";
    else if (m_data.startsWith("smsto:")) m_content = "SMS";
    else if (m_data.startsWith("BEGIN:VCARD") || m_data.startsWith("MECARD:")) m_content = "Contact";
    else if (m_data.startsWith("BEGIN:VEVENT")) m_content = "Calendar";

    //m_lastCoordinates << p1 << p2 << p3 << p4;
    m_isOnScreen = true;
    m_lastSeen = lastseen;
    m_color = color;

    if (m_lastSeen.isValid())
    {
        connect(&m_lastTimer, &QTimer::timeout, [this]() { m_isOnScreen = false; Q_EMIT lastseenChanged(); });
        m_lastTimer.start(1000);
    }
}

Barcode::~Barcode()
{
    //
}

/* ************************************************************************** */

void Barcode::setStarred(const bool value)
{
    if (m_starred != value)
    {
        m_starred = value;
        Q_EMIT barcodeChanged();
    }
}

void Barcode::setLastSeen(const QDateTime &value)
{
    if (m_lastSeen != value)
    {
        m_isOnScreen = true;
        m_lastTimer.start(1000);
        m_lastSeen = value;
        Q_EMIT lastseenChanged();
    }
}

void Barcode::setLastCoordinates(const QPointF &p1, const QPointF &p2, const QPointF &p3, const QPointF &p4)
{
    m_lastCoordinates.clear();
    m_lastCoordinates << p1 << p2 << p3 << p4;
    Q_EMIT lastseenChanged();
}

/* ************************************************************************** */
