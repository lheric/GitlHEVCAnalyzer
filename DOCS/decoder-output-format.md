# Decoder output format

The GUI analyses a stream by running the HM-18 decoder (`decoders/HM_180`) and
reading the files it writes. This document is the contract between the two:

- **Writer:** `appgitlhevcdecoder/HM_18.0/TLibSysuAnalyzer/TSysuAnalyzerOutput.cpp`,
  called from the hooks in `appgitlhevcdecoder/HM_18.0/sysu-analyzer.patch`.
  The YUV files are written from `TAppDecTop`.
- **Readers:** `src/parsers/*` (text files), `src/model/io/yuv420rgbbuffer.cpp`
  (YUV files), driven by `src/commands/openbitstreamcommand.cpp`.

Change both sides together, and keep the output byte-compatible unless the
readers are updated in the same change.

## Invocation

```
<application dir>/decoders/HM_180 -b <bitstream> -o decoder_yuv.yuv -d 8
```

`-d 8` makes HM write the reconstruction (and the prediction, which follows the
same setting) as 8-bit, whatever the stream's bit depth. The GUI displays
8-bit YUV only.

- **Working directory:** `<cache folder>/<sequence index>/`, where the cache
  folder is the `cache_path` preference. Every output file is written there,
  using the fixed names below.
- **Exit status:** must be 0, or the GUI reports a decoding failure.
- **stdout:** captured by the GUI into `decoder_general.txt` in the same folder.
  While decoding, each line matching `POC *(-?[0-9]+)` is shown as progress,
  "POC n Decoded".
- **Order of work:** the GUI waits for the process to exit, then parses the
  files in this order: SPS, general, CU/PU, TU, pred, MV, merge, intra, bits
  (CTU, then CU), tiles.

## Files

| File | Written | Read by | Content |
| --- | --- | --- | --- |
| `decoder_general.txt` | stdout | `DecoderGeneralParser` | Version, one line per picture (POC, decode time, reference lists), total time |
| `decoder_sps.txt` | once per SPS | `SpsParser` | Picture size, CTU size, depths, bit depth |
| `decoder_cupu.txt` | per CTU | `CUPUParser` | CU quadtree and partition (PU) mode of each leaf CU |
| `decoder_tu.txt` | per CTU | `TUParser` | TU quadtree of each leaf CU |
| `decoder_pred.txt` | per CTU | `PredParser` | Prediction mode of each PU |
| `decoder_mv.txt` | per CTU | `MVParser` | Inter direction, reference POCs and MVs of each PU |
| `decoder_merge.txt` | per CTU | `MergeParser` | Merge index of each PU |
| `decoder_intra.txt` | per CTU | `IntraParser` | Luma and chroma intra modes of each PU |
| `decoder_bit_lcu.txt` | per CTU | `BitParser::parseLCUBitFile` | Bytes used by the CTU |
| `decoder_bit_scu.txt` | per CTU | `BitParser::parseSCUBitFile` | Bytes used by each leaf CU |
| `decoder_tile.txt` | per picture | `TileParser` | Tile layout |
| `decoder_yuv.yuv` | per output picture | frame buffer | Reconstruction |
| `pred_yuv.yuv` | per output picture | frame buffer | Prediction |
| `resi_yuv.yuv` | per output picture | frame buffer | Residual |
| `encoder_me.txt` | per CTU | none | Legacy encoder output; only the line prefix is written |

All text files are ASCII, one record per line, values separated by single
spaces. A line may end with a trailing space.

## `decoder_general.txt`

The decoder's console output. The GUI uses three kinds of lines and ignores
everything else, so extra lines (warnings, SEI messages) are harmless.

```
HM software: Decoder Version [18.0] (including RExt)[Mac OS X][GCC 4.2.1][64 bit]
POC    0 TId: 0 ( I-SLICE, QP 32 ) [DT  0.117] [L0 ] [L1 ] [:,(unk)]
POC    8 TId: 0 ( B-SLICE, QP 33 ) [DT  0.072] [L0 0 ] [L1 0 ] [:,(unk)]
...
 Total Time:        1.234 sec.
```

