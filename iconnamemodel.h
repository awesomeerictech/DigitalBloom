

#pragma once

#include <QAbstractListModel>

class IconNameModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged)
public:
    enum Role {
        Category = Qt::UserRole + 1,
        Name
    };

    explicit IconNameModel(QObject *parent = nullptr);

    QString category() const;
    void setCategory(const QString &category);

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

Q_SIGNALS:
    void categoryChanged();

private:
    QMap<QString, QStringList> m_items;
    QString m_category;

private Q_SLOTS:
    void readFile();
};
