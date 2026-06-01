#ifndef UTILS_CLIPBOARD_H
#define UTILS_CLIPBOARD_H
/* ************************************************************************** */
#include <QObject>
#include <QString>

/*!
 * \brief Clipboard wrapper for QML
 *
 * https://doc.qt.io/qt-6/qclipboard.html
 */
class UtilsClipboard : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE static void clear();

    Q_INVOKABLE static void setText(const QString &txt);
    Q_INVOKABLE static  QString getText();
};

/* ************************************************************************** */
#endif // UTILS_CLIPBOARD_H
