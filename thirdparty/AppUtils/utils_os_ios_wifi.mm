#include "utils_os_ios_wifi.h"

#if defined(Q_OS_IOS)

#import <SystemConfiguration/CaptiveNetwork.h>

/* ************************************************************************** */

QString UtilsIOSWiFi::getWifiSSID()
{
    NSString *ssid = nil;
    NSArray *interfaces = (__bridge_transfer id)CNCopySupportedInterfaces();

    for (NSString *interfaceName in interfaces)
    {
        NSDictionary *networkInfo = (__bridge_transfer id)CNCopyCurrentNetworkInfo((__bridge CFStringRef)interfaceName);
        if (networkInfo[@"SSID"])
        {
            ssid = networkInfo[@"SSID"];
            break;
        }
    }

    return QString::fromNSString(ssid);
}

/* ************************************************************************** */
#endif // Q_OS_IOS
