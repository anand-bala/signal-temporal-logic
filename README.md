Argus: Temporal Logic Monitoring Tool
=====================================

[Argus] aims to be a holistic tool to define monitors for Signal Temporal Logic and
Metric Temporal Logic, and their different semantics.

[Argus]: https://www.britannica.com/topic/Argus-Greek-mythology

The library is inspired by the following projects:

- [py-metric-temporal-logic] is a tool written in pure Python, and provides an elegant
  interface for evaluating discrete time signals using Metric Temporal Logic (MTL).
- [Breach] and [S-TaLiRo] are [Matlab] toolboxes designed for falsification and
  simulation-based testing of cyber-physical systems with STL and MTL specifications,
  respectively. One of their various features includes the ability to evaluate the
  robustness of signals against STL/MTL.

The `Argus` library aims to be different from the above in the following
ways:

- Written for speed and targets Python.
    - While [py-metric-temporal-logic] is written in Python, it doesn't perform
      computations efficiently and quickly.
- Support for multiple quantitative semantics.
    - All the above tools have their own way of computing the quantitative semantics for
      STL/MTL specifications.
    - This tool will try to support common ways of computing the robustness, but will
      also have support for other quantitative semantics of STL. 


[CMake]: https://cmake.org/
[pybind11]: https://pybind11.readthedocs.io/en/stable/
[py-metric-temporal-logic]: https://github.com/mvcisback/py-metric-temporal-logic/
[Matlab]: https://www.mathworks.com/products/matlab.html
[Breach]: https://github.com/decyphir/breach
[S-TaLiRo]: https://sites.google.com/a/asu.edu/s-taliro/s-taliro


# Prerequisites

- C++ compiler that supports at least C++17.
    - The library is tested against Clang 8+, GCC 8+, Visual Studio 2017, and Visual
      Studio 2019.
- CMake version >= 3.11
    - The tool uses [FetchContent] to manage dependencies, and thus needs at least version
      3.11 to use. It is relatively easy to install a newer version of CMake using the
      [official APT repositories](https://apt.kitware.com/) (for Debian/Ubuntu) or using
      [`pip`](https://pypi.org/project/cmake/).
- [git](https://git-scm.com/) (for managing dependencies again).

[FetchContent]: https://cmake.org/cmake/help/latest/module/FetchContent.html

# Usage

See the [examples/ directory](examples/) for some usage examples in C++ and Python.


