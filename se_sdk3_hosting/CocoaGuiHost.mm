//
//  CocoaGuiHost.m
//  Standalone
//
//  Created by Jenkins on 22/09/17.
//  Copyright © 2017 Jenkins. All rights reserved.
//

//#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import "EventHelper.h"
//#include "CocoaGuiHost.h"



@implementation SYNTHEDIT_EVENT_HELPER_CLASSNAME

- (void)initWithClient:(EventHelperClient*)pclient
{
    client = pclient;
}

- (void)menuItemSelected: (id) sender
{
 //   drawingFrame.onMenuItemSelected(sender);
    client->CallbackFromCocoa(sender);
}

- (void)endEditing: (id) sender
{
    client->CallbackFromCocoa(sender);
}

- (void)onMenuAction: (id) sender
{
   client->CallbackFromCocoa(sender);
}


@end
