

#include <QFile>
#include <QTextStream>

#include "iconcategorymodel.h"

IconCategoryModel::IconCategoryModel(QObject *parent)
    : QAbstractListModel(parent)
{
    readFile();
}

QHash<int, QByteArray> IconCategoryModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(Category, QByteArrayLiteral("category"));
    return roles;
}

int IconCategoryModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_items.size();
}

QVariant IconCategoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
    case Category:
        return m_items.at(index.row());
    default:
        break;
    }

    return QVariant();
}

void IconCategoryModel::readFile()
{
    beginResetModel();
    m_items.clear();
    endResetModel();

    QFile file(QStringLiteral(":/qml/icons.txt"));
    if (file.open(QFile::ReadOnly)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();

            if (!line.startsWith(QLatin1Char('\t'))) {
                beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
                m_items.append(line);
                endInsertRows();
            }
        }
        file.close();
    } else {
        qCritical("Unable to open icons list for the categories: %s", file.errorString().toLocal8Bit().constData());
    }
}
