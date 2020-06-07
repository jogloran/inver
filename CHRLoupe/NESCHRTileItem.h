//
//  NESCHRTileItem.h
//  CHRLoupe
//
//  Created by Daniel Tse on 6/6/20.
//  Copyright © 2020 Daniel Tse. All rights reserved.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface NESCHRTileItem : NSCollectionViewItem

@property (weak) IBOutlet NSImageView* tile;
@property (strong, nonatomic) NSImage* img;

- (char*) bufPtr;

@end

NS_ASSUME_NONNULL_END
