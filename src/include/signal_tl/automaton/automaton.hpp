#pragma once

#ifndef SIGNALTL_AUTOMATON_HPP
#define SIGNALTL_AUTOMATON_HPP

#include <cmath>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace signal_tl::automaton {

struct Variable;
struct State;
struct Transition;
struct Automaton;

struct Constraint;

/// An Automaton variable.
struct Variable {
  enum struct Type { INPUT, OUTPUT };

  Variable(
      std::string name,
      unsigned int min_range,
      unsigned int max_range,
      Type type) :
      m_name{std::move(name)},
      m_value{0},
      m_min_value{min_range},
      m_max_value{max_range},
      m_type{type} {}

  /// Return the name of the Variable
  std::string to_string() const {
    return this->m_name;
  }

  inline bool operator==(const Variable& other) {
    return (
        (this->m_name == other.m_name) && (this->m_value == other.m_value) &&
        (this->m_min_value == other.m_value) &&
        (this->m_max_value == other.m_max_value) && (this->m_type == other.m_type));
  }

  struct Compare {
    bool operator()(const Variable& a, const Variable& b) {
      return a.name() < b.name();
    }
  };

  constexpr int value() const {
    return this->m_value;
  }

  std::string name() const {
    return this->m_name;
  }

  constexpr unsigned int min_value() const {
    return m_min_value;
  }

  constexpr unsigned int max_value() const {
    return m_max_value;
  }

  constexpr bool is_boolean() const {
    return (m_min_value == 0) && (m_max_value == 1);
  }

 private:
  /// Name of the variable
  std::string m_name;

  /// Value of the variable
  unsigned int m_value; // TODO: Should this be a double?

  /// Minimum value of the variable range
  unsigned int m_min_value;
  /// Maximum value of the variable range.
  unsigned int m_max_value;

  /// Determines if the variable is an input or output type.
  Type m_type;

  friend Automaton;
  friend Transition;
};

/// A state in the automaton.
struct State {
  State()             = default;
  State(const State&) = default;
  State(State&&)      = default;

  State(bool is_initial, bool is_accepting) :
      m_is_initial{is_initial}, m_is_accepting{is_accepting} {}

  State(int id, bool is_initial, bool is_accepting) :
      m_id{id}, m_is_initial{is_initial}, m_is_accepting{is_accepting} {}

  State(int id, int label, bool is_initial, bool is_accepting) :
      m_id{id},
      m_label{label},
      m_is_initial{is_initial},
      m_is_accepting{is_accepting} {}

  struct Compare {
    bool operator()(const State& a, const State& b) {
      return a.id() < b.id();
    }
  };

  int id() const {
    return this->m_id;
  }

  int label() const {
    return this->m_label;
  }

  bool is_initial() const {
    return this->m_is_initial;
  }

  bool is_accepting() const {
    return this->m_is_accepting;
  }

 private:
  int m_id;
  int m_label;

  bool m_is_initial;
  /// Is this State accepting?
  bool m_is_accepting;

  friend Transition;
  friend Automaton;
};

/// A transition in the automaton
struct Transition {
  Transition(const Transition&) = default;
  Transition(Transition&&)      = default;

  Transition(State src, State dst) : m_src{src}, m_dst{dst} {}

  State src() const {
    return m_src;
  }

  State dst() const {
    return m_dst;
  }

  const std::shared_ptr<Constraint> guard() const {
    return m_guard;
  }

  struct Compare {
    constexpr bool operator()(const Transition& a, const Transition& b) {
      if (a.src().id() < b.src().id()) {
        return true;
      } else if (a.src().id() == b.src().id() && a.dst().id() < b.dst().id()) {
        return true;
      }
      return false;
    }
  };

 private:
  State m_src, m_dst;

  std::set<Variable, Variable::Compare> m_reset_vars;
  std::set<Variable, Variable::Compare> m_increment_vars;

  std::shared_ptr<Constraint> m_guard;

  friend Automaton;
};

struct Automaton {
 protected:
  /// A map from a variable name to a corresponding `Variable`
  std::map<std::string, Variable> m_variables;
  /// Map of states, where the key is the `int` ID of the `State`.
  std::map<int, State> m_states;
  /// Set of initial states, there the content is the ` int` ID of the `State`.
  std::set<int> m_initial_states;
  /// Set of accepting states, there the content is the ` int` ID of the
  /// `State`.
  std::set<int> m_accepting_states;
  /// Set of transitions in adjacency list form.
  std::map<int, std::set<Transition, Transition::Compare>> m_transitions;

 public:
  Automaton()                 = default;
  Automaton(const Automaton&) = default;
  Automaton(Automaton&&)      = default;

  /// Create an automaton containing exactly 1 state that corresponds to an "error
  /// state" with ID -1, which is not accepting nor is it an initial state.
  Automaton(bool has_error_state) : Automaton{} {
    if (has_error_state) {
      this->add_state(State{-1, false, false});
    }
  }

  const std::map<std::string, Variable>& variables() const {
    return m_variables;
  }
  const std::map<int, State>& states() const {
    return m_states;
  }

  size_t num_variables() const {
    return m_variables.size();
  }
  size_t num_states() const {
    return m_states.size();
  }

  /// Add a variable to the automaton.
  ///
  /// TODO: Pass by value or reference?
  bool add_variable(Variable var) {
    auto ret = m_variables.insert({var.name(), var});
    return ret.second;
  }

  /// Add a state to the automaton. Returns `true` if it was a success, `false`
  /// otherwise.
  ///
  /// Internally, this handles the case for if the `State` is accepting and if it is
  /// an initial state.
  bool add_state(State state) {
    auto [it, success] = m_states.insert({state.id(), state});
    if (success) {
      if (it->second.is_accepting()) {
        m_accepting_states.insert(it->first);
      }
      if (it->second.is_initial()) {
        m_initial_states.insert(it->first);
      }
    }
    return success;
  }

  /// Add a transition to the automaton.
  ///
  /// TODO: Pass by value or reference?
  bool add_transition(Transition transition) {
    auto src    = transition.src();
    auto search = m_transitions.find(src.id());
    // TODO: Should I insert the state if it doesn't exist?
    if (search != m_transitions.end()) {
      auto ret = search->second.insert(transition);
      return ret.second;
    }
    return false; // TODO (see above)
  }
};

} // namespace signal_tl::automaton

#endif /* end of include guard: SIGNALTL_AUTOMATON_HPP */
