import QtQuick 2.0
import Quickeric 1.0

import "../"

ResourceApi {

    resource: "/DigitalBloom/menu"

    function data(query){

        _rest.get(resource+ "/all", query)
        return promise
    }


}