| Line | Pattern the GUI matches | Used for |
| --- | --- | --- |
| Version | `HM software: Decoder Version \[([0-9.]+)\].*` | Stored as the sequence's encoder version (not otherwise used); the picture lines are searched for after it |
| Picture | `POC *(-?[0-9]+).*\[DT *([0-9.]+) *\] \[L0(( -?[0-9]+){0,}) \] \[L1(( -?[0-9]+){0,}) \]` | One `ComFrame` per line: POC, decode time in seconds, L0/L1 reference POCs |
| Total | ` Total Time: *([0-9.]+).*` | Total decode time in seconds |

- **Picture lines** come in decoding order, one per decoded picture. Their
  order defines the decoding-order index that every per-CTU file is mapped to.
- **Display order** is derived from these lines by sorting on POC within each
  group of pictures that starts at an IDR (POC 0). The first picture always
  starts a group, so a stream need not begin at POC 0.
- **No other line positions are assumed.** HM-18 prints the first picture line
  directly after the version line. HM-12 printed a blank line first; the parser
  used to skip it and lost picture 0 with HM-18.

## `decoder_sps.txt`

One block of seven lines per SPS received. The GUI reads the first value of
each key it knows, in this order, and ignores everything else.

```
Resolution:408x240
Max CU Size:64
Max CU Depth:4
Max Inter TU Depth:1
Max Intra TU Depth:1
Input Bit Depth:8
Conformance Window:0 6 0 6
```

| Key | Value |
| --- | --- |
| `Resolution` | `pic_width_in_luma_samples` x `pic_height_in_luma_samples`: the full coded size, which is also the frame size of every YUV file |
| `Max CU Size` | CTU size in luma samples |
| `Max CU Depth` | `MaxTotalCUDepth`: CU depth plus the extra depth that gets down to the smallest TU. The CTU holds 4^depth units of 4x4 samples, which the readers use for z-order offsets. |
| `Max Inter TU Depth`, `Max Intra TU Depth` | `max_transform_hierarchy_depth_inter` / `_intra` |
| `Input Bit Depth` | Luma bit depth of the stream (the YUV files are 8-bit regardless; see Invocation) |
| `Conformance Window` | Left, right, top and bottom offsets in luma samples. Informational only: the GUI does not read it, and the YUV files are **not** cropped to it. |

## Per-CTU files

`decoder_cupu`, `_tu`, `_pred`, `_mv`, `_merge`, `_intra`, `_bit_lcu` and
`_bit_scu` all have exactly one line per CTU:

```
<POC,CtuAddr> v1 v2 v3 ...
```

- **`POC`** is the picture's POC and **`CtuAddr`** the CTU's raster-scan
  address in the picture (`x = CtuAddr % ctusPerRow`).
- **Lines are in decoding order:** pictures in decoding order and, within a
  picture, CTUs in the order they are decoded (slice and tile scan). With tiles
  this is not raster order. The CU/PU reader sorts each picture's CTUs by
  address afterwards.
- **Matching a line to a picture** (`DecOrderFrameTracker`): a new picture
  starts when the POC changes, or when a `CtuAddr` repeats within the current
  picture. The second rule is what separates consecutive pictures that share a
  POC, such as all-IDR streams where every picture is POC 0. The n-th picture
  found this way is the n-th picture line of `decoder_general.txt`.
- **Every picture has a line for every CTU,** including CTUs that lie partly
  outside the picture.

### Traversal order

The values after the prefix walk the CTU's CU quadtree depth first, in z-order
(top-left, top-right, bottom-left, bottom-right). `decoder_cupu.txt` defines
the tree; every other per-CTU file is read by walking the tree that
`decoder_cupu.txt` built, and consumes values only at leaf CUs:

