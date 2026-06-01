
#ifndef WRAPUPLOAD_H_
#define WRAPUPLOAD_H_

#include <QObject>

class DataUploader;

class WrapUpload : public QObject
{
  Q_OBJECT

public:
  WrapUpload(DataUploader *uploader);

  void passUpload(const QJsonObject&);
  Q_INVOKABLE void uploadtoserver(const QJsonObject&);

Q_SIGNALS:
  void upload(const QJsonObject&);

public Q_SLOTS:
  void doUpload(const QJsonObject&);

private:
  DataUploader *uploader_;
};

#endif // WRAPUPLOAD_H_
