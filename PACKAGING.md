# Packaging

This document explains the packaging infrastructure for libfacts and the rationale behind each choice.

## Two libraries

Facts ships two libraries from the same header (`facts.h`):

- **libfacts** — built from `src/facts.c`. Pure C, no C++ runtime dependency. Link with `-lfacts`.
- **libfacts++** — built from `src/facts.cpp` (which `#include "facts.c"` internally). Contains the full C core plus C++ extras (FactsTrace, FACTS_WATCH). Link with `-lfacts++`.

Users link one or the other, never both. The C++ library is self-contained — it does not depend on the C library.

## Build systems

### Makefile (primary, Unix)

The `Makefile` is the primary build system for Unix-like platforms (Linux, macOS, BSD).

| Target | What it builds | Notes |
|--------|---------------|-------|
| `make` / `make all` | `libfacts.a`, `libfacts++.a` + shared libraries | Default — libs only |
| `make install` | Installs libs, header, pkg-config | Respects `DESTDIR`, `PREFIX`, `LIBDIR`, `INCLUDEDIR` |
| `make check` | Builds tests + examples, runs diffs | Full test suite |
| `make expected` | Refreshes golden test output | After intentional output changes |
| `make tests` | Builds test binaries only | Without running them |
| `make examples` | Builds example programs only | |
| `make cosmo` | Builds `libfacts.a` with Cosmopolitan | C library only |
| `make cosmo-check` | Builds and runs tests with Cosmopolitan | |
| `make uninstall` | Removes installed files | |
| `make clean` | Removes all build artifacts | |

All install targets respect the standard GNU variables (`DESTDIR`, `PREFIX`, `LIBDIR`, `INCLUDEDIR`) that distro packagers rely on. This is what makes `dpkg-buildpackage`, `abuild`, and Homebrew formulas work without patches.

**Why a Makefile and not just CMake?** For a library this small, a Makefile is the simplest correct solution. Unix distro packagers (Debian, Alpine, Homebrew) are all comfortable with Makefiles. CMake adds a configure step and a build directory that is unnecessary overhead for the common case.

### CMakeLists.txt (alternative, cross-platform)

CMake is provided as an alternative, primarily for:

- **Windows** — where `make` and the POSIX toolchain are not standard
- **vcpkg and Conan** — the two largest C/C++ package managers, which strongly prefer or require CMake
- **IDE integration** — Visual Studio, CLion, and VS Code all have native CMake support

The CMake build produces the same artifacts as the Makefile, plus:

- CMake package config files (`factsConfig.cmake`, `factsTargets.cmake`) enabling `find_package(facts)` in downstream projects
- pkg-config `.pc` files (same as the Makefile produces)

Tests are opt-in via `-DFACTS_BUILD_TESTS=ON`. The default CMake build only produces the libraries.

**Why not CMake only?** CMake is the right choice for complex projects with many dependencies and platform-specific logic. For a library this small, it adds complexity (out-of-source builds, generator selection, cache variables) that the Makefile avoids. Both are maintained because they serve different audiences.

## Package manager support

### Debian / Ubuntu (`debian/`)

**Location:** `debian/`

**Why:** Debian packaging is the foundation for Ubuntu, Mint, Pop!_OS, and most cloud server distributions. The `debian/` directory contains everything `dpkg-buildpackage` needs to produce `.deb` files.

**Produces two packages:**

- `libfacts-1` — shared libraries (libfacts.so and libfacts++.so)
- `libfacts-dev` — headers, static libraries, pkg-config files

**How to build:**

```sh
dpkg-buildpackage -us -uc
```

**Key files:**

| File | Purpose |
|------|---------|
| `debian/control` | Package metadata, dependencies, descriptions |
| `debian/rules` | Build script (delegates to `make`) |
| `debian/changelog` | Version history (required by dpkg) |
| `debian/copyright` | License in Debian machine-readable format |
| `debian/source/format` | Declares `3.0 (native)` source format |
| `debian/libfacts-1.install` | Files that go into the runtime package |
| `debian/libfacts-dev.install` | Files that go into the dev package |

### Homebrew (macOS / Linux)

**Location:** `packaging/homebrew/facts.rb`

**Why:** Homebrew is the dominant package manager on macOS and is widely used on Linux. A formula file is all that is needed.

**How to use:**

The formula can be hosted in a custom tap (a separate Git repository):

```sh
# Create a tap repository (one-time)
# Add packaging/homebrew/facts.rb to it as Formula/facts.rb

brew tap wmacevoy/facts https://github.com/wmacevoy/homebrew-facts
brew install wmacevoy/facts/facts
```

Or install directly from the formula for testing:

