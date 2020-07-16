[![pypi](https://badge.fury.io/py/signal-temporal-logic.svg)](https://badge.fury.io/py/signal-temporal-logic)

# Signal Temporal Logic Monitoring

This package provides an interface to define offline monitoring for Signal Temporal
Logic (STL) specifications. The library is written in C++ (and can be used with [CMake])
and has been wrapped for Python usage using [pybind11].

The library is inspired by the following projects:

- [py-metric-temporal-logic] is a tool written in pure Python, and provides an elegant
  interface for evaluating discrete time signals using Metric Temporal Logic (MTL).
- [Breach] and [S-TaLiRo] are [Matlab] toolboxes designed for falsification and
  simulation-based testing of cyber-physical systems with STL and MTL specifications,
  respectively. One of their various features includes the ability to evaluate the
  robustness of signals against STL/MTL.

The `signal-temporal-logic` library aims to be different from the above in the following
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

## List of Quantitative Semantics

- [Classic Robustness](http://www-verimag.imag.fr/~maler/Papers/sensiform.pdf)
    - A. Donzé and O. Maler, "Robust Satisfaction of Temporal Logic over Real-Valued
    Signals," in Formal Modeling and Analysis of Timed Systems, Berlin, Heidelberg,
    2010, pp. 92–106.
- [Temporal Logic as Filtering](https://arxiv.org/abs/1510.08079)
    - A. Rodionova, E. Bartocci, D. Nickovic, and R. Grosu, "Temporal Logic As
    Filtering," in Proceedings of the 19th International Conference on Hybrid Systems:
    Computation and Control, New York, NY, USA, 2016, pp. 11–20.
- [Smooth Cumulative Semantics](https://arxiv.org/abs/1904.11611)
    - I. Haghighi, N. Mehdipour, E. Bartocci, and C. Belta, "Control from Signal
    Temporal Logic Specifications with Smooth Cumulative Quantitative Semantics,"
    arXiv:1904.11611 [cs], Apr. 2019.

# Installing Python package

## Using `pip`

```shell
$ python3 -m pip install signal-temporal-logic
```

## Build from source

**Requirements:** `cmake` >= 3.5, `git` and a C++ compiler that supports C++17.

First clone the repository:
```shell
$ git clone https://github.com/anand-bala/signal-temporal-logic
```

Then install using `pip`, install the package:
```shell
$ python3 -m pip install -U .
```

# Usage

See the [examples/ directory](examples/) for some usage examples in C++ and Python.


