# Implementation Plan: Build `wb_view` from Connectome Workbench on this Mac

## Goal

Build the **Connectome Workbench** (`wb_view`) GUI from this repository so it can be used for real work. The plan is written for a **junior execution agent** (DeepSeek V4 Flash): each step is small, explicit, copy-pasteable, and verifiable. Do not skip steps. Do not improvise. If a step fails, read the **What to do if it fails** note for that step before continuing.

---

## 0. Context the executing agent must know

- **Repo root:** `/Users/aghavamp/Desktop/Projects/workbench`
- **Source dir:** `/Users/aghavamp/Desktop/Projects/workbench/src`
- **Build dir (we will create):** `/Users/aghavamp/Desktop/Projects/workbench/build` (this is gitignored)
- **Target OS:** macOS 26.5.1, arm64 (Apple Silicon)
- **Compiler:** Apple clang 21.0.0 (via `/Library/Developer/CommandLineTools`)
- **Build system:** CMake (already installed via Homebrew, version 4.3.2)
- **The end product:** `build/Desktop/wb_view.app/Contents/MacOS/wb_view` (a macOS app bundle). A second useful binary, `build/CommandLine/wb_command`, is also built.
- **Uncommitted local change:** `src/Files/PaletteFile.cxx` has +570 lines adding extra default palettes (viridis, etc.). This is a *user* change and must **not** be reverted. It is self-contained, uses only existing helper functions (`addColor`, `addPaletteScalarAndColor`, `addPalette`, `addPaletteScalarAndColorFloat`), and closes cleanly before the existing `modifiedStatus` block. It should compile as-is.

### Dependency strategy (read before Step 1)

Workbench bundles most of its deps in-tree (`Qwt`, `Quazip`, `FtglFont`, `GLMath`, `kloewe`). External deps that must be provided by the system:

