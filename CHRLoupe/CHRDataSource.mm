//
//  CHRDataSource.m
//  CHRLoupe
//
//  Created by Daniel Tse on 6/6/20.
//  Copyright © 2020 Daniel Tse. All rights reserved.
//

#import "CHRDataSource.h"
#import "NESCHRTileItem.h"
#import "HeaderView.h"
#import "utils.hpp"

#include <algorithm>
#include <vector>
#include <iostream>
#include "header.hpp"

@implementation CHRDataSource

std::vector<char> chr;

- (id)initWithChrData:(char *)ptr length:(NSUInteger)len {
  chr.reserve(len);
  NESHeader* h = reinterpret_cast<NESHeader*>(ptr);
  size_t offset = 0x10 + prg_rom_size(h) * 0x4000;
  std::copy(ptr + offset, ptr + offset + chr_rom_size(h) * 0x2000, std::back_inserter(chr));
  return self;
}

void releaser(void* info, const void* data, size_t size) {
  delete[] ((byte*) data);
}

- (nonnull NSCollectionViewItem *)collectionView:(nonnull NSCollectionView *)collectionView itemForRepresentedObjectAtIndexPath:(nonnull NSIndexPath *)indexPath {
  NESCHRTileItem* item = (NESCHRTileItem*)[collectionView makeItemWithIdentifier: @"NESCHRTileItem"
                            forIndexPath:indexPath];
  long offset = indexPath.section * 0x400 + 16 * indexPath.item;

  byte* buf = new byte[64*4];
  byte* ptr = buf;
  for (int i = 0; i < 8; ++i) {
    std::array<byte, 8> row = unpack_bits(chr[offset + i], chr[offset + i + 8]);
    for (int col = 0; col < 8; ++col) {
      *ptr++ = row[col] * 64;
      *ptr++ = row[col] * 64;
      *ptr++ = row[col] * 64;
      *ptr++ = 255;
    }
  }
  CGDataProviderRef provider = CGDataProviderCreateWithData(nullptr, buf, 64*4, releaser);
  size_t bitsPerComponent = 8;
  size_t bitsPerPixel = 32;
  size_t bytesPerRow = 4 * 8;
  CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
  CGColorRenderingIntent intent = kCGRenderingIntentDefault;
  CGBitmapInfo info = kCGImageByteOrderDefault | kCGImageAlphaPremultipliedLast;
  CGImageRef iref = CGImageCreate(8, 8, bitsPerComponent, bitsPerPixel, bytesPerRow, colorSpaceRef, info, provider, nullptr, false, intent);
  item.imageView.image = [[NSImage alloc] initWithCGImage: iref size: NSMakeSize(8, 8)];
  [item.imageView.layer setMagnificationFilter: kCAFilterNearest];
  return item;
}

- (NSView *)collectionView:(NSCollectionView *)collectionView viewForSupplementaryElementOfKind:(NSCollectionViewSupplementaryElementKind)kind atIndexPath:(NSIndexPath *)indexPath {
   HeaderView* headerView = [collectionView makeSupplementaryViewOfKind: NSCollectionElementKindSectionHeader withIdentifier: @"HeaderView" forIndexPath: indexPath];
  headerView.header.stringValue = [NSString stringWithFormat: @"Page %02X", indexPath.section];
  return headerView;
}

- (NSInteger)collectionView:(nonnull NSCollectionView *)collectionView numberOfItemsInSection:(NSInteger)section {
  return 64;
}

- (NSInteger)numberOfSectionsInCollectionView:(NSCollectionView *)collectionView {
  return chr.size() / 0x400;
}

@end
