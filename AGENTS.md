# AGENTS.md

Guidance for AI coding agents working in this repository.

Gitl HEVC Analyzer is a Qt 5 / C++11 desktop tool for analysing HEVC (H.265)
bitstreams. It runs a modified HM reference decoder as a separate process,
parses the text/YUV files that decoder writes, and draws CU/PU/TU, MV, intra
mode, bit heat-map etc. on top of the decoded frames through plugin "filters".

## Build

Everything is qmake. Submodules are required:

```bash
git submodule update --init --recursive
mkdir -p build && cd build                      # build/ is gitignored
qmake -r ../GitlHEVCAnalyzer.pro "CONFIG+=release" "CONFIG-=debug debug_and_release"
make -j"$(getconf _NPROCESSORS_ONLN)"           # Windows/MSVC: nmake
```

- Add `"CONFIG+=sdk_no_version_check"` on macOS (Homebrew `qt@5`) to silence
  Qt's SDK-version warning.
- Output layout in the build dir: `Gitl_HEVC_Analyzer` (GUI),
  `decoders/HM_180` (the only decoder), `plugins/` (filter libraries).
  The GUI finds `decoders/` next to its binary but `plugins/` relative to the
  working directory, so run it from the build dir.
- To build just the decoder: `qmake -r ../appgitlhevcdecoder/HM_18.0/HM_18.0.pro && make`
  (puts it in `decoders/HM_180` under that build dir).
- CI: `.github/workflows/build.yml` builds the whole tree on Linux (apt Qt 5),
  macOS (Homebrew `qt@5`) and Windows (Qt 5.15.2 + MSVC, nmake).

## Tests

Only libgitlmvc has tests (QtTest). Both suites are built by the normal build and
are both named `TestGitlEvtBus`:

```bash
./build/libgitlmvc/test/TestGitlEvtBus                  # MVC front controller / commands
./build/libgitlmvc/libgitlevtbus/test/TestGitlEvtBus    # event bus
./build/libgitlmvc/test/TestGitlEvtBus firParamTest     # single test function (QtTest)
```

There are no tests for the GUI, the plugins or the decoder. To check a decoder
change, decode a stream before and after and compare every output file:
`decoders/HM_180 -b in.265 -o decoder_yuv.yuv` in an empty directory
(x265/ffmpeg can make test streams). Older outputs to compare against: build
the `legacy-decoders` tag, whose HM_120 produces byte-identical analysis files
on Main profile streams.

## Architecture

### Top-level projects (`GitlHEVCAnalyzer.pro`, ordered `SUBDIRS`)

| Dir | What |
|---|---|
| `appgitlhevcdecoder/` | The HM-18 decoder the GUI shells out to (`HM_18.0/`), including `TLibSysuAnalyzer/`, the analysis-output library |
| `libgitlmvc/` | Submodule: tiny MVC framework (front controller + commands) on top of `libgitlevtbus/` (event bus, nested submodule) |
| `src/` | The GUI application |
| `libplugins/` | Drawing filters, each a Qt plugin loaded at runtime from `plugins/` |

### GUI: MVC over an event bus (libgitlmvc)

- Views never call the model directly. They dispatch
  `GitlIvkCmdEvt("command_name")` with parameters; `AppFrontController`
  (`src/commands/appfrontcontroller.cpp`) maps command names to
  `GitlAbstractCommand` subclasses in `src/commands/` and runs `execute()`.
  **A new command must be added to `s_sCmdTable` there.**
- Commands mutate the model (`ModelLocator` singleton in `src/model/`:
  sequences, frame buffer, draw engine, preferences) and notify views with
  `GitlUpdateUIEvt` parameters; views subscribe via `listenToParams(...)`.

### Decoder ↔ GUI contract

`OpenBitstreamCommand` → `BitstreamParser` runs
`<application dir>/decoders/HM_180 -b <bitstream> -o decoder_yuv.yuv` with the
working directory set to `<cache folder>/<sequence index>` (cache folder = the
`cache_path` preference, default `./cache`). There is no version choice: HM-18
is the only decoder. The decoder must write, into its working directory:

