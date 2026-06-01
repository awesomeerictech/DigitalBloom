#include "utils_clipboard.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>

/* ************************************************************************** */

void UtilsClipboard::clear()
{
    //
}

void UtilsClipboard::setText(const QString &txt)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString originalText = clipboard->text();

    clipboard->setText(txt);
}

QString UtilsClipboard::getText()
{
    const QClipboard *clipboard = QGuiApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    if (mimeData->hasText())
    {
        return mimeData->text();
        // Qt::PlainText;
    }
    else
    {
        // not handled
    }

    return QString();
}

/* ************************************************************************** */
