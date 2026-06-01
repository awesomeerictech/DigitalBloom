#pragma once

#include <QAbstractListModel>
#include <QGeoCoordinate>

class MarkerModel : public QAbstractListModel {
  Q_OBJECT

public:
  explicit MarkerModel(QObject *parent = nullptr);

  enum MarkerRoles { positionRole = Qt::UserRole + 1 };

  Q_INVOKABLE void addMarker(const QGeoCoordinate &coordinate);
  Q_INVOKABLE void clearMarkers();

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

private:
  QList<QGeoCoordinate> m_coordinates;
};