| File | Values per non-leaf CU | Values per leaf CU |
| --- | --- | --- |
| `decoder_cupu.txt` | `99` | its partition mode (1 value) |
| `decoder_tu.txt` | none | the leaf's TU quadtree (below) |
| `decoder_pred.txt`, `_merge`, `_intra`, `_mv` | none | one record per PU, in PU order |
| `decoder_bit_scu.txt` | none | 1 value |
| `decoder_bit_lcu.txt` | none (1 value for the whole CTU) | none |

A file whose value count doesn't match the tree trips an assert in debug
builds and misreads data in release builds.

### `decoder_cupu.txt`: CU tree and partition mode

- `99` means the CU is split into four. The next four subtrees follow.
- Any other value means a leaf CU with this partition mode:

| Value | Mode | PUs, in PU order |
| --- | --- | --- |
| 0 | 2Nx2N | 1 |
| 1 | 2NxN | 2: top, bottom |
| 2 | Nx2N | 2: left, right |
| 3 | NxN | 4: z-order |
| 4 | 2NxnU | 2: top (N/2 high), bottom |
| 5 | 2NxnD | 2: top, bottom (N/2 high) |
| 6 | nLx2N | 2: left (N/2 wide), right |
| 7 | nRx2N | 2: left, right (N/2 wide) |
| 15 | none | 0: the CU lies outside the picture |

HM-18 marks "no partition" internally as `NUMBER_OF_PART_SIZES` (8). The writer
outputs it as 15, the value older HM versions used, and that the GUI's
`SIZE_NONE` expects.

Example (illustrative): a 64x64 CTU split into four 32x32 CUs. The second of
those is split again into four 16x16 CUs.

```
<8,3> 99 0 99 0 4 1 0 1 0
```

The leaves in order: 32x32 2Nx2N, 16x16 2Nx2N, 16x16 2NxnU, 16x16 2NxN,
16x16 2Nx2N, 32x32 2NxN, 32x32 2Nx2N. That makes 10 PUs, so the PU-level files
have 10 records for this CTU.

### `decoder_tu.txt`: TU tree of each leaf CU

For each leaf CU, in order, its transform quadtree, written the same way as
the CU tree: `99` means split into four, any other value is a leaf TU. The
leaf value is the TU's transform depth relative to the CU, and the GUI only
checks it against 99. A CU outside the picture still has a TU tree, usually
`0`.

Example for the 7 leaves above, where the sixth leaf (a 32x32 CU) has its
residual split once:

```
<8,3> 0 0 0 0 0 99 1 1 1 1 0
```

### `decoder_pred.txt`: prediction mode per PU

| Value | Meaning |
| --- | --- |
| 1 | Inter (skipped CUs included) |
| 2 | Intra |
| 15 | None (outside the picture) |

The GUI's `PredMode` enum also has `0 = MODE_SKIP`. HM removed skip as a
separate prediction mode after HM-8.0, so the decoder never writes 0.

### `decoder_merge.txt`: merge index per PU

`-1` when the PU is not merged, otherwise the merge candidate index (0 to 4).

### `decoder_intra.txt`: intra modes per PU

Two values per PU: the luma mode, then the chroma mode.

- **Luma:** 0 planar, 1 DC, 2 to 34 angular.
- **Chroma:** 0 to 34 as luma, or 36 for "derived from luma" (DM).
- **Only meaningful when the PU's prediction mode is intra.** For inter PUs the
  values are whatever HM leaves in its arrays.

### `decoder_mv.txt`: motion per PU

For each PU, the inter direction followed by 0, 1 or 2 motion records:

| Inter direction | Values that follow |
| --- | --- |
| 0 (none: intra, or outside the picture) | nothing |
| 1 (list 0) | `refPOC mvX mvY` |
| 2 (list 1) | `refPOC mvX mvY` |
| 3 (bi-prediction) | `refPOC0 mvX0 mvY0 refPOC1 mvX1 mvY1`, list 0 first |

`refPOC` is the POC of the reference picture, not the reference index.
`mvX` and `mvY` are in quarter luma samples.

