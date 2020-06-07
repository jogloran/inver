//
//  ViewController.m
//  CHRLoupe
//
//  Created by Daniel Tse on 6/6/20.
//  Copyright © 2020 Daniel Tse. All rights reserved.
//

#import "ViewController.h"
#import "NESCHRTileItem.h"
#import "HeaderView.h"
#import "CHRDataSource.h"

@implementation ViewController

- (void)viewDidLoad {
  [super viewDidLoad];

  // Do any additional setup after loading the view.
  NSString* path = [[NSBundle mainBundle] pathForResource: @"smb2" ofType: @"nes"];
  NSData* bytes = [NSData dataWithContentsOfURL: [NSURL fileURLWithPath: path]];
  char* ptr = (char*) [bytes bytes];
  self.dataSource = [[CHRDataSource alloc] initWithChrData: ptr length: [bytes length]];
  self.collectionView.dataSource = self.dataSource;
  [self.collectionView registerClass: [NESCHRTileItem class] forItemWithIdentifier: @"NESCHRTileItem"];
  
}

- (NSSize)collectionView:(NSCollectionView *)collectionView layout:(NSCollectionViewLayout *)collectionViewLayout referenceSizeForHeaderInSection:(NSInteger)section {
  return NSMakeSize(1000, 40);
}

- (NSCollectionViewItem *)collectionView:(NSCollectionView *)collectionView itemForRepresentedObjectAtIndexPath:(NSIndexPath *)indexPath {
   NESCHRTileItem* item = (NESCHRTileItem*)[collectionView makeItemWithIdentifier: @"NESCHRTileItem"
                            forIndexPath:indexPath];
  NSString* path = [[NSBundle mainBundle] pathForResource: @"mm" ofType: @"png"];
  item.imageView.image = [[NSImage alloc] initWithContentsOfFile: path];
  return item;
}

- (void)setRepresentedObject:(id)representedObject {
  [super setRepresentedObject:representedObject];

  // Update the view, if already loaded.
}


@end
