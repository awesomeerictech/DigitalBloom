import QtQuick 2.0
import Quickeric 1.0

import "../"

ResourceApi {

    resource: "/DigitalBloom"

    function data(query){

        _rest.get(resource+ "/messages", query)
        return promise
    }


}
