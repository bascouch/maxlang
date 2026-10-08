# maxlang

![Texte alternatif](maxlang.doc/doc-img/lfo-3.png)

A small text language and a set of Max/MSP externals for describing **modulators**
(LFOs, ramps, random generators, sequencers, envelopes…) and driving module
parameters with them from Max.



```
lfo(freq=6 min=0 max=100 mode=1)
lfo(freq=seq(list=[1 16 1 32] freq=1) min=40 max=70 mode=2)
choice(freq=10 list=[10 15 50 90] seed=DUF)
```

Modulators can be nested (any parameter can itself be a modulator), named, and
addressed later, e.g. `modparam mymodtor.freq 0.3`.

## Contents

| Path | Description |
| --- | --- |
| `maxlang/` | Core library (header-only C++): PEGTL grammar, parse / macro / modulator trees, utilities. Bundles `maxcpp` and `tao/pegtl` in `include/`. |
| `maxlang.modulator/` | `maxlang.modulator` external — control-rate (message) modulator. |
| `maxlang.modulator~/` | `maxlang.modulator~` external — signal-rate, multichannel modulator. |
| `bufgranulp~/` | `bufgranulp~` — multi-buffer granular synthesis external (GMEM, 2002–2004), with maxlang modulator support. |
| `maxlang.doc/` | Modulator reference documentation (`maxlang_modulators_doc.md` / `.html`). |
| `max-files/` | Max patches and JS: `maxlang_core` (parser, scenes) and `maxlang_tests`. |
| `maxlang/tests/` | Standalone grammar test programs. |
| `maxlang/python-tools/` | Notebooks used to design curves (e.g. S-curve formula). |
| `pack/` | Packaged builds. |

## Available modulators

`line`, `lfo`, `rand`, `randi`, `choice`, `choicei`, `seq`, `seqi`, `env`,
`xfade`, `input` — plus parameters common to all (`name`, `sync`, `seed`).

See [`maxlang.doc/maxlang_modulators_doc.md`](maxlang.doc/maxlang_modulators_doc.md)
for every parameter, its unit, default and range.

## Building

These projects are built as part of the
[Max SDK](https://github.com/Cycling74/max-sdk). This repository is expected to
live at `max-sdk/source/cb/`, since the CMake files include
`../../max-sdk-base/script/max-pretarget.cmake`.

```bash
cd max-sdk
mkdir -p build && cd build
cmake -G Xcode ..
cmake --build . --config Release
```

Requirements: CMake ≥ 3.19, a C++17 compiler (Xcode on macOS). On macOS the
externals are built as universal binaries (`x86_64;arm64`).

After a build, the `.mxo` bundle produced in `max-sdk/externals/` is copied next
to its source folder (post-build step in each `CMakeLists.txt`).

## Usage

Open the help patchers in Max:

- `maxlang.modulator/maxlang.modulator.maxhelp`
- `maxlang.modulator~/maxlang.modulator~.maxhelp`
- `bufgranulp~/bufgranulp~.maxhelp`

## Author

Charles Bascou — `bufgranulp~` originally by Laurent Pottier, Loïc Kessous,
Charles Bascou and Léopold Frey (GMEM).
