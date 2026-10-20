# OpenEnroth MM6 for PS5

A native PlayStation 5 homebrew port of **Might and Magic VI: The Mandate of Heaven**, running on
[OpenEnroth](https://github.com/OpenEnroth/OpenEnroth), the open-source reimplementation of the Might and Magic
VI–VIII engine. It installs as its own home-screen title on a jailbroken PS5: native x86-64 code, OpenGL 4.6 on the
console's GPU, DualSense input and console audio. No emulation or Wine.

Upstream OpenEnroth plays only MM7 so far. MM6 comes from xygtshadow's
[`mm6-support` branch](https://github.com/xygtshadow/OpenEnroth/tree/mm6-support), which this port builds on. The
Might and Magic VII port lives in its own repository (`mm7-ps5`) and shares the PS5 work: the platform layer, the
controller support and the build.

> **Status: early.** The MM6 branch passes its 156 scripted MM6 game tests on the Steam release's data, with the
> PS5 patches applied. Nobody has played MM6 through on it yet, so expect bugs, especially in dungeons and combat.

Bring your own game files: nothing from Might and Magic is included.

## Installing

See [docs/INSTALL.md](docs/INSTALL.md). In short: a PS5 on firmware 13.60 or lower with kstuff and ShadowMountPlus,
your own Might and Magic VI (GOG or Steam), and `python3 install.py --ps5 <ip> --mm6 <game folder>` from a release.

## Building

Builds on x86-64 Linux. You need:

- **LLVM 21** clang, lld and llvm (the toolchain the PS5 OpenGL driver was validated with).
  Arch: `pacman -S clang21 lld21 llvm21`. Debian/Ubuntu: `clang-21 lld-21 llvm-21` from apt.llvm.org, and set
  `LLVM21=/usr/lib/llvm-21` in `config.local.sh`.
- git, curl, wget, unzip, make, cmake (3.27+), ninja, python3 with venv, gcc, bison, flex,
  glslangValidator, spirv-tools.
- About 15 GB of disk space.

```sh
cp config.example.sh config.local.sh   # set PS5_HOST and MM6_DATA
scripts/setup.sh                       # fetch, verify and build the dependencies into deps/ (~10 minutes)
scripts/build.sh                       # build OpenEnroth and package build/title/dist/PPSA99966
scripts/package.sh deploy-data         # once: upload your MM6 files to the console
scripts/build.sh deploy                # build and upload the program
scripts/build.sh release               # build a release zip in build/release
```

### Working on OpenEnroth

`scripts/setup.sh` checks out the MM6 branch at a pinned commit in `deps/openenroth` and applies
`patches/openenroth` on a `ps5` branch. Make changes there, commit them to that branch, then run
`scripts/export-patches.sh` and commit the updated patches here. A desktop build of the same tree
(`-DOE_USE_SDL2=ON -DOE_GL33=ON`) is a quick way to test changes before going to the console, and the branch's
MM6 game tests run against your own data:

```sh
OpenEnroth_GameTest --test-path <any folder> --game-version mm6 --data-path <MM6 folder> --headless --gtest_filter='Mm6.*'
```

### Debugging on the console

Native titles have no console output, so stdout and stderr go to the kernel log:

```sh
scripts/send-payload.sh klogsrv   # once per console restart: kernel log server on port 3232
scripts/klog.sh 600               # record 10 minutes of kernel log into logs/, then launch the title
```

A crash shows up there as "A user thread receives a fatal signal" with a backtrace. The title is loaded at
`0x400000`; subtract that and look the addresses up with
`$LLVM21/bin/llvm-symbolizer --obj=build/title/build/llvm-pie.elf <address>`.

## How it works

- **OpenEnroth's MM6 branch** is patched (`patches/openenroth`): fixes for newer compilers and FFmpeg 7, an
  **SDL2 platform backend** (the PS5 homebrew SDL is 2.30, OpenEnroth uses SDL3), an **OpenGL 3.3 core** mode (what
  the driver's SDL2 bridge accepts), a **`ps5` build platform**, presenting frames with a draw (the driver blits
  scaled frames on the CPU), and the controller layer: the **touchpad as the mouse**, **D-pad menu navigation**,
  smooth looking, and saving without a keyboard.
- **SDL2** is the PS5 homebrew fork, built by ps5-opengl with its PS5 patches, plus `patches/ps5-sdl` (touchpad
  input).
- **Graphics** come from [ps5-opengl](https://github.com/mihawk-99/ps5-opengl) v1.0.0, a static Mesa/Gallium
  OpenGL 4.6 driver for the console's GPU. Native titles can't load shared libraries, so it is linked in.
- **The title** is built with the PS5 payload SDK and the native app template's runtime, linker conversion and
  signing (`scripts/package.sh`).
- **`platform/`** fills the gaps between those libraries and what the console's libc exports to a native title:
  directory listing, locale functions, console output, `exit`, random numbers, and so on. Several of these come
  from the [OpenRCT2 PS5 port](https://github.com/icepuente/rct2-ps5).
- **LuaJIT** is built for LuaJIT's own PS5 target (interpreter only), with the settings OpenEnroth's scripts need.

## Layout

| | |
| --- | --- |
| `scripts/` | setup, build, package, install, debugging |
| `toolchain/` | CMake toolchain file and compiler wrappers for the PS5 |
| `platform/` | runtime glue linked into the title |
| `patches/` | changes to OpenEnroth, ps5-opengl and SDL2 |
| `title/` | home-screen art, turned into the icon and backgrounds by `scripts/make-title-art.py` |
| `docs/` | install guide |
| `deps/`, `build/`, `logs/` | created by the scripts, not committed |

## License

GPL-3.0-or-later, see [LICENSE](LICENSE). Third-party components and their licenses are listed in
[THIRD_PARTY.md](THIRD_PARTY.md).

Might and Magic is a trademark of Ubisoft. This project is not affiliated with or endorsed by Ubisoft, New World
Computing or Sony Interactive Entertainment.
