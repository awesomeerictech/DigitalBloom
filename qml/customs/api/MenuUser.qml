import QtQuick 2.0
import Quickeric 1.0
import "../"

ResourceApi {

    resource: "/DigitalBloom/menu/"

    function data(query,user) {

        _rest.get(resource+String(user), query)
        return promise


    }


}
