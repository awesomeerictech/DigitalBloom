
#include "DataUploader.h"
#include "WrapUpload.h"

WrapUpload::WrapUpload(DataUploader *uploader)
  : uploader_(uploader)
{ }

void WrapUpload::passUpload(const QJsonObject& payload)
{
  Q_EMIT upload(payload);
}

void WrapUpload::doUpload(const QJsonObject& payload)
{
  uploader_->doUpload(payload);
}

void WrapUpload::uploadtoserver(const QJsonObject& payload) {

    uploader_->uploadExtractedItems(payload);

}