Example (illustrative), one PU predicted from POC 4, moved 3 samples left and
2 down:

```
1 4 -12 8
```

### `decoder_bit_lcu.txt` and `decoder_bit_scu.txt`: coded size

- **`decoder_bit_lcu.txt`:** one value per CTU line, the number of **bytes**
  the CTU took in the bitstream. Despite the file names, both files count bytes.
- **`decoder_bit_scu.txt`:** one value per leaf CU, the bytes that CU took.
  CUs outside the picture are 0.
- **Accuracy:** measured as the change in the substream's byte position around
  the CTU or CU. CABAC reads ahead, so a single CU's value is approximate. The
  sums are close to the real slice data size.
- **Where the GUI uses them:** a frame's bit count is the sum of its CTU values
  (timeline). The per-CTU values feed the bit heat-map filter.

## `decoder_tile.txt`

One line per tile, written once per picture (for its first slice), tiles in
raster order:

```
<POC,NumTiles> FirstCtuAddr WidthInCtus HeightInCtus
```

```
<0,25> 0 4 8
<0,25> 4 10 8
<0,25> 14 5 8
...
```

Lines are matched to pictures with the same rule as the per-CTU files,
using `FirstCtuAddr` as the address. A picture without tiles has one line
covering the whole picture.

## YUV files

All three files are planar YUV 4:2:0 (Y, then Cb, then Cr), one frame per
output picture in **display (output) order**.

**Terms**, with a 402x234 source as the example:

| Term | Meaning | Example |
| --- | --- | --- |
| Coded size | The size the encoder actually codes, rounded up to a multiple of the minimum CU size, so it can include padding. `Resolution` in `decoder_sps.txt`. | 408x240 |
| Padding | Rows and columns the encoder adds at the edges to reach the coded size; not part of the source image | 6 columns right, 6 rows bottom |
| Conformance window | The part of the coded picture that is the source image, stored as the amount to trim from each edge (= the padding). `Conformance Window` in `decoder_sps.txt`. | 0 6 0 6 (left right top bottom) |
| Cropped size | Coded size minus the conformance-window offsets: what a standard decoder outputs | 402x234 |

Most streams have no padding: coded size = cropped size, window `0 0 0 0`.

**Frame size: the full coded size, padding included** (`Resolution` in
`decoder_sps.txt`, which is also the frame size the GUI reads), **not** the
cropped size. Stock HM crops; the analyzer
patch disables that (`SYSU_CROP` in `TAppDecTop.cpp`). This keeps the
pictures aligned with the CTU/CU coordinates in the per-CTU files, which start
at the coded picture's top-left. With a conformance window, the edge rows and
columns are the encoder's padding. Cropping a frame with the offsets from
`Conformance Window` gives the standard decoder output: the cropped
`CONFWIN_A_Sony_1` output matches the conformance package's MD5.

| File | Sample format | GUI view |
| --- | --- | --- |
| `decoder_yuv.yuv` | 8-bit unsigned (from `-d 8`) | "reconstructed": after deblocking and SAO |
| `pred_yuv.yuv` | 8-bit unsigned (follows `-d 8`) | "predicted": intra/inter prediction before the residual is added; a PCM CU's samples are copied as its prediction |
| `resi_yuv.yuv` | 16-bit little-endian, signed two's complement, at the stream's internal bit depth | "residual": the GUI shows `clip(value + 128, 0, 255)` |

For 8-bit streams without loop filters, `clip(pred + resi)` equals the
reconstruction exactly. With deblocking or SAO enabled it does not, because the
reconstruction is filtered. For higher bit depths it does not either, because
the prediction is scaled to 8 bits but the residual is not.

## Known gaps

- **Residual of high bit-depth streams.** It is written at the stream's bit
  depth, so a 10-bit residual is about four times larger than an 8-bit one,
  and the GUI's `+128` display saturates sooner.
- **No partial results.** The GUI parses only after the decoder exits. See the
  frame-by-frame decoding design for the planned replacement of this text
  protocol.
