#ifndef CAMERA_UTILS_H
#define CAMERA_UTILS_H
/* ************************************************************************** */

#include <QImage>
#include <QVideoFrame>
#include <QCameraFormat>
#include <QMediaDevices>

#include <QQmlEngine>

/* ************************************************************************** */

/*!
 * \brief The UtilsCamera class
 */
class UtilsCamera: public QObject
{
    Q_OBJECT

    // Singleton
    static UtilsCamera *instance;
    UtilsCamera() = default;
    ~UtilsCamera() = default;

public:
    static UtilsCamera *getInstance();

    static Q_INVOKABLE QCameraFormat selectCameraFormat(int idx = 0);
};

/* ************************************************************************** */
#endif // CAMERA_UTILS_H
