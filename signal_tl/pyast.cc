#include "bindings.hh"
#include "signal_tl/ast.hh"

namespace {

template <typename T>
ast::Expr and_op(const std::shared_ptr<T>& lhs, const ast::Expr& rhs) {
  return ast::Expr{lhs} & rhs;
}

template <typename T>
ast::Expr or_op(const std::shared_ptr<T>& lhs, const ast::Expr& rhs) {
  return ast::Expr{lhs} | rhs;
}

template <typename T>
ast::Expr not_op(const std::shared_ptr<T>& expr) {
  return ~ast::Expr{expr};
}

} // namespace

void init_ast_module(py::module& parent) {
  using namespace ast;

  parent.def("Const", &Const::as_expr, "value"_a);
  parent.def("Predicate", &Predicate::as_expr, "name"_a);
  parent.def("Not", &Not::as_expr, "arg"_a);
  parent.def("And", &And::as_expr, "args"_a);
  parent.def("Or", &Or::as_expr, "args"_a);
  parent.def("Always", &Always::as_expr, "arg"_a, "interval"_a = std::nullopt);
  parent.def("Eventually", &Eventually::as_expr, "arg"_a, "interval"_a = std::nullopt);
  parent.def("Until", &Until::as_expr, "arg0"_a, "arg1"_a, "interval"_a = std::nullopt);

  auto m = parent.def_submodule("ast", "Define the AST nodes for STL");

  py::class_<Const, ConstPtr>(m, "Const")
      .def(py::init<bool>(), "value"_a)
      .def_readonly("value", &Const::value)
      .def("__repr__", &repr<Const>)
      .def("__and__", &and_op<Const>)
      .def("__or__", &or_op<Const>)
      .def("__invert__", &not_op<Const>);

  py::class_<Predicate, PredicatePtr>(m, "Predicate")
      .def(py::init<const std::string&>(), "name"_a)
      .def_readonly("name", &Predicate::name)
      .def("__repr__", &repr<Predicate>)
      .def("__and__", &and_op<Predicate>)
      .def("__or__", &or_op<Predicate>)
      .def("__invert__", &not_op<Predicate>);

  py::class_<Not, NotPtr>(m, "Not")
      .def(py::init<const Expr&>(), "arg"_a)
      .def_readonly("arg", &Not::arg)
      .def("__repr__", &repr<Not>)
      .def("__and__", &and_op<Not>)
      .def("__or__", &or_op<Not>)
      .def("__invert__", &not_op<Not>);

  py::class_<And, AndPtr>(m, "And")
      .def(py::init<const std::vector<Expr>&>(), "args"_a)
      .def_readonly("args", &And::args)
      .def("__repr__", &repr<And>)
      .def("__and__", &and_op<And>)
      .def("__or__", &or_op<And>)
      .def("__invert__", &not_op<And>);

  py::class_<Or, OrPtr>(m, "Or")
      .def(py::init<const std::vector<Expr>&>(), "args"_a)
      .def_readonly("args", &Or::args)
      .def("__repr__", &repr<Or>)
      .def("__and__", &and_op<Or>)
      .def("__or__", &or_op<Or>)
      .def("__invert__", &not_op<Or>);

  py::class_<Eventually, EventuallyPtr>(m, "Eventually")
      .def(
          py::init<const Expr&, std::optional<Interval>>(),
          "arg"_a,
          "interval"_a = std::nullopt)
      .def_readonly("arg", &Eventually::arg)
      .def_readonly("interval", &Eventually::arg)
      .def("__repr__", &repr<Eventually>)
      .def("__and__", &and_op<Eventually>)
      .def("__or__", &or_op<Eventually>)
      .def("__invert__", &not_op<Eventually>);

  py::class_<Always, AlwaysPtr>(m, "Always")
      .def(
          py::init<const Expr&, std::optional<Interval>>(),
          "arg"_a,
          "interval"_a = std::nullopt)
      .def_readonly("arg", &Always::arg)
      .def_readonly("interval", &Always::arg)
      .def("__repr__", &repr<Always>)
      .def("__and__", &and_op<Always>)
      .def("__or__", &or_op<Always>)
      .def("__invert__", &not_op<Always>);

  py::class_<Until, UntilPtr>(m, "Until")
      .def(
          py::init<const Expr&, const Expr&, std::optional<Interval>>(),
          "arg0"_a,
          "arg1"_a,
          "interval"_a = std::nullopt)
      .def_readonly("args", &Until::args)
      .def_readonly("interval", &Until::args)
      .def("__repr__", &repr<Until>)
      .def("__and__", &and_op<Until>)
      .def("__or__", &or_op<Until>)
      .def("__invert__", &not_op<Until>);
}
