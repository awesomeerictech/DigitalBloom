#ifndef DOCVIEWCONTROLLER_IOS_H
#define DOCVIEWCONTROLLER_IOS_H
/* ************************************************************************** */

#import "SharingUtils_ios.h"

#import <UIKit/UIKit.h>

/* ************************************************************************** */

@interface DocViewController : UIViewController <UIDocumentInteractionControllerDelegate>

@property int requestId;

@property IosShareUtils *mIosShareUtils;

@end

/* ************************************************************************** */
#endif // DOCVIEWCONTROLLER_IOS_H
