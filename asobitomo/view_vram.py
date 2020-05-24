import matplotlib.pyplot as plt
import numpy as np

import sys

tiles_fn = sys.argv[1]
data = np.fromfile(open(tiles_fn), dtype=np.uint8, sep=' ').reshape((512, 16))

btt_fn = sys.argv[2]
btt = np.fromfile(open(btt_fn), dtype=np.uint8, sep=' ').reshape((32, 32))

tiles = [
    np.unpackbits(datum).reshape((16, 8))
    for datum in data
]

tiles_ = [(tile[::2] << 1) | tile[1::2] for tile in tiles]
# # fig, ax = plt.subplots(16, 16)
# for i, tile in enumerate(tiles_):
#     plt.subplot(16, 16, i+1)
#     plt.xticks([])
#     plt.yticks([])
#     plt.imshow(tile, cmap='gray')
# plt.show()

screen = np.zeros((144, 160))
for row in range(20):
    for col in range(18):
        screen[8*col:8*(col+1), 8*row:8*(row+1)] = tiles_[btt[col, row]]

plt.imshow(screen)
plt.show()
