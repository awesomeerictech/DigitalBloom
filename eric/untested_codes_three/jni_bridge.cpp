#include <QDebug>
#include <QtCore/private/qandroidextras_p.h>
#include <QObject>
#include <QQmlEngine>

// A QObject derived class to emit signals back to QML
class AndroidFileSaver : public QObject {
    Q_OBJECT
public:
    explicit AndroidFileSaver(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE void showSaveDialog(const QString& fileName, const QString& fileContent) {
        QJniObject javaFileName = QJniObject::fromString(fileName);
        QJniObject javaFileContent = QJniObject::fromString(fileContent);

        // Call the static Java method defined in MainActivity.java
        QJniObject::callStaticMethod<void>(
            "org/stlltd/digitalbloom/MainActivity", // REPLACE THIS
            "showSaveFileDialog",
            "(Ljava/lang/String;Ljava/lang/String;)V",
            javaFileName.object<jstring>(),
            javaFileContent.object<jstring>()
        );
    }

signals:
    void fileSaveSuccess(const QString& uri);
    void fileSaveError(const QString& message);
};

// Global pointer to the object instance
static AndroidFileSaver* s_fileSaverInstance = nullptr;

// --- JNI Callbacks from Java ---

// These functions must match the signature defined by 'native void' in Java
extern "C" {
    // Note: The function name must exactly match the package structure + class name + method name
    JNIEXPORT void JNICALL
    Java_com_your_packagename_MainActivity_onFileSaveSuccess(JNIEnv *env, jobject thiz, jstring uriString) {
        if (s_fileSaverInstance) {
            const QString uri = QJniObject(uriString).toString();
            emit s_fileSaverInstance->fileSaveSuccess(uri);
        }
    }

    JNIEXPORT void JNICALL
    Java_com_your_packagename_MainActivity_onFileSaveError(JNIEnv *env, jobject thiz, jstring errorMessage) {
        if (s_fileSaverInstance) {
            const QString message = QJniObject(errorMessage).toString();
            emit s_fileSaverInstance->fileSaveError(message);
        }
    }
}

// Helper function to register the C++ object with QML
void registerAndroidFileSaverType(QQmlEngine *engine) {
    s_fileSaverInstance = new AndroidFileSaver();
    // Expose the C++ object to QML under the name "AndroidFileSaver"
    engine->rootContext()->setContextProperty("AndroidFileSaver", s_fileSaverInstance);
}

#include "androidsaver.moc" // Required for Q_OBJECT signals/slots

