# Building the API reference

The API reference is generated with [Doxygen](https://www.doxygen.nl/) from the
source under `Code/`. Graphviz supplies inheritance diagrams. Building the
documentation does not require compiling OpenW3D, installing its SDK dependencies,
or providing game data.

## Build locally

Install CMake 3.25 or later, Ninja, Doxygen, and
[Graphviz](https://graphviz.org/download/), and make sure `cmake`, `ninja`,
`doxygen`, and `dot` are on your `PATH`. On Ubuntu:

```sh
sudo apt-get update
sudo apt-get install --no-install-recommends cmake ninja-build doxygen graphviz
```

From the repository root, run:

```sh
cmake -S docs -B build/docs -G Ninja
cmake --build build/docs --target docs
```

Open `build/docs/html/index.html` in your browser. Check that the class and file
lists are populated, search for `Vector3`, and follow a source link. If your
browser restricts search when opening local files, serve the output instead:

```sh
python -m http.server 8000 --bind 127.0.0.1 --directory build/docs/html
```

Then open <http://localhost:8000/>. Generated files stay under the already-ignored
`build/` directory and should not be committed. For a completely fresh build,
remove only `build/docs/` before rerunning both CMake commands. CMake creates the
output directory before Doxygen runs, including on a fresh checkout.

To add the same `docs` target to an existing game build, configure the repository
root with `-DW3D_BUILD_DOCS=ON`, then run `cmake --build <build-dir> --target docs`.
The generated reference is under `<build-dir>/docs/html/`. Documentation is
optional and is not built as part of the default game build.

The configuration enables `EXTRACT_ALL`, so existing code is listed even without
documentation comments. It excludes `Code/Libs`, `Code/Tests`, and
`Code/Tools/pluglib`; the latter contains older copies of engine classes that
would otherwise be merged into the same class pages. Doxygen warnings are saved to
`build/docs/doxygen-warnings.log`. Existing parser and documentation warnings do
not fail the build; review the log when changing comments or configuration.

## Add documentation

Use Doxygen comments immediately before a declaration, for example:

```cpp
/**
 * @brief Returns the squared distance between two positions.
 * @param from Starting position.
 * @param to Ending position.
 * @return Squared distance in world units.
 */
float DistanceSquared(const Vector3& from, const Vector3& to);
```

Describe behavior, units, ownership, and preconditions when they matter. Build the
reference locally to check how your comments render. The generated homepage is
maintained in `docs/index.md`; generation settings are in `docs/Doxyfile.in`.

## Pull request review

The `Documentation` workflow in `.github/workflows/docs.yml` builds the reference
for pull requests targeting `main` that change `Code/`, `docs/`, the root
`CMakeLists.txt`, or the workflow. It uses the same CMake target inside an
Ubuntu 24.04 container on an `ubuntu-latest` runner.
It uploads an `openw3d-api-docs` artifact containing the `html/` directory and
Doxygen warning log. Download and extract it from the workflow run's summary,
then open `html/index.html` to review the result. Artifacts are retained for 14 days.

Pull requests, including those from forks, only build and upload review artifacts.
The build job uses read-only repository permissions and does not require Pages
to be enabled. Maintainers may need to approve workflow runs from new contributors.

Only runs on `main` in `w3dhub/OpenW3D` publish the reference to GitHub Pages.
