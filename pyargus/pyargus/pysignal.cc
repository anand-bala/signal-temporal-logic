#include "bindings.hpp"         // for init_signal_module
#include "signal_tl/ast.hpp"    // for signal_tl
#include "signal_tl/fmt.hpp"    // IWYU pragma: keep
#include "signal_tl/signal.hpp" // for Sample, Trace, Signal, SignalPtr

#include <array>                    // for array
#include <cstddef>                  // for size_t
#include <exception>                // for exception
#include <fmt/format.h>             // for format
#include <map>                      // for operator==, map, operator!=
#include <memory>                   // for allocator, operator<<, __shared_...
#include <pybind11/attr.h>          // for buffer_protocol, keep_alive
#include <pybind11/cast.h>          // for operator""_a, handle::cast, cast_op
#include <pybind11/detail/common.h> // for ignore_unused, constexpr_first
#include <pybind11/detail/descr.h>  // for operator+
#include <pybind11/operators.h>     // for self, self_t, operator<, operator<=
#include <pybind11/pybind11.h>      // for class_, init, make_iterator, mod...
#include <pybind11/pytypes.h>       // for getattr, iterable, sequence, dict
#include <pybind11/stl_bind.h>      // for bind_vector, bind_map
#include <string>                   // for basic_string
#include <vector>                   // for vector

using namespace signal_tl;

void init_signal_module(py::module& parent) {
  using namespace signal;

  auto m = parent.def_submodule("signal", "A general class of signals (PWL, etc.)");
  py::bind_vector<std::vector<double>>(m, "DoubleList", py::buffer_protocol());
  py::bind_vector<std::vector<Sample>>(m, "SampleList");
  py::bind_map<Trace>(m, "Trace").def(py::init<const Trace&>(), "other"_a);

  py::class_<Sample>(m, "Sample")
      .def(py::init())
      .def_readonly("time", &Sample::time)
      .def_readonly("value", &Sample::value)
      .def_readonly("derivative", &Sample::value)
      .def(py::self < py::self)
      .def(py::self > py::self)
      .def(py::self >= py::self)
      .def(py::self <= py::self)
      .def("__repr__", [](const Sample& e) { return fmt::format("{}", e); });

  py::class_<Signal, std::shared_ptr<Signal>>(m, "Signal")
      .def(py::init<>())
      .def(py::init<const Signal&>(), "other"_a)
      .def(py::init<const std::vector<Sample>&>())
      .def(
          py::init<const std::vector<double>&, const std::vector<double>&>(),
          "points"_a,
          "times"_a)
      .def_property_readonly("begin_time", &Signal::begin_time)
      .def_property_readonly("end_time", &Signal::end_time)
      .def("simplify", &Signal::simplify)
      .def("resize", &Signal::resize, "start"_a, "end"_a, "fill"_a)
      .def("shift", &Signal::shift, "dt"_a)
      .def("__repr__", [](const Signal& e) { return fmt::format("{}", e); })
      .def(
          "__iter__",
          [](const Signal& s) { return py::make_iterator(s.begin(), s.end()); },
          py::keep_alive<0, 1>())
      .def(
          "__getitem__",
          [](const SignalPtr& s, size_t i) { return s->at_idx(i).value; })
      .def("__len__", &Signal::size)
      .def("at", [](const SignalPtr& s, double t) { return s->at(t).value; });

  m.def("synchronize", &synchronize, "x"_a, "y"_a);
}
