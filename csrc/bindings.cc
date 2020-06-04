#include <iostream>
#include <sstream>

#include <pybind11/pybind11.h>

#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <pybind11/operators.h>

#include "ast.hh"
using namespace pybind11::literals;
namespace py = pybind11;

template <typename T>
std::string repr(const T& expr) {
  std::ostringstream os;
  os << expr;
  return os.str();
}

void init_ast_module(py::module&);

PYBIND11_MODULE(_cext, m) {
  m.doc() = "Signal Temporal Logic library.";
  init_ast_module(m);
}

void init_ast_module(py::module& parent) {
  using namespace ast;

  parent.def("Const", &Const::asExpr);
  parent.def("Predicate", &Predicate::asExpr);
  parent.def("Not", &Not::asExpr);
  parent.def("And", &And::asExpr);
  parent.def("Or", &Or::asExpr);
  parent.def("Always", &Always::asExpr);
  parent.def("Eventually", &Eventually::asExpr);
  parent.def("Until", &Until::asExpr);

  auto m = parent.def_submodule("ast", "Define the AST nodes for STL");
  // py::class_<ast::Expr> expr_cls(m, "Expr");
  // expr_cls.def(py::self & py::self);
  // expr_cls.def(py::self | py::self);
  // expr_cls.def(~py::self);

  py::class_<Const, ConstPtr>(m, "Const")
      .def(py::init<bool>(), "value"_a)
      .def_readonly("value", &Const::value)
      .def("__str__", &repr<Const>);

  py::class_<Predicate, PredicatePtr>(m, "Predicate")
      .def(py::init<const std::string&>(), "name"_a)
      .def_readonly("name", &Predicate::name)
      .def("__str__", &repr<Predicate>);

  py::class_<Not, NotPtr>(m, "Not")
      .def(py::init<const Expr&>(), "arg"_a)
      .def_readonly("arg", &Not::arg)
      .def("__str__", &repr<Not>);

  py::class_<And, AndPtr>(m, "And")
      .def(py::init<const std::vector<Expr>&>(), "args"_a)
      .def_readonly("args", &And::args)
      .def("__str__", &repr<And>);

  py::class_<Or, OrPtr>(m, "Or")
      .def(py::init<const std::vector<Expr>&>(), "args"_a)
      .def_readonly("args", &Or::args)
      .def("__str__", &repr<Or>);

  py::class_<Eventually, EventuallyPtr>(m, "Eventually")
      .def(
          py::init<const Expr&, std::optional<Interval>>(),
          "arg"_a,
          "interval"_a = std::nullopt)
      .def_readonly("arg", &Eventually::arg)
      .def_readonly("interval", &Eventually::arg)
      .def("__str__", &repr<Eventually>);

  py::class_<Always, AlwaysPtr>(m, "Always")
      .def(
          py::init<const Expr&, std::optional<Interval>>(),
          "arg"_a,
          "interval"_a = std::nullopt)
      .def_readonly("arg", &Always::arg)
      .def_readonly("interval", &Always::arg)
      .def("__str__", &repr<Always>);

  py::class_<Until, UntilPtr>(m, "Until")
      .def(
          py::init<const Expr&, const Expr&, std::optional<Interval>>(),
          "arg0"_a,
          "arg1"_a,
          "interval"_a = std::nullopt)
      .def_readonly("args", &Until::args)
      .def_readonly("interval", &Until::args)
      .def("__str__", &repr<Until>);
}