| Dependency        | How provided on this machine | Notes |
|-------------------|------------------------------|-------|
| Qt 5              | Homebrew `qt@5` 5.15.19 at `/opt/homebrew/opt/qt@5` | Required. Use `-DWORKBENCH_USE_QT5=TRUE`. |
| FreeType          | Homebrew `freetype` 2.14.3 at `/opt/homebrew/opt/freetype` | Workbench will build its **bundled FTGL** against this (good — Homebrew's standalone FTGL is unreliable). |
| OpenSSL           | Homebrew `openssl@3` at `/opt/homebrew/opt/openssl@3` | Needed for network/HTTP features. Pass `-DOPENSSL_ROOT_DIR`. |
| zlib              | macOS system / Xcode SDK        | CMake's `FindZLIB` will find it. No action needed. |
| OpenGL            | macOS system framework         | CMake finds automatically on Apple. |
| Qwt               | **Bundled** (`src/Qwt`)         | Do **not** install Homebrew qwt; let CMake use the bundled one. |
| QuaZip             | **Bundled** (`src/Quazip`)      | Let CMake use the bundled one. |
| FTGL              | **Bundled** (`src/FtglFont`)    | Built only because system FTGL is absent (good). |
| GLM (GLMath)       | **Bundled** (`src/GLMath`)      | pkg-config has no `glm`; falls back to bundled. |
| OpenMP             | **Absent** (no libomp installed) | CMake's `FindOpenMP` will fail → Workbench prints "OpenMP was not found" warning and disables it. This is **non-fatal**. `wb_view` works without OpenMP. Do not try to install libomp. |
| OSMesa             | **Absent**                     | Optional (only affects `wb_command -show-scene`). Leave unset. `OSMesaDummy` module is built instead. |
| Z5 / OME-Zarr      | **Off by default**             | Leave `-DWORKBENCH_USE_Z5=FALSE` (the default). |
| pkg-config         | **Absent**                     | CMake's `PKG_CHECK_MODULES` calls will silently no-op; bundled libs are used. This is fine. |

**Tool layout note:** Homebrew installs into `/opt/homebrew/opt/<name>`. Each formula has a stable symlink there, e.g. `/opt/homebrew/opt/qt@5/lib/cmake`. We pass these as `CMAKE_PREFIX_PATH` so CMake's find-modules resolve.

---

## Step 1 — Verify prerequisites are present

**Purpose:** Confirm the tools/dependencies we expect actually exist before doing anything else. Cheap, fast, saves debugging later.

**Commands to run (run each, read the output):**

```bash
cmake --version | head -1
clang++ --version | head -1
qmake --version
ls -d /opt/homebrew/opt/qt@5
ls /opt/homebrew/opt/qt@5/lib/cmake/Qt5
ls -d /opt/homebrew/opt/freetype
ls /opt/homebrew/opt/freetype/lib/libfreetype.dylib
ls -d /opt/homebrew/opt/openssl@3
ls /opt/homebrew/opt/openssl@3/lib/libssl.dylib
ls /opt/homebrew/include/freetype2
```

**Pass criteria:** All `ls` commands print a path (no "No such file or directory"). `qmake --version` reports "Qt version 5.15.x".

**What to do if it fails:**
- If `qmake` / Qt5 is missing: `brew install qt@5` (network required; if sandboxed, ask the user to run it, or proceed to Step 1b note below).
- If FreeType is missing: `brew install freetype`.
- If OpenSSL is missing: `brew install openssl@3`.
- If `cmake` is missing: `brew install cmake`.

> **Note on installing via Homebrew in this sandbox:** `brew install` needs network. If the sandbox blocks it, run the command with `required_permissions: ["full_network"]` on the Shell tool, or ask the user to run the install in a normal terminal. Do **not** try to build without Qt5/FreeType/OpenSSL.

---

## Step 2 — Clean any stale build directory

**Purpose:** Start from a known-clean state so an old failed CMake cache can't poison the configure.

**Commands:**

```bash
rm -rf /Users/aghavamp/Desktop/Projects/workbench/build
mkdir -p /Users/aghavamp/Desktop/Projects/workbench/build
```

**Pass criteria:** Command exits 0. `build/` exists and is empty.

**What to do if it fails:** Almost never fails. If `rm -rf` errors due to permissions, stop and ask the user.

---

## Step 3 — Configure with CMake

**Purpose:** Generate the Makefile build graph. This is the step most likely to surface missing-dependency errors.

**Run from inside the build dir:**

```bash
cd /Users/aghavamp/Desktop/Projects/workbench/build
cmake \
  -D CMAKE_BUILD_TYPE=Release \
  -D CMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@5;/opt/homebrew/opt/freetype;/opt/homebrew/opt/openssl@3 \
  -D WORKBENCH_USE_QT5=TRUE \
  -D WORKBENCH_USE_QT6=FALSE \
  -D WORKBENCH_USE_QT5_QOPENGL_WIDGET=TRUE \
  -D WORKBENCH_USE_Z5=FALSE \
  -D WORKBENCH_INCLUDE_HELP_HTML_RESOURCES=TRUE \
  ../src
```

**Notes on each flag (do not change these):**
- `CMAKE_BUILD_TYPE=Release` — optimized build (the README example uses Release).
- `CMAKE_PREFIX_PATH` — tells CMake where Qt5 / FreeType / OpenSSL CMake config packages live. Semicolon-separated, **no spaces**.
- `WORKBENCH_USE_QT5_QOPENGL_WIDGET=TRUE` — use `QOpenGLWidget` instead of deprecated `QGLWidget`. Avoids deprecation/link errors on modern macOS Qt5.
- `WORKBENCH_USE_Z5=FALSE` — skip the optional OME-Zarr stack (we don't have it; default is already FALSE, this just pins it).
- `../src` — the source tree (where the top-level `CMakeLists.txt` lives).

**Pass criteria:** CMake exits 0 and the summary at the end shows:
- `Qt: 5.15.x`
- `FreeType: 2.14.x …`
- `QuaZip: In Workbench Source Code`
- `Qwt: In Workbench Source Code`
- `GLMath: In Workbench Source Code`
- `OSMESA: Not found`  ← OK to ignore
- `OpenMP: Not Found`  ← OK to ignore
- `FTGL: In Workbench Source Code`  ← important: confirms bundled FTGL will be built

And a `Makefile` now exists at `build/Makefile`.

**What to do if it fails:**
- **"Qt5 or Qt6 REQUIRED" / `Qt5 not found`:** Verify `CMAKE_PREFIX_PATH` includes `/opt/homebrew/opt/qt@5` and that `/opt/homebrew/opt/qt@5/lib/cmake/Qt5` exists. Re-run Step 2 then Step 3.
- **OpenSSL NOT FOUND but continues:** This is a warning, not fatal. But if you also see a *later* link error involving `ssl`, ensure `CMAKE_PREFIX_PATH` includes `/opt/homebrew/opt/openssl@3`.
- **FreeType library NOT found:** Verify `/opt/homebrew/opt/freetype` is in `CMAKE_PREFIX_PATH` and `/opt/homebrew/opt/freetype/lib/libfreetype.dylib` exists (Step 1).
- **Any other FATAL_ERROR:** Read the message. It usually names the missing package. Do not continue to Step 4 until CMake exits 0.

---

## Step 4 — Compile `wb_view` (and the rest) with `make`

**Purpose:** Build the binaries. This is the longest step (~5–25 min on this machine).

**Pick a parallelism value.** Apple Silicon Macs here have many cores. Use 8 as a safe default (the README example uses 8). If you can determine the CPU count (e.g. `sysctl -n hw.logicalcpu`), you may use that; otherwise 8 is fine.

**Command:**

```bash
cd /Users/aghavamp/Desktop/Projects/workbench/build
nice make -j8 wb_view
```

> Why the target `wb_view`? Building just the `wb_view` target (and its dependencies) skips the test driver and `wb_command` if you only need the GUI, which is faster. If you want both, run `nice make -j8` with no target. Building all targets is also fine.

**Pass criteria:** `make` exits 0 and produces:
- `build/Desktop/wb_view.app/Contents/MacOS/wb_view` (the GUI executable)
- (if you built all targets) `build/CommandLine/wb_command`

Verify with:

```bash
ls -l /Users/aghavamp/Desktop/Projects/workbench/build/Desktop/wb_view.app/Contents/MacOS/wb_view
file /Users/aghavamp/Desktop/Projects/workbench/build/Desktop/wb_view.app/Contents/MacOS/wb_view
```

The `file` output should say `Mach-O 64-bit executable arm64`.

**What to do if it fails:**
- **A single source file fails to compile:** Read the compiler error. If it's in `src/Files/PaletteFile.cxx` (the locally modified file), inspect the error line; the diff adds palettes using existing helpers, so a compile error here likely means a typo was introduced — compare against the surrounding existing palette blocks for the correct pattern. **Do not revert the whole diff** unless the user confirms; try to fix the specific line.
- **Link error about a missing Qt symbol / `QGLWidget`:** Make sure Step 3 set `WORKBENCH_USE_QT5_QOPENGL_WIDGET=TRUE`. If you forgot it, redo Step 2 + Step 3.
- **"ld: warning ... link to dylib which was built for macOS X but is being linked for a different platform":** Usually harmless on arm64. Continue.
- **Link error `library not found for -lomp` / OpenMP:** CMake should have disabled OpenMP, but if it half-detected it, set these in Step 3 and reconfigure:
  ```
  -D WORKBENCH_OPENMP_COMPILER_FLAGS="" \
  -D WORKBENCH_OPENMP_INCLUDE_DIR="" \
  -D WORKBENCH_OPENMP_LIBRARY="" \
  ```
  Then `make` again. (You generally don't need to fully `rm -rf build`, but if the error persists, do Step 2 → Step 3 fresh.)
- **Out of memory / killed:** Lower parallelism to `-j4` or `-j2` and retry. `ccache` is not installed; large C++ files can be memory-heavy under `-j8`.
- **If a compile error is in a third-party bundled module (Qwt/Quazip/FtglFont/CZIlib):** These are stable upstream code; an error usually means a flag from Step 3 was wrong. Re-check Step 3 rather than patching bundled code.

---

## Step 5 — Smoke-test `wb_view`

**Purpose:** Confirm the binary actually launches (not just that it links).

**Command (non-graphical sanity check — prints help and exits):**

```bash
/Users/aghavamp/Desktop/Projects/workbench/build/Desktop/wb_view.app/Contents/MacOS/wb_view -help
```

**Pass criteria:** The command prints the usage text beginning with `Usage: … [options] [files]` and a list of options (`-help`, `-enable-perf`, `-graphics-size`, `-logging`, `-mac-menu-in-window`, etc.), then exits 0. This proves the binary is loadable and core initialization (Qt, args parsing, palette loading) works — including the locally-added palettes in `PaletteFile.cxx`, which are loaded at startup.

**What to do if it fails:**
- **`dyld: Library not loaded: ... Qt ... .framework`:** The Qt dylibs need to be found at runtime. Two fixes:
  1. Quick test: `DYLD_FRAMEWORK_PATH=/opt/homebrew/opt/qt@5/lib /Users/aghavamp/Desktop/Projects/workbench/build/Desktop/wb_view.app/Contents/MacOS/wb_view -help`
  2. Proper fix: run `macdeployqt` (see Step 6) to bundle the Qt libs into the app, after which it launches standalone.
- **`zsh: killed` / immediate crash with no message:** Possibly code-signing on arm64. Try:
  ```bash
  codesign --force --deep --sign - /Users/aghavamp/Desktop/Projects/workbench/build/Desktop/wb_view.app
  ```
  then retry. If it still crashes, run under the debugger for a backtrace:
  ```bash
  /usr/bin/lldb -o run -o bt -o quit -- /Users/aghavamp/Desktop/Projects/workbench/build/Desktop/wb_view.app/Contents/MacOS/wb_view -help
  ```
  and read where it dies before asking the user.
- **Segmentation fault inside palette loading:** Likely the local `PaletteFile.cxx` diff has a logic error (e.g., a palette referenced by name that wasn't registered). Inspect the diff around the crash and fix the specific entry; don't revert wholesale.

---

## Step 6 — (Optional but recommended) Bundle Qt frameworks with `macdeployqt`

**Purpose:** Make `wb_view.app` double-clickable from Finder without relying on Homebrew's dylibs being on the runtime path. Do this only after Step 5 passes.

**Command:**

```bash
/opt/homebrew/opt/qt@5/bin/macdeployqt /Users/aghavamp/Desktop/Projects/workbench/build/Desktop/wb_view.app
```

**Pass criteria:** exits 0; the app bundle now contains its Qt frameworks under `wb_view.app/Contents/Frameworks/`:

```bash
ls /Users/aghavamp/Desktop/Projects/workbench/build/Desktop/wb_view.app/Contents/Frameworks | head
```

You should see `QtCore.framework`, `QtGui.framework`, `QtWidgets.framework`, etc.

Then re-test launching (Step 5's `-help` command) — it should now work **without** setting `DYLD_FRAMEWORK_PATH`.

**What to do if it fails:**
- `macdeployqt` complains about codesigning: re-run the ad-hoc sign from Step 5:
  ```bash
  codesign --force --deep --sign - /Users/aghavamp/Desktop/Projects/workbench/build/Desktop/wb_view.app
  ```
- If `macdeployqt` reports missing plugins, that's usually non-fatal; the app generally still launches. Proceed to Step 7.

---

## Step 7 — Final verification: launch the GUI

**Purpose:** Confirm the full GUI comes up (the real end goal).

**Command:**

```bash
open /Users/aghavamp/Desktop/Projects/workbench/build/Desktop/wb_view.app
```

**Pass criteria:** The `wb_view` window opens (the Connectome Workbench main window with the toolbar and tabbed view). This requires a graphical session; in a headless/SSH context, skip this step and rely on Step 5. If the window appears, **the goal is met**: `wb_view` is built and usable.

**What to do if it fails:**
- Bounces in the Dock and quits: check Console.app for the crash, or run the executable directly (not via `open`) from a terminal so stderr is visible:
  ```bash
  /Users/aghavamp/Desktop/Projects/workbench/build/Desktop/wb_view.app/Contents/MacOS/wb_view
  ```
  and read the error. Most commonly this is a missing Qt plugin path — fixed by Step 6. If Step 6 wasn't done, do it now.

---

## Step 8 — (Optional) Install system-wide

Only do this if the user wants `wb_view`, `wb_command`, `wb_shortcuts`, and bash completion installed to standard paths (default `/usr/local/bin` on macOS). **Ask the user before running** because it writes outside the repo.

```bash
cd /Users/aghavamp/Desktop/Projects/workbench/build
make install
```

(Note: `make install` for an app bundle copies `wb_view.app` to `${CMAKE_INSTALL_PREFIX}/bin` or similar; this may need `sudo` depending on the install prefix.)

---

## Recap of the critical path

1. Verify deps (Step 1) →
2. Clean `build/` (Step 2) →
3. `cmake` configure (Step 3) →
4. `make -j8 wb_view` (Step 4) →
5. `wb_view -help` smoke test (Step 5) →
6. `macdeployqt` (Step 6, optional) →
7. `open wb_view.app` (Step 7).

If Steps 1–5 all pass, the build is functionally complete and `wb_view` can be used for work. Steps 6–7 just make it nicer to launch.

---

## Things this plan deliberately does NOT do

- **Does not** revert or modify the local `src/Files/PaletteFile.cxx` change. It is a user change and is expected to compile.
- **Does not** install OpenMP / libomp. `wb_view` works without it; CMake will warn but proceed.
- **Does not** install OSMesa. Only needed for `wb_command -show-scene` offscreen rendering, not for `wb_view` GUI work.
- **Does not** enable Z5 / OME-Zarr. Off by default and not installed here.
- **Does not** build with Qt6. Qt5 is what's installed and what the README recommends.
- **Does not** patch any bundled third-party code (Qwt/Quazip/FTGL/CZIlib). If a build error appears there, it indicates a misconfiguration in Step 3, not a code bug.

---

## One-line quick path (for when Steps 1–2 are already done)

```bash
cd /Users/aghavamp/Desktop/Projects/workbench/build
cmake -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@5;/opt/homebrew/opt/freetype;/opt/homebrew/opt/openssl@3 \
  -DWORKBENCH_USE_QT5=TRUE -DWORKBENCH_USE_QT6=FALSE \
  -DWORKBENCH_USE_QT5_QOPENGL_WIDGET=TRUE -DWORKBENCH_USE_Z5=FALSE \
  -DWORKBENCH_INCLUDE_HELP_HTML_RESOURCES=TRUE \
  ../src \
&& nice make -j8 wb_view \
&& ./Desktop/wb_view.app/Contents/MacOS/wb_view -help
```

If that whole chain succeeds, you're done.
