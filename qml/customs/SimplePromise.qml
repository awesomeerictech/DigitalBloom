import QtQuick 2.0

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
