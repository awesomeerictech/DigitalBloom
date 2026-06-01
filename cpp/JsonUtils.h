#pragma once
// Qt
#include <QVariantMap>

namespace eric
{

class JsonUtils : public QObject
{
    Q_OBJECT

public:
    JsonUtils();

    static QString toJsonString(const QVariantMap &data);
    static QString toJsonString(const QVariant &data);
    static QString toJsonString(const QList<QVariant> &data);

    static QVariantMap toVariantMap(const QString &data);
    static QList<QVariant> toVariantList(const QString &data);
};

}
