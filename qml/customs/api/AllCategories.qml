import QtQuick 2.0
import Quickeric 1.0

import "../"

ResourceApi {

    resource: "/DigitalBloom/category"

    function data(query){

        _rest.get(resource+ "/all", query)
        return promise
    }


}
