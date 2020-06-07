//
//  ViewController.h
//  CHRLoupe
//
//  Created by Daniel Tse on 6/6/20.
//  Copyright © 2020 Daniel Tse. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "CHRDataSource.h"

@interface ViewController : NSViewController<NSCollectionViewDelegateFlowLayout>

@property (weak) IBOutlet NSCollectionView* collectionView;
@property (strong, nonatomic) CHRDataSource* dataSource;

@end

