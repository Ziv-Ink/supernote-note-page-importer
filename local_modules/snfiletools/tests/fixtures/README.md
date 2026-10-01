`bitmap-test.rle` is the unchanged synthetic chart from
`tools/read_file_on_device_cpp/research/experiments/bitmap-replacement/replacement.rle`.
It encodes three square outlines, an 8 by 6 checkerboard, and three horizontal
bars on a 1920 by 2560 canvas. Palette codes are 0x61 and 0x62; ordinary
RATTA_RLE pairs encode runs of 1–128 pixels. Decoding yields 4,915,200 pixels.

Payload: 94,324 bytes.
SHA-256: 59f3b847bd635ba0e695a23a6c64bc1012b91486caecac54d4349b78e1872ff1.

The device diagnostic expects this asset at
`/storage/emulated/0/Note/snfiletools-test-assets/bitmap-test.rle`.
This bitmap is a rendering test and contains no vector strokes.
