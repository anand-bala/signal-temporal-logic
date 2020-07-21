#include "bindings.hh"
#include "signal_tl/ast.hh"
#include "signal_tl/fmt.hh"

using namespace signal_tl;
namespace {

template <typename T>
ast::Expr and_op(const T lhs, const ast::Expr& rhs) {
  return ast::Expr{lhs} & rhs;
}

template <typename T>
ast::Expr or_op(const T& lhs, const ast::Expr& rhs) {
  return ast::Expr{lhs} | rhs;
}

template <typename T>
ast::Expr not_op(const T& expr) {
  return ~ast::Expr{expr};
}

} // namespace

void init_ast_module(py::module& parent) {
  parent.def("Const", &Const, "value"_a);
  parent.def("Predicate", &Predicate, "name"_a);
  parent.def("Not", &Not, "arg"_a);
  parent.def("And", &And, "args"_a);
  parent.def("Or", &Or, "args"_a);
  parent.def("Always", &Always, "arg"_a, "interval"_a = std::nullopt);
  parent.def("Eventually", &Eventually, "arg"_a, "interval"_a = std::nullopt);
  parent.def("Until", &Until, "arg0"_a, "arg1"_a, "interval"_a = std::nullopt);

  auto m = parent.def_submodule("ast", "Define the AST nodes for STL");
  py::class_<ast::Const>(m, "Const")
      .def(py::init<bool>(), "value"_a)
      .def_readonly("value", &ast::Const::value)
      .def("__and__", &and_op<ast::Const>)
      .def("__or__", &or_op<ast::Const>)
      .def("__invert__", &not_op<ast::Const>)
      .def("__repr__", [](const ast::Const& e) { return fmt::format("{}", e); });

  py::class_<ast::Predicate>(m, "Predicate")
      .def(py::init<const std::string&>(), "name"_a)
      .def_readonly("name", &ast::Predicate::name)
      .def("__and__", &and_op<ast::Predicate>)
      .def("__or__", &or_op<ast::Predicate>)
      .def("__invert__", &not_op<ast::Predicate>)
      .def(py::self < double())
      .def(py::self <= double())
      .def(py::self > double())
      .def(py::self >= double())
      .def("__repr__", [](const ast::Predicate& e) { return fmt::format("{}", e); });

  py::class_<ast::Not, ast::NotPtr>(m, "Not")
      .def(py::init<const Expr&>(), "arg"_a)
      .def_readonly("arg", &ast::Not::arg)
      .def("__and__", &and_op<ast::NotPtr>)
      .def("__or__", &or_op<ast::NotPtr>)
      .def("__invert__", &not_op<ast::NotPtr>)
      .def("__repr__", [](const ast::NotPtr e) { return fmt::format("{}", *e); });

  py::class_<ast::And, ast::AndPtr>(m, "And")
      .def(py::init<const std::vector<Expr>&>(), "args"_a)
      .def_readonly("args", &ast::And::args)
      .def("__and__", &and_op<ast::AndPtr>)
      .def("__or__", &or_op<ast::AndPtr>)
      .def("__invert__", &not_op<ast::AndPtr>)
      .def("__repr__", [](const ast::AndPtr& e) { return fmt::format("{}", *e); });

  py::class_<ast::Or, ast::OrPtr>(m, "Or")
      .def(py::init<const std::vector<Expr>&>(), "args"_a)
      .def_readonly("args", &ast::Or::args)
      .def("__and__", &and_op<ast::OrPtr>)
      .def("__or__", &or_op<ast::OrPtr>)
      .def("__invert__", &not_op<ast::OrPtr>)
      .def("__repr__", [](const ast::Or& e) { return fmt::format("{}", e); });

  py::class_<ast::Eventually, ast::EventuallyPtr>(m, "Eventually")
      .def(
          py::init<const Expr&, std::optional<ast::Interval>>(),
          "arg"_a,
          "interval"_a = std::nullopt)
      .def_readonly("arg", &ast::Eventually::arg)
      .def_readonly("interval", &ast::Eventually::arg)
      .def("__and__", &and_op<ast::EventuallyPtr>)
      .def("__or__", &or_op<ast::EventuallyPtr>)
      .def("__invert__", &not_op<ast::EventuallyPtr>)
      .def("__repr__", [](const ast::Eventually& e) { return fmt::format("{}", e); });

  py::class_<ast::Always, ast::AlwaysPtr>(m, "Always")
      .def(
          py::init<const Expr&, std::optional<ast::Interval>>(),
          "arg"_a,
          "interval"_a = std::nullopt)
      .def_readonly("arg", &ast::Always::arg)
      .def_readonly("interval", &ast::Always::arg)
      .def("__and__", &and_op<ast::AlwaysPtr>)
      .def("__or__", &or_op<ast::AlwaysPtr>)
      .def("__invert__", &not_op<ast::AlwaysPtr>)
      .def("__repr__", [](const ast::Always& e) { return fmt::format("{}", e); });

  py::class_<ast::Until, ast::UntilPtr>(m, "Until")
      .def(
          py::init<const Expr&, const Expr&, std::optional<ast::Interval>>(),
          "arg0"_a,
          "arg1"_a,
          "interval"_a = std::nullopt)
      .def_readonly("args", &ast::Until::args)
      .def_readonly("interval", &ast::Until::args)
      .def("__and__", &and_op<ast::UntilPtr>)
      .def("__or__", &or_op<ast::UntilPtr>)
      .def("__invert__", &not_op<ast::UntilPtr>)
      .def("__repr__", [](const ast::Until& e) { return fmt::format("{}", e); });
}
