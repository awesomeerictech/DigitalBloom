import QtQuick 2.0
import Quickeric 1.0
import "api"



Item {



    property AllCategories catgetall:  AllCategories   {}
    property AllMenus menugetall:  AllMenus {}
    property AllMessages messagegetall:  AllMessages {}
    property AllQr qrgetall:  AllQr {}
    property AllUsers usersgetall:  AllUsers {}
    property CategoryUser catusergetall:  CategoryUser {}
    property MenuUser menuusergetall:  MenuUser {}
    property DataUser datausergetall:  DataUser {}

    property ResourceApi registernew: ResourceApi {
        resource: "/DigitalBloom/ids/new"
    }

    property ResourceApi reglogin: ResourceApi {
        resource: "/DigitalBloom/ids/login"
    }

    property ResourceApi reghint: ResourceApi {
        resource: "/DigitalBloom/ids/hint"
    }

    property ResourceApi regsub: ResourceApi {
        resource: "/DigitalBloom/ids/subqrmenu"
    }

    property ResourceApi regphoneupdate: ResourceApi {
        resource: "/DigitalBloom/ids/phoneupdate"
    }

    property ResourceApi regemailupdate: ResourceApi {
        resource: "/DigitalBloom/ids/emailupdate"
    }

    property ResourceApi regmainurl: ResourceApi {
        resource: "/DigitalBloom/ids/mainurl"
    }


    property ResourceApi regpassedit: ResourceApi {
        resource: "/DigitalBloom/ids/secure"
    }

    property ResourceApi block: ResourceApi {
        resource: "/DigitalBloom/ids/blockunblock"
    }

    property ResourceApi admin: ResourceApi {
        resource: "/DigitalBloom/ids/adminpanel"
    }

    property ResourceApi signout: ResourceApi {
        resource: "/DigitalBloom/ids/logout"
    }

    property ResourceApi deleteacc: ResourceApi {
        resource: "/DigitalBloom/ids/deletebiz"
    }

    property ResourceApi catnew: ResourceApi {
        resource: "/DigitalBloom/category/new"
    }

    property ResourceApi catupdate: ResourceApi {
        resource: "/DigitalBloom/category/update"
    }

    property ResourceApi catuserpost: ResourceApi {
        resource: "/DigitalBloom/category/user"

    }

    property ResourceApi catdelete: ResourceApi {
        resource: "/DigitalBloom/category/delete"
    }


    property ResourceApi menunew: ResourceApi {
        resource: "/DigitalBloom/menu/new"
    }

    property ResourceApi menuitempost: ResourceApi {
       resource: "/DigitalBloom/menu/user"
   }

  property ResourceApi menuupdate: ResourceApi {
        resource: "/DigitalBloom/menu/update"
    }
    
    property ResourceApi menudelete: ResourceApi {
        resource: "/DigitalBloom/menu/delete"
    }
    
    property ResourceApi qrnew: ResourceApi {
        resource: "/DigitalBloom/qr/new"
    }
    
    property ResourceApi qruserpost: ResourceApi {
        resource: "/DigitalBloom/qr/user"
    }
    
    property ResourceApi qrupdate: ResourceApi {
        resource: "/DigitalBloom/qr/update"
    }
    
     property ResourceApi qrdelete: ResourceApi {
        resource: "/DigitalBloom/qr/delete"
    }
    
    property ResourceApi messages : ResourceApi {
        resource: "/DigitalBloom/messages"
    }
    
    property ResourceApi messagenew: ResourceApi {
        resource: "/DigitalBloom/message/new"
    }





}
