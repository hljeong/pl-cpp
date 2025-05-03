#pragma once

#include <cassert>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "lib/cpp_utils/cpy/cpy.h"
#include "lib/cpp_utils/fmt/fmt.h"

// todo: delete this -- move impl to nfa.cc?
using namespace cpy;
using namespace fmt;

namespace nfa {

// poor man's sum type
class Symbol {
public:
  static constexpr struct Epsilon {
  } epsilon{};

  Symbol(Epsilon e_) : e(e_) {}
  Symbol(char c_) : c(c_) {}

  operator Epsilon() const { return *e; }
  operator char() const { return *c; }

  template <typename T> bool is() const;

  bool operator<(const Symbol &other) const;

  friend String repr(const Symbol &value);

private:
  std::optional<Epsilon> e{std::nullopt};
  std::optional<char> c{std::nullopt};
};

template <> inline bool Symbol::is<Symbol::Epsilon>() const {
  return e.has_value();
}
template <> inline bool Symbol::is<char>() const { return c.has_value(); }

// have to define this after instantiations of Symbol::is<T>...
inline bool Symbol::operator<(const Symbol &other) const {
  if (is<Epsilon>()) {
    return !other.is<Epsilon>();
  } else {
    return c < other.c;
  }
}

// todo: defining casting to std::optional<T> and then defineing is<T>() to be
// static_cast<std::optional<T>>(*this).has_value() doesnt work. static_cast-ing
// outside also shows has_value() is true -- why?
inline String repr(const Symbol &value) {
  return value.is<Symbol::Epsilon>() ? "nfa::Symbol::Epsilon"
                                     : fmt::repr(static_cast<char>(value));
}

class Nfa {
public:
  class State {
  public:
    State() : id(next_id++) {}

    bool operator<(State other) const { return id < other.id; }

    bool operator==(State other) const { return id == other.id; }

    inline friend String repr(const State &value) {
      return format("q{}", value.id);
    }

  private:
    static uint next_id;
    uint id;
  };

  using Alphabet = Set<char>;
  using StateTransition = Map<Symbol, Set<State>>;
  using Transition = Map<State, StateTransition>;

  using TQ = Set<State>;
  using Tq0 = State;
  using TS = Alphabet;
  using Td = Transition;
  using TF = Set<State>;

  Nfa(const TQ &Q_, const Tq0 &q0_, const TS &S_, const Td &d_, const TF &F_)
      : Q(Q_), q0(q0_), S(S_), d(d_), F(F_) {
    assert(Q.count(q0));
    for (const auto &state : Q) {
      if (!d.count(state)) {
        d[state] = {};
      }
    }
    for (const auto &[state, d] : d) {
      assert(Q.count(state));
      for (const auto &[symbol, states] : d) {
        assert(symbol.is<Symbol::Epsilon>() ||
               (symbol.is<char>() && S.count(symbol)));
        for (const auto &state_ : states) {
          assert(Q.count(state_));
        }
      }
    }
    for (const auto &state : F) {
      assert(Q.count(state));
    }
  }

  bool accepts(const String &s) const {
    Set<State> current = reachable({q0});

    for (char c : s) {
      if (not S.count(c)) {
        throw std::invalid_argument(format("illegal character '{}'", c));
      }

      Set<State> next;
      for (const auto &state : current) {
        if (d.at(state).count(c)) {
          assign(next, d.at(state).at(c));
        }
      }

      current = reachable(next);
    }

    for (const auto &state : current) {
      if (F.count(state)) {
        return true;
      }
    }
    return false;
  }

  operator Tuple<const TQ &, const Tq0 &, const TS &, const Td &, const TF>()
      const {
    return {Q, q0, S, d, F};
  }

  // concatenation
  Nfa operator+(const Nfa &other) const {
    const auto &[Q_, q0_, S_, d_, F_] = other;
    assert(disjoint(Q, Q_));

    auto d__ = combine(d, d_);
    for (const auto &state : F) {
      d__[state][Symbol::epsilon].insert(q0_);
    }

    return {combine(Q, Q_), q0, combine(S, S_), d__, F_};
  }

  // disjunction
  Nfa operator|(const Nfa &other) const {
    const auto &[Q_, q0_, S_, d_, F_] = other;
    assert(disjoint(Q, Q_));

    Nfa::State q0__;
    return {combine<TQ>({q0__}, Q, Q_), q0__, combine(S, S_),
            combine<Td>({{q0__, {{Symbol::epsilon, {q0, q0_}}}}}, d, d_),
            combine(F, F_)};
  }

  // kleene star
  Nfa operator*() const {
    State q0_;
    auto d_ = d;
    d_[q0_][Symbol::epsilon].insert(q0);
    for (const auto &state : F) {
      d_[state][Symbol::epsilon].insert(q0);
    }

    return {combine(Q, {q0_}), q0_, S, d_, combine(F, {q0_})};
  }

  friend String repr(const Nfa &value) {
    if (!value.d.size()) {
      return "{}";
    }

    auto str_state_transition_entry = [&](const auto &entry) {
      const auto &[symbol, states] = entry;
      return format("{} -> {}", symbol, states);
    };

    auto str_transition_entry = [&](const auto &entry) {
      const auto &[state, d] = entry;
      return format(
          "{}: {},", format(value.F.count(state) ? "[{}]" : "{}", state),
          brace(join(",\n", map(str_state_transition_entry, entries(d))),
                Bracket::Style::Block));
    };

    return brace(join("\n", map(str_transition_entry, entries(value.d))),
                 Bracket::Style::Block);
  }

private:
  TQ Q;
  Tq0 q0;
  TS S;
  Td d;
  TF F;

  Set<State> reachable(Set<State> states) const {
    Set<State> states_;

    Queue<State> bfs;
    for (const auto &state : states) {
      bfs.push(state);
      states_.insert(state);
    }

    while (bfs.size()) {
      State state = bfs.front();
      bfs.pop();

      if (d.at(state).count(Symbol::epsilon)) {
        for (const auto &next_state : d.at(state).at(Symbol::epsilon)) {
          if (!states_.count(next_state)) {
            bfs.push(next_state);
            states_.insert(next_state);
          }
        }
      }
    }

    return states_;
  }
};

} // namespace nfa
