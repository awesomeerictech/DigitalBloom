#ifndef GUIAPPLICATION_H
#define GUIAPPLICATION_H

#include <QGuiApplication>

namespace eric {

class GuApplication : public QGuiApplication
{
    Q_OBJECT
public:
#ifdef Q_QDOC
    explicit GuApplication(int &argc, char **argv);
#else
    explicit GuApplication(int &argc, char **argv, int = ApplicationFlags);
#endif

    bool notify(QObject *receiver, QEvent *event);
Q_SIGNALS:
    void back(QEvent *event);

};

}
#endif // GUIAPPLICATION_H
