#include "utils_camera.h"

#include <QImage>
#include <QVideoFrame>
#include <QCameraFormat>
#include <QMediaDevices>

/* ************************************************************************** */

UtilsCamera *UtilsCamera::instance = nullptr;

UtilsCamera *UtilsCamera::getInstance()
{
    if (instance == nullptr)
    {
        instance = new UtilsCamera();
    }

    return instance;
}

/* ************************************************************************** */

QCameraFormat UtilsCamera::selectCameraFormat(int idx)
{
    QList <QCameraFormat> formats = QMediaDevices::defaultVideoInput().videoFormats();

    if (idx > 0 && idx < QMediaDevices::videoInputs().size())
    {
        formats = QMediaDevices::videoInputs().at(idx).videoFormats();
    }

    int optimalPixelFormat = QVideoFrameFormat::Format_Invalid;

    // square formats
    for (const auto &format : formats)
    {
        qWarning() << "QCameraFormat(" << idx << ") res: " << format.resolution() << " pix: " << format.pixelFormat();

        if (format.resolution() == QSize(1440, 1440) &&
            (!optimalPixelFormat || (optimalPixelFormat && format.pixelFormat() == optimalPixelFormat)))
        {
            qWarning() << "SELECTED FORMAT res: " << format.resolution() << " pix: " << format.pixelFormat();
            return format;
        }
        if (format.resolution() == QSize(1200, 1200) &&
            (!optimalPixelFormat || (optimalPixelFormat && format.pixelFormat() == optimalPixelFormat)))
        {
            qWarning() << "SELECTED FORMAT res: " << format.resolution() << " pix: " << format.pixelFormat();
            return format;
        }
        if (format.resolution() == QSize(1080, 1080) &&
            (!optimalPixelFormat || (optimalPixelFormat && format.pixelFormat() == optimalPixelFormat)))
        {
            qWarning() << "SELECTED FORMAT res: " << format.resolution() << " pix: " << format.pixelFormat();
            return format;
        }
    }

    // non square formats
    for (const auto &format : formats)
    {
        qWarning() << "QCameraFormat(" << idx << ") res: " << format.resolution() << " pix: " << format.pixelFormat();

        if (format.resolution() == QSize(1440, 1080) &&
            (!optimalPixelFormat || (optimalPixelFormat && format.pixelFormat() == optimalPixelFormat)))
        {
            qWarning() << "SELECTED FORMAT res: " << format.resolution() << " pix: " << format.pixelFormat();
            return format;
        }

        if (format.resolution() == QSize(2560, 1440) &&
            (!optimalPixelFormat || (optimalPixelFormat && format.pixelFormat() == optimalPixelFormat)))
        {
            qWarning() << "SELECTED FORMAT res: " << format.resolution() << " pix: " << format.pixelFormat();
            return format;
        }
    }

    return QCameraFormat();
}

/* ************************************************************************** */
