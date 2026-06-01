
#pragma once

#include <QObject>
#include <Qt>

// Use these for apps targeting Android 13+ to access media files
#include <QPermissions>
#include <QMediaImagesPermission>
#include <QMediaVideoPermission>

class UtilsAndroid : public QObject
{
    Q_OBJECT
public:
    explicit UtilsAndroid(QObject *parent = nullptr);

    Q_INVOKABLE void requestMediaPermissions(const std::function<void(bool)>& callback);

    // Check specific media permissions
    Q_INVOKABLE bool hasMediaImagesPermission();
    Q_INVOKABLE bool hasMediaVideoPermission();

    // Use these for older devices if necessary (will not work on modern Android)
    Q_INVOKABLE void requestLegacyStoragePermissions(const std::function<void(bool)>& callback);
    Q_INVOKABLE bool hasLegacyStorageReadPermission();

private:
    void onPermissionsResult(const QPermission &permission, const std::function<void(bool)>& callback);
};

