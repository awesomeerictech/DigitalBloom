import QtQuick 2.7
import Quickeric 1.0

Urls

{
  root: app.production? "https://www.awesomeerictech.com" :  "https://www.awesomeerictech.com"
 //  root: app.production? "https://0.0.0.0" :  "https://0.0.0.0"
    // custom urls
  property string uploadUrl: app.production? "https://www.awesomeerictech.com" :  "https://www.awesomeerictech.com"
   // property string uploadUrl: app.production? "https://0.0.0.0" :  "https://0.0.0.0"
}
