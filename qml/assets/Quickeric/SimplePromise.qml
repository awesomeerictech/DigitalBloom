import QtQml 2.2

QtObject {

    function then(successCallback, serverErrorCallback, networkErrorCallback){

        this.successCallback = successCallback
        this.networkErrorCallback = networkErrorCallback
        this.serverErrorCallback = serverErrorCallback
    }

    property var successCallback
    property var networkErrorCallback
    property var serverErrorCallback
}
