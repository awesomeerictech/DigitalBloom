import QtQuick
import QtQml
import Quickeric 1.0
import Org.Eric.Technologies 1.0


QtObject {

    property string resource
    property SimplePromise promise:  SimplePromise{}
    property bool autoInform: true

    function get(what, from, to, query){
        _rest.get(resource + what, from, to, query)
        return promise
    }

    function get_one(id, query){

        _rest.idGet(resource + "/id/" + id, query)
        return promise
    }

    function list(from, to , query){

        _rest.get(resource, from, to, query)
        return promise
    }

    function create(data, query){

        _rest.post(resource, data, query)
        return promise
    }

    function save(id, data, query){

        _rest.idPut(resource + "/id/" + id, data, query)
        return promise
    }

    function remove(id, query){

        _rest.idDelete(resource + "/id/" + id, query)
        return promise
    }


    property Rest _rest: Rest {
        restConsumer: app.restConsumer

        onReady: {

            if(promise.successCallback) promise.successCallback(rawData)
        }

        onPosted: {

            if(promise.successCallback) promise.successCallback(rawData)
        }

        onUpdated: {

            if(promise.successCallback) promise.successCallback(rawData)
        }

        onDeleted: {

            if(promise.successCallback) promise.successCallback(rawData)
        }

        onNetworkError: {

            if(promise.networkErrorCallback) promise.networkErrorCallback(err)
        }

        onServerError: {
            console.log("ResourceApi", resource, err)
            if(promise.serverErrorCallback) promise.serverErrorCallback(err)
        }
    }
}
