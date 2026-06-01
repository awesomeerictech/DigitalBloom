#include "utils_os_ios.h"

#if defined(Q_OS_IOS)

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

/* ************************************************************************** */

void UtilsIOS::screenKeepOn(bool on)
{
    if (on)
    {
        [[UIApplication sharedApplication] setIdleTimerDisabled: YES];
    }
    else
    {
        [[UIApplication sharedApplication] setIdleTimerDisabled: NO];
    }
}

// For reference:
//enum ScreenOrientation_iOS {
//    UIInterfaceOrientationUnknown = 0,          // The orientation of the device is unknown.
//    UIInterfaceOrientationPortrait,             // The device is in portrait mode, with the device upright and the Home button on the bottom.
//    UIInterfaceOrientationPortraitUpsideDown,   // The device is in portrait mode but is upside down, with the device upright and the Home button at the top.
//    UIInterfaceOrientationLandscapeLeft,        // The device is in landscape mode, with the device upright and the Home button on the left.
//    UIInterfaceOrientationLandscapeRight,       // The device is in landscape mode, with the device upright and the Home button on the right.
//};

void UtilsIOS::screenLockOrientation(int orientation)
{
    NSNumber *value = [NSNumber numberWithInt:UIInterfaceOrientationLandscapeRight];
    if (orientation != 0) value = [NSNumber numberWithInt:UIInterfaceOrientationPortrait];

    [[UIDevice currentDevice] setValue:value forKey:@"orientation"];
}

void UtilsIOS::screenLockOrientation(int orientation, bool autoRotate)
{
    NSNumber *value = [NSNumber numberWithInt:UIInterfaceOrientationUnknown];

    if (orientation == 0 || autoRotate) value = [NSNumber numberWithInt:UIInterfaceOrientationUnknown];
    else if (orientation == 1) value = [NSNumber numberWithInt:UIInterfaceOrientationPortrait];
    else if (orientation == 2) value = [NSNumber numberWithInt:UIInterfaceOrientationPortraitUpsideDown];
    else if (orientation == 4) value = [NSNumber numberWithInt:UIInterfaceOrientationLandscapeLeft];
    else if (orientation == 8) value = [NSNumber numberWithInt:UIInterfaceOrientationLandscapeRight];

    [[UIDevice currentDevice] setValue:value forKey:@"orientation"];
}

/* ************************************************************************** */

void UtilsIOS::vibrate(int ms)
{
    Q_UNUSED(ms)

    UISelectionFeedbackGenerator *generator = [[UISelectionFeedbackGenerator alloc] init];
    [generator prepare];
    [generator selectionChanged];
    generator = nil;
}

/* ************************************************************************** */
#endif // Q_OS_IOS