```sh
brew install --formula packaging/homebrew/facts.rb
```

**Note:** The formula uses CMake because Homebrew's `std_cmake_args` helper handles prefix, RPATH, and bottle creation automatically. The `sha256` field must be updated after creating a GitHub release tarball.

### Alpine Linux

**Location:** `packaging/alpine/APKBUILD`

**Why:** Alpine is the base image for most Docker containers. Its minimal footprint and musl libc make it the default for containerized deployments. An `APKBUILD` is Alpine's equivalent of a Debian `debian/rules`.

**How to build:**

```sh
# Copy APKBUILD to an abuild workspace
abuild -r
```

**Note:** The `sha512sums` field must be updated after creating a GitHub release tarball.

### Chocolatey (Windows)

**Location:** `packaging/choco/`

**Why:** Chocolatey is the most established package manager for Windows. It enables `choco install libfacts` on Windows machines.

**How to build:**

```powershell
cd packaging\choco
choco pack
choco install libfacts --source .
```

**Requires:** CMake and a C/C++ compiler (MSVC or MinGW) installed on the build machine. The install script downloads the source, builds with CMake, and installs to the Chocolatey lib directory.

### Cosmopolitan Libc (any platform)

**Why:** [Cosmopolitan Libc](https://github.com/jart/cosmopolitan) produces Actually Portable Executables (APE) that run natively on Linux, macOS, Windows, FreeBSD, OpenBSD, and NetBSD from a single binary. The C library (`libfacts`) works with Cosmopolitan without any code changes. The C++ library (`libfacts++`) is not supported since `cosmocc` is C-only.

Cosmopolitan is static-only — there is no shared library support.

**How to build:**

```sh
make cosmo          # build libfacts.a with cosmocc
make cosmo-check    # build and run tests as a portable executable
```

This requires `cosmocc` and `cosmoar` on your `PATH`. Install from https://cosmo.zip/ or build from source.

**Linking against the library:**

```sh
cosmocc -Iinclude -o check your.c libfacts.a
```

The resulting `check` binary runs on Linux, macOS, Windows, FreeBSD, OpenBSD, and NetBSD without recompilation.

## VSCode extension

**Location:** `vscode-facts/`

**Why:** The extension integrates Facts into VSCode's Test Explorer, providing test discovery, run/debug, and automatic `FACTS_WATCH` breakpoints — all without leaving the editor.

**Features:**

- Scans source files for `FACTS(Name)` declarations and shows them in the Test Explorer sidebar
- Builds and runs individual or all tests with one click
- Launches the debugger with `--facts_include=Name` for targeted debugging
- Auto-sets conditional GDB breakpoints from `FACTS_WATCH(var, ...)` annotations
- Re-discovers tests when source files change

**Building locally in a container:**

```sh
# Using the project's Dockerfile (includes Node 20 + build-essential)
docker build -t facts-dev .
docker run --rm --entrypoint /bin/bash \
  -v $PWD:/app -w /app/vscode-facts facts-dev \
  -c "npm install && npm run compile && npx @vscode/vsce package --allow-missing-repository"
```

This produces `vscode-facts/facts-test-runner-X.Y.Z.vsix`.

**Building in the devcontainer (VSCode):**

1. Open the project in VSCode
2. Reopen in Container (Ctrl+Shift+P → "Dev Containers: Reopen in Container")
3. The `postCreateCommand` runs `npm install && npm run compile` automatically
4. To package: `cd vscode-facts && npm run package`

**Installing the extension:**

In VSCode: Extensions → `...` → Install from VSIX → select the `.vsix` file.

Or from the command line:

```sh
code --install-extension vscode-facts/facts-test-runner-0.1.0.vsix
```

**CI:** The `package-vscode` job in `.github/workflows/c-cpp.yml` builds and uploads the `.vsix` as a GitHub Actions artifact on every push to `main`.

## Package managers NOT included (and why)

| Manager | Why not | What to do |
|---------|---------|------------|
| **vcpkg** | Port files live in the [vcpkg registry](https://github.com/microsoft/vcpkg), not upstream repos | Submit a port once the library is published; the `CMakeLists.txt` and `find_package` support is all vcpkg needs from upstream |
| **Conan** | Recipes live in [conan-center-index](https://github.com/conan-io/conan-center-index), not upstream repos | Submit a recipe; CMake support is sufficient |
| **RPM / Fedora** | `.spec` files are similar to Debian packaging; the Makefile `install` target provides everything a spec file needs | A packager can write the spec against the existing `make install` interface |
| **pkgsrc (BSD)** | Same as RPM — the Makefile `install` target is the interface | BSD porters write the Makefile fragment against `make install` |
| **Meson** | Solves the same problem as CMake with less Windows ecosystem support | Not worth maintaining a third build system |

## pkg-config

Both build systems generate `facts.pc` and `facts++.pc` from their `.in` templates:

```sh
# C only
cc $(pkg-config --cflags facts) -o check your.c $(pkg-config --libs facts)

# C++
c++ $(pkg-config --cflags facts++) -o check your.cpp $(pkg-config --libs facts++)
```

## Versioning

The version (`1.3.0`) is defined in two places:

- `Makefile` — `VERSION_MAJOR`, `VERSION_MINOR`, `VERSION_PATCH`
- `CMakeLists.txt` — `project(facts VERSION 1.3.0)`

Both must be updated together when releasing a new version. The shared library soname uses only the major version (`libfacts.so.1`), so minor and patch releases are ABI-compatible drop-in replacements.

## CI/CD — GitHub Actions

### CI workflow (`.github/workflows/c-cpp.yml`)

Runs on every push to `main` and on pull requests:

- **Ubuntu + macOS**: `make` (libs only), `make check` (tests), `make install` (verify layout)
- **Windows**: CMake build + `ctest`
- **Debian**: `dpkg-buildpackage` in a Debian container, verify `.deb` contents
- **Alpine**: CMake build in an Alpine container, verify installed files
- **Homebrew**: CMake install on macOS + link tests for both C and C++
- **Chocolatey**: CMake build on Windows + verify installed files
- **VSCode extension**: `npm install`, compile TypeScript, package `.vsix`, upload as artifact

### Release workflow (`.github/workflows/release.yml`)

Pushing a version tag (`v*`) triggers the release workflow, which:

1. **Tests** — runs `make check` on Ubuntu and macOS
2. **Builds .deb** — `dpkg-buildpackage` in a Debian container
3. **Builds Alpine tarball** — CMake build in an Alpine container
4. **Builds Windows** — CMake + MSVC on `windows-latest`
5. **Creates GitHub Release** — uploads all artifacts with install instructions and the source tarball `sha256`
6. **Publishes apt repo** — (optional) deploys a signed apt repository to GitHub Pages via `reprepro`

### apt repo via GitHub Pages (optional setup)

The `apt-repo` job is gated behind the repository variable `ENABLE_APT_REPO=true`. When enabled, it publishes a signed Debian repository to GitHub Pages so users can install with:

```sh
# Download the signing key
curl -fsSL https://wmacevoy.github.io/facts/facts.gpg \
  | sudo tee /usr/share/keyrings/facts.gpg > /dev/null

# Add the repository
echo "deb [signed-by=/usr/share/keyrings/facts.gpg] \
  https://wmacevoy.github.io/facts stable main" \
  | sudo tee /etc/apt/sources.list.d/facts.list

sudo apt update && sudo apt install libfacts-dev
```

**One-time setup:**

1. Generate a GPG signing key:

   ```sh
   gpg --batch --gen-key <<EOF
   %no-protection
   Key-Type: RSA
   Key-Length: 4096
   Name-Real: libfacts repo signing key
   Name-Email: wmacevoy@gmail.com
   Expire-Date: 0
   EOF
   ```

2. Export the private key and add it as the repository secret `GPG_PRIVATE_KEY`:

   ```sh
   gpg --armor --export-secret-keys
   # Copy output -> GitHub repo Settings -> Secrets -> GPG_PRIVATE_KEY
   ```

3. Export the public key and commit it to the repository so users can download it:

   ```sh
   gpg --armor --export > apt/facts.gpg
   ```

4. Enable GitHub Pages: Settings -> Pages -> Source: **GitHub Actions**

5. Set the repository variable `ENABLE_APT_REPO` to `true`: Settings -> Variables -> Actions -> New variable

### Homebrew tap

The Homebrew formula lives in `packaging/homebrew/facts.rb`. To serve it to users:

1. Create a separate repository: `wmacevoy/homebrew-facts`
2. Copy `packaging/homebrew/facts.rb` to `Formula/facts.rb` in that repo
3. After each release, update the `sha256` in the formula with the value printed in the GitHub Release notes

Users then install with:

```sh
brew tap wmacevoy/facts
brew install facts
```

## Release checklist

1. Update version in `Makefile`, `CMakeLists.txt`, and `debian/changelog`
2. Commit, tag, and push:
   ```sh
   git tag v1.3.0
   git push origin v1.3.0
   ```
3. The release workflow runs automatically and creates the GitHub Release
4. Update the Homebrew formula `sha256` with the value from the release notes
5. Update the APKBUILD `sha512sums` with the tarball hash
6. (If using apt repo) Verify the Pages deployment at `https://wmacevoy.github.io/facts`
