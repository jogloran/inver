//
//  CHRDataSource.h
//  CHRLoupe
//
//  Created by Daniel Tse on 6/6/20.
//  Copyright © 2020 Daniel Tse. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface CHRDataSource : NSObject <NSCollectionViewDataSource>

- initWithChrData: (char*) ptr length: (NSUInteger) len;

@end

NS_ASSUME_NONNULL_END