- stdout → `decoder_general.txt` (the GUI regex-parses
  `HM software: Decoder Version [x.y]` and the per-picture `POC … [DT …] [L0 …] [L1 …]` lines)
- `decoder_sps.txt`, `decoder_cupu.txt`, `decoder_tu.txt`, `decoder_pred.txt`,
  `decoder_mv.txt`, `decoder_merge.txt`, `decoder_intra.txt`,
  `decoder_bit_lcu.txt`, `decoder_bit_scu.txt`, `decoder_tile.txt`
  (one line per CTU, `<POC,CTU addr>` prefix; parsers in `src/parsers/`)
- `decoder_yuv.yuv` (reconstruction), `resi_yuv.yuv` (residual, signed
  16-bit), `pred_yuv.yuv` (prediction)

All of these are produced by `TLibSysuAnalyzer/TSysuAnalyzerOutput`. Keep the
file formats stable: any decoder change must keep the output byte-compatible
with what `src/parsers/` expects (e.g. "no partition / no prediction" is
written as `15`, the value of the old HM `SIZE_NONE`/`MODE_NONE`).

### HM-18 decoder (`appgitlhevcdecoder/HM_18.0/`)

- HM is an **unmodified upstream submodule** (`HM_18.0/HM`, tag HM-18.0).
  The `.pro` files sit next to it and glob sources like HM's CMake.
- The analyzer hooks are in `HM_18.0/sysu-analyzer.patch`, guarded by
  `ENABLE_ANAYSIS_OUTPUT` (sic, defined in `hm.pri`). `hm.pri` applies the
  patch to the submodule working tree at qmake time (skipped if already
  applied; the submodule is `ignore = dirty`). The hooks call
  `HM_18.0/TLibSysuAnalyzer/TSysuAnalyzerOutput`, which writes all output files.
  - To change a hook: edit the files in `HM_18.0/HM`, then regenerate the
    patch with `git -C appgitlhevcdecoder/HM_18.0/HM diff > appgitlhevcdecoder/HM_18.0/sysu-analyzer.patch`.
  - To move to a newer HM tag: `git -C appgitlhevcdecoder/HM_18.0/HM checkout -- .`,
    switch the tag, re-run qmake, refresh the patch if it no longer applies.
- The legacy HM-4.0/5.2/10.0/12.0 decoders and the version picker were
  removed; the tag `legacy-decoders` is the last commit that has them.

### Filters (`libplugins/`)

Each filter implements `AbstractFilter` (`src/model/drawengine/abstractfilter.h`:
`drawFrame/drawCTU/drawCU/drawPU/drawTU/drawTile`, mouse/key handlers,
`config`). `FilterLoader` loads every library in `plugins/` with
`QPluginLoader`. Plugins compile the model sources they need directly (see
their `.pro` `SOURCES`) and share config widgets via `libplugins/filterconfiggui.pri`.
Building with `CONFIG+=EXTRA_PRIVATE_FILTER` also builds `libplugins/PrivateFilter`.

## Conventions and gotchas

- **Line endings are mixed.** Many files are CRLF (most GUI sources,
  `TLibSysuAnalyzer`); others, including the HM-18 submodule and its `.pro`
  files, are LF. Preserve each file's existing line endings when editing.
- **MSVC builds with Qt's `-Zc:strictStrings`.** Never initialise `char*` from a
  string literal in GUI/plugin/libgitlmvc code; use `const char*`. clang's
  "ISO C++11 does not allow conversion from string literal to 'char *'"
  warning flags the same thing locally. The HM-18 projects remove that flag
  (as HM's own CMake does) and must not add HM's `compat/msvc` include dir,
  whose `stdint.h` stub breaks current MSVC.
- HM-style naming (Hungarian prefixes: `m_`, `pc`, `i`, `ui`, `b`; HM typedefs
  `Int`, `UInt`, `Bool`, `Void`, `Pel`) is used throughout the decoders and
  much of the GUI; follow the surrounding code.
- Only 8-bit YUV 4:2:0 is supported by the GUI.
