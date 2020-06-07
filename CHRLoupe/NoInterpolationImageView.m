//
//  NoInterpolationImageView.m
//  CHRLoupe
//
//  Created by Daniel Tse on 6/6/20.
//  Copyright © 2020 Daniel Tse. All rights reserved.
//

#import "NoInterpolationImageView.h"

@implementation NoInterpolationImageView

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    // Drawing code here.
  [[NSGraphicsContext currentContext] setImageInterpolation: NSImageInterpolationNone];
}

@end
