#include "bindings.hh"
#include "signal_tl/signal.hh"

void init_signal_module(py::module& parent) {
  using namespace signal;

  auto m = parent.def_submodule("signal", "A general class of signals (PWL, etc.)");
  py::bind_vector<std::vector<float>>(m, "FloatList", py::buffer_protocol());
  py::bind_vector<std::vector<Sample>>(m, "SampleList");
  py::bind_map<Trace>(m, "Trace")
      .def(py::init<>())
      .def(py::init<const Trace&>(), "other"_a);

  py::class_<Sample>(m, "Sample")
      .def(py::init())
      .def_readonly("time", &Sample::time)
      .def_readonly("value", &Sample::value)
      .def_readonly("derivative", &Sample::value);

  py::class_<Signal, std::shared_ptr<Signal>>(m, "Signal")
      .def(py::init<>())
      .def(py::init<const std::vector<Sample>&>())
      .def(
          py::init<const std::vector<float>&, const std::vector<float>&>(),
          "points"_a,
          "times"_a)
      .def_property_readonly("begin_time", &Signal::begin_time)
      .def_property_readonly("end_time", &Signal::end_time)
      .def("__str__", &repr<Signal>)
      .def(
          "__iter__",
          [](const Signal& s) { return py::make_iterator(s.begin(), s.end()); },
          py::keep_alive<0, 1>())
      .def("simplify", &Signal::simplify)
      .def("resize", &Signal::resize, "start"_a, "end"_a, "fill"_a)
      .def("shift", &Signal::shift, "dt"_a);
}
