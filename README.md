[![Windows](https://github.com/blagodarin/seir/actions/workflows/windows.yml/badge.svg?branch=main)](https://github.com/blagodarin/seir/actions/workflows/windows.yml)
[![Linux (GCC)](https://github.com/blagodarin/seir/actions/workflows/linux_gcc.yml/badge.svg?branch=main)](https://github.com/blagodarin/seir/actions/workflows/linux_gcc.yml)
[![Linux (Clang)](https://github.com/blagodarin/seir/actions/workflows/linux_clang.yml/badge.svg?branch=main)](https://github.com/blagodarin/seir/actions/workflows/linux_clang.yml)
[![macOS](https://github.com/blagodarin/seir/actions/workflows/macos.yml/badge.svg?branch=main)](https://github.com/blagodarin/seir/actions/workflows/macos.yml)
[![Coverage](https://codecov.io/gh/blagodarin/seir/branch/main/graph/badge.svg?token=HSYMD4YFV5)](https://codecov.io/gh/blagodarin/seir)

# Seir

Seir is a framework for building interactive applications.

Seir aims to be:
* **Easy to start using.**
  Building Seir requires internet connection, [Git](https://git-scm.com/downloads/),
  recent [CMake](https://cmake.org/download/) and a supported C++ compiler,
  which is essentially the bare minimum for building modern C++ projects.
  On top of that, you don't even need to build Seir manually in order to use it, as
  [a few lines of CMake script](examples/usage/cmake_fetchcontent/CMakeLists.txt)
  will get you Seir with all its dependencies.
* **Highly configurable.**
  Every component and every supported format can be disabled individually,
  and third-party libraries can be provided by the build system and statically
  linked into the final binary. Static runtime builds on Windows are also supported.

### Disclaimer

Seir was created as a vessel for its author's erratic software engineering work.
It may be of use, especially in part, but it may never become a production-ready solution.
No specific features are to be expected, but opening issues is still appreciated.
