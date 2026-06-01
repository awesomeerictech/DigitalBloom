#include "MarkerModel.hpp"

MarkerModel::MarkerModel(QObject *parent)
  : QAbstractListModel(parent)
{
}

void MarkerModel::addMarker(const QGeoCoordinate &coordinate) {
  beginInsertRows(QModelIndex(), rowCount(), rowCount());
  m_coordinates.append(coordinate);
  endInsertRows();
}

void MarkerModel::clearMarkers(){
  beginResetModel();
  m_coordinates.clear();
  endResetModel();
}

int MarkerModel::rowCount(const QModelIndex &parent) const {
  Q_UNUSED(parent)
  return m_coordinates.count();
}

QVariant MarkerModel::data(const QModelIndex &index, int role) const {
  if (index.row() < 0 || index.row() >= m_coordinates.count())
    return QVariant();
  if (role == MarkerModel::positionRole)
    return QVariant::fromValue(m_coordinates[index.row()]);
  return QVariant();
}

QHash<int, QByteArray> MarkerModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[positionRole] = "position";
  return roles;
}
