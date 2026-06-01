#ifndef ERICIMAGE_HPP
#define ERICIMAGE_HPP


#include <QObject>
#include <QElapsedTimer>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QSharedPointer>
#include <QImage>

class EricImage : public QObject
{
    Q_OBJECT

public:
  Q_INVOKABLE  void save( QQuickItem *item,QString pathnameext)
    {
        QSharedPointer<const QQuickItemGrabResult> grabResult = item->grabToImage();

        connect(grabResult.data(), &QQuickItemGrabResult::ready, [=]() {
            grabResult->saveToFile(pathnameext);

        });
    }

private:

public Q_SLOTS:

Q_SIGNALS:

};



#endif // ERICIMAGE_HPP
