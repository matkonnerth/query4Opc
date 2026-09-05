# AGENTS.md — AI Coding Agent Instructions

Purpose
- Provide concise, actionable guidance for AI coding agents working in this repository.

Quick Links
- [README.md](README.md) — project goals and query language overview
- [Implementation.md](Implementation.md) — deeper design notes for graph traversal and matching
- [CMakeLists.txt](CMakeLists.txt) — build entry point and target layout
- [conanfile.txt](conanfile.txt) — Conan dependencies and versions
- [src/](src/) — main implementation
- [tests/](tests/) — automated tests registered through CTest
- [benchmark/](benchmark/) — performance checks
- [playground/](playground/) — example server/client and query demos
- [ExampleQueries.md](ExampleQueries.md) — sample Cypher queries

How agents should work here
- Prefer link-first guidance: use existing docs instead of copying them into agent notes.
- Keep edits small and focused; do not broaden scope when fixing a bug or adding a feature.
- Verify with the smallest relevant build/test command and report the actual output.
- Check [README.md](README.md) and [Implementation.md](Implementation.md) before assuming the intended graph/query semantics.

Build and test workflow
```bash
# from repo root (Conan 2.x required)
mkdir -p build && cd build
# open62541 1.5.0's bundled nodesetLoader is missing #include <assert.h>; the -c flag
# force-includes it so the recipe builds under GCC 14.
conan install .. --build=missing -of . \
  -c 'open62541/*:tools.build:cflags=["-D_GNU_SOURCE","-include","assert.h"]'
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build . -- -j$(nproc)
ctest --output-on-failure -j1
```

Project-specific conventions
- This is a CMake + Conan 2 project using the `CMakeDeps`/`CMakeToolchain` generators; dependencies (`open62541`, `nlohmann_json`, `gtest`, `benchmark`, `cpp-httplib`) are declared in [conanfile.txt](conanfile.txt).
- The main library target is `graphForOpc`, defined in [CMakeLists.txt](CMakeLists.txt).
- Test executables are added in [tests/CMakeLists.txt](tests/CMakeLists.txt); current examples include `matchClause`, `parser`, `query`, `Path`, `HierachicalVisitor`, and `ReferenceDescription`.
- The `playground` target is used for query server/client demonstrations, and the benchmark target is useful for performance-oriented changes.
- The repo contains a vendored or nested `libcypher-parser` dependency; be careful when changing parser interfaces or consuming parser output.

Files to inspect first for likely root causes
- [Implementation.md](Implementation.md)
- [README.md](README.md)
- [src/graph/](src/graph/)
- [src/cypher/](src/cypher/)
- [tests/CMakeLists.txt](tests/CMakeLists.txt)

Areas that need extra care
- Query semantics and graph traversal logic in [src/graph/](src/graph/)
- Cypher parsing and path representation in [src/cypher/](src/cypher/)
- Changes to match behavior, path matching, or source traversal should be checked against the relevant tests under [tests/](tests/)
- Example queries in [ExampleQueries.md](ExampleQueries.md) and [playground/](playground/) are useful for integration-level validation

If more guidance is needed
- Create smaller focused agent files only when the repo becomes too large for a single AGENTS.md, for example by test or subsystem area.
