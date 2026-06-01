#import "docviewcontroller_ios.h"

/* ************************************************************************** */

@interface DocViewController ()
@end
@implementation DocViewController
#pragma mark -
#pragma mark View Life Cycle
- (void)viewDidLoad {
    [super viewDidLoad];
}
#pragma mark -
#pragma mark Document Interaction Controller Delegate Methods
- (UIViewController *)documentInteractionControllerViewControllerForPreview: (UIDocumentInteractionController *) controller {
#pragma unused (controller)
    return self;
}
- (void)documentInteractionControllerDidEndPreview: (UIDocumentInteractionController *) controller {
#pragma unused (controller)
    self.mIosShareUtils->handleDocumentPreviewDone(self.requestId);
    [self removeFromParentViewController];
}
@end

/* ************************************************************************** */
