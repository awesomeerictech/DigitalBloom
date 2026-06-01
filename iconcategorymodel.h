

#pragma once

#include <QAbstractListModel>

class IconCategoryModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Role {
        Category = Qt::UserRole + 1
    };

    explicit IconCategoryModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QStringList m_items;

private Q_SLOTS:
    void readFile();
};
