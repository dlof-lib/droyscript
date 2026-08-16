# droy script x

A small data/graph description language — `group`, `row`, `collection`,
`equals`, `number`, `bridge`, `id`, `name`, `edge`, `map` — with a C++17
engine and an Android app to write and run it on-device.

See **[SPEC.md](SPEC.md)** for the full language spec, sigils, and grammar.

## Layout

```
SPEC.md                     language specification + worked example
engine/                      standalone C++17 engine (lexer, parser, interpreter)
  include/droy/               model.hpp, lexer.hpp, interpreter.hpp
  src/                         lexer.cpp, interpreter.cpp, main.cpp (CLI)
  examples/sample.droy         reference program
android/                     Android app (Kotlin + Jetpack views, NDK/JNI)
  app/src/main/java/...         MainActivity.kt, engine/DroyEngine.kt
  app/src/main/cpp/             JNI bridge + a copy of the engine sources
.github/workflows/build-apk.yml   CI: builds the debug APK and sanity-checks the engine
```

## Try the engine standalone (no Android needed)

```bash
g++ -std=c++17 -Iengine/include -o droyc \
    engine/src/lexer.cpp engine/src/interpreter.cpp engine/src/main.cpp
./droyc engine/examples/sample.droy
```

This has already been compiled and run as part of building this project —
the sample program correctly filters `Users` into `ActiveUsers` by
`number=100`, builds the `Friendship` bridge's edges, prints the `Settings`
map, and computes `~len` / `~sum`.

## The Android app

`com.dlof.droyscript` — a single-screen editor (top: multiline code field,
bottom: scrollable output) with **Run** and **Load sample** buttons. Code
runs entirely on-device: `MainActivity` calls `DroyEngine.run(source)`,
which calls into `libdroyengine.so` (built from the same `engine/` sources
via CMake/NDK) and returns the `~print` / `~len` / `~sum` output as text.

Open `android/` in Android Studio (Hedgehog+, with an NDK/CMake install) and
run it, or let CI build it for you — see below.

## CI: GitHub Actions APK build

`.github/workflows/build-apk.yml` has two jobs:

1. **build** — installs JDK 17, the Android SDK, NDK `26.1.10909125` and
   CMake, then runs `gradle assembleDebug` in `android/` and uploads the
   resulting debug APK as a workflow artifact.
2. **build-engine-only** — compiles and runs the pure C++ engine as a fast
   sanity check that doesn't need the Android toolchain.

The workflow uses `gradle/actions/setup-gradle` rather than a committed
`gradlew` binary, so no binary wrapper jar needs to be checked into the
repo — Gradle 8.7 is fetched at CI time (see
`android/gradle/wrapper/gradle-wrapper.properties` for the version used by
Android Studio locally).

## Extending the engine (LLVM backend)

The interpreter in `engine/src/interpreter.cpp` is a tree-walking evaluator
by design (fast startup for on-device use, zero external deps). If you want
an LLVM backend later: keep `Lexer` + the AST-shaped parsing in `Parser` as
they are, and add a `codegen.cpp` that walks the same grammar and emits
LLVM IR via `llvm::IRBuilder` for `group`/`row` as struct literals, `bridge`
edges as a small adjacency-list global, and `map` as a lookup table —
built-ins (`~print`, `~len`, `~sum`) become calls into a small C runtime
linked alongside the generated module.
