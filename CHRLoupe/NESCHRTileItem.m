//
//  NESCHRTileItem.m
//  CHRLoupe
//
//  Created by Daniel Tse on 6/6/20.
//  Copyright © 2020 Daniel Tse. All rights reserved.
//

#import "NESCHRTileItem.h"

@interface NESCHRTileItem ()

@end

@implementation NESCHRTileItem

char buf[64];

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
}

- (void)setImg:(NSImage *)img {
  self.tile.image = img;
}

- (char*)bufPtr {
  return buf;
}

@end
