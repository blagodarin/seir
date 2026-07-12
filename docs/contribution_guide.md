# Contribution guide

###### Directory layout

Each build system target (program or library) resides in its own directory.

Targets are grouped into the following top-level directories:
- `libs` for libraries.
- `utils` for command line utilities.
- `tools` for standalone UI tools.
- `examples` for example applications build with Seir.
- `usage` for examples of using Seir in other projects.
  These aren't included in the main build system, but are build by the CI.

Each target has a common diractory layout:
- `src` for source files (including target-private headers).
  Optional for header-only libraries.
- `include/seir_{target}` for public header files (only for libraries).
  No subdirectories are allowed.
- `data` for target-specific data files.
- `tests` for unit test subtarget (only for libraries and utilities).
  Has an `src` subdirectory with source files,
  and may have a `data` subdirectory with test-specific data.
- `benchmarks` contains performance benchmark subtarget (only for libraries).
  Has an `src` subdirectory with source files.

Other top-level directories are:
- `cmake` for build system scripts.
- `data` for shared data files (e. g. assets).
- `docs` for common documentation.

###### Commit messages

Commit messages must follow [Conventional Commits](https://conventionalcommits.org/),
with headers structured as `type(scope): subject` (scope is optional).

Type must be one of:
- `feat` — code changes that introduce new behavior.
- `fix` — code changes that fix unintended behavior.
- `refactor` — code changes that should not change behavior.
- `test` — changes to tests.
- `build` — changes to the build system.
- `ci` — changes to CI configuration files and scripts.
- `docs` — changes to documentation only.
- `chore` — changes to other files.

## Naming conventions

Functions that produce immutable objects from source data are generally called `load`.
They usually return `SharedPtr`s.

Functions that produce stateful objects are generally called `create`.
They usually return `UniquePtr`s.
