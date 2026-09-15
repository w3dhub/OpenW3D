# Building and publishing the API reference

The API reference is generated with [Doxygen](https://www.doxygen.nl/) from the
source under `Code/`. Graphviz supplies inheritance diagrams. Building the
documentation does not require compiling OpenW3D, installing its SDK dependencies,
or providing game data.

## Build locally

Install Doxygen and [Graphviz](https://graphviz.org/download/), and make sure
`doxygen` and `dot` are on your `PATH`. On Ubuntu 24.04:

```sh
sudo apt-get update
sudo apt-get install --no-install-recommends doxygen graphviz
```

From the repository root, run:

```sh
doxygen docs/Doxyfile
```

Open `build/docs/html/index.html` in your browser. Check that the class and file
lists are populated, search for `Vector3`, and follow a source link. If your
browser restricts search when opening local files, serve the output instead:

```sh
python -m http.server 8000 --bind 127.0.0.1 --directory build/docs/html
```

Then open <http://localhost:8000/>. Generated files stay under the already-ignored
`build/` directory and should not be committed. For a completely fresh build,
remove only `build/docs/` before rerunning Doxygen.

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
maintained in `docs/index.md`; generation settings are in `docs/Doxyfile`.

## Pull request review

The `Documentation` workflow in `.github/workflows/docs.yml` builds the reference
for pull requests targeting `main` that change `Code/`, `docs/`, or the workflow.
It uploads an `openw3d-api-docs` artifact containing the `html/` directory and
Doxygen warning log. Download and extract it from the workflow run's summary,
then open `html/index.html` to review the result. Artifacts are retained for 14 days.

Pull requests, including those from forks, only build and upload review artifacts.
The build job uses read-only repository permissions and does not require Pages
to be enabled. Maintainers may need to approve workflow runs from new contributors.

## Enable publishing once, in the upstream repository

A repository administrator or maintainer must open `w3dhub/OpenW3D` on GitHub and
select **Settings > Pages > Build and deployment > Source > GitHub Actions**.
These repository settings are separate from the pull request.

The workflow builds on relevant pushes to `main` and can also be started manually
from **Actions > Documentation > Run workflow** after it is merged. Only runs on
`main` in `w3dhub/OpenW3D` can publish; runs in forks do not deploy.

The deploy job uses the `github-pages` environment and grants `pages: write` and
`id-token: write` only to that job. If the environment has deployment protection
rules, allow `main` and satisfy any required approvals. No personal access token
or separate documentation repository is needed.

After the first successful deployment, the default address is
<https://w3dhub.github.io/OpenW3D/>. The deployment job reports the actual URL,
which can differ if a custom domain is configured. Verify the homepage, class
list, search, and a source link there.

If deployment fails because Pages is not configured yet, enable it and rerun the
workflow on `main`. The documentation build and review artifact can still succeed
before hosting is configured.
