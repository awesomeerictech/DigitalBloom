#include "SharingApplication.h"
#include "SharingUtils.h"

#include <QGuiApplication>
#include <QQmlContext>

#include <QDir>
#include <QFile>
#include <QFileOpenEvent>

#include <QDebug>

/* ************************************************************************** */

SharingApplication::SharingApplication(int &argc, char **argv) : QGuiApplication(argc, argv)
{
    mShareUtils = new ShareUtils(this);

    connect(this, &QGuiApplication::applicationStateChanged, this, &SharingApplication::onApplicationStateChanged);
}

SharingApplication::~SharingApplication()
{
    //
}

/* ************************************************************************** */

bool SharingApplication::event(QEvent *e)
{
    // macOS "drag to dock" feature
    if (e->type() == QEvent::FileOpen)
    {
        QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(e);
        if (QFile::exists(openEvent->file()))
        {
            Q_EMIT fileDropped(openEvent->file());
        }
    }

    return QGuiApplication::event(e);
}

void SharingApplication::onApplicationStateChanged(Qt::ApplicationState appState)
{
    if (appState == Qt::ApplicationState::ApplicationActive)
    {
        if (!mPendingIntentsChecked)
        {
            mPendingIntentsChecked = true;
            mShareUtils->checkPendingIntents(mAppDataFilesPath);
        }
    }
}

/* ************************************************************************** */
