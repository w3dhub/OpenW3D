# Automated tests

CTest sources, registration, and shared test helpers live in this directory.
`tests/CMakeLists.txt` runs after the production targets in `Code/` have been
defined. `BUILD_TESTING` controls the entire suite; setting it to `OFF` creates no
test executables or test-only Qt dependencies.

Test source directories mirror the corresponding component under `Code/`.
Production libraries, widgets, and Designer forms stay with their applications.
The historical projects under `Code/Tests/` are legacy samples and are not part
of the CMake/CTest suite.

## Build and run

Configure a normal build with `-DBUILD_TESTING=ON`, then run:

```sh
cmake --build build --config Release --target w3d_tests
ctest --test-dir build -C Release --output-on-failure
```

Use your configured build directory in place of `build`. All existing individual
test targets and CTest names remain available. For example:

```sh
cmake --build build --config Release --target wwlib_msgloop_tests
ctest --test-dir build -C Release -R '^wwlib_msgloop_tests$' --output-on-failure
```

Qt tests are registered when their tool targets exist. Renderer tests retain
their Windows requirement. The standalone message-loop test can be built on
Linux with SDL3 without building the rest of `wwlib`; other engine targets still
have portability limitations, so `w3d_tests` is not a portable-only subset.

## Add a test

1. Place the source under the matching component directory here.
2. Register it in `tests/CMakeLists.txt` with `w3d_add_test`, listing its sources
   and the production targets it links. This also adds it to `w3d_tests`.
3. Keep platform and optional-component conditions around its registration.
4. For Qt tests, pass `ARGS -o -,txt` and call `w3d_configure_qt_test` to configure
   Qt autogeneration, headless execution, runtime lookup, and the timeout.
5. Use `w3d_add_designer_test(test_name widget_target)` to validate forms listed
   in an existing widget target. Each form check gets its own output directory.

Keep test setup out of production `CMakeLists.txt` files. Link shared production
targets instead of duplicating application source lists where possible. Tests
that need native graphics or external game assets should keep explicit opt-ins
and CTest labels when they are introduced.
