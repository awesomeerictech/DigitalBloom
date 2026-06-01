#ifndef CLOSEALL_HPP
#define CLOSEALL_HPP

#include <QObject>
#include <QDebug>
#include <QGuiApplication>
#include <QDebug>
#include <QTimer>
#include <QtMultimediaQuick/private/qquickvideooutput_p.h>
#include <QObject>



/*
extern "C" {
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavcodec/avcodec.h>
}
*/

class CloseAll : public QObject {

Q_OBJECT

public slots:

void closeapp() {


  //  avformat_network_deinit();
    exit(EXIT_SUCCESS);



}





};

#endif // CLOSEALL_HPP
