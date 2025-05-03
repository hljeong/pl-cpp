#include "nfa.h"
#include "lib/cpp_utils/cpy/cpy.h"
#include "lib/cpp_utils/fmt/fmt.h"

using namespace cpy;
using namespace fmt;
using namespace nfa;

uint Nfa::State::next_id = 0;

template <> bool Symbol::is<Symbol::Epsilon>() const { return e.has_value(); }

template <> bool Symbol::is<char>() const { return c.has_value(); }

bool Symbol::operator<(const Symbol &other) const {
  if (is<Epsilon>()) {
    return !other.is<Epsilon>();
  } else {
    return c < other.c;
  }
}

// todo: defining casting to std::optional<T> and then defineing is<T>() to be
// static_cast<std::optional<T>>(*this).has_value() doesnt work. static_cast-ing
// outside also shows has_value() is true -- why?
String nfa::repr(const Symbol &value) {
  return value.is<Symbol::Epsilon>() ? "nfa::Symbol::Epsilon"
                                     : fmt::repr(static_cast<char>(value));
}

String nfa::repr(const Nfa::State &value) { return format("q{}", value.id); }

Nfa::Nfa(CTQr Q_, CTq0r q0_, CTSr S_, CTdr d_, CTFr F_)
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

bool Nfa::accepts(const String &s) const {
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

Nfa::operator Tuple<CTQr, CTq0r, CTSr, CTdr, CTFr>() const {
  return {Q, q0, S, d, F};
}

// concatenation
Nfa Nfa::operator+(const Nfa &other) const {
  const auto &[Q_, q0_, S_, d_, F_] = other;
  assert(disjoint(Q, Q_));

  auto d__ = combine(d, d_);
  for (const auto &state : F) {
    d__[state][Symbol::epsilon].insert(q0_);
  }

  return {combine(Q, Q_), q0, combine(S, S_), d__, F_};
}

// disjunction
Nfa Nfa::operator|(const Nfa &other) const {
  const auto &[Q_, q0_, S_, d_, F_] = other;
  assert(disjoint(Q, Q_));

  Nfa::State q0__;
  return {combine<TQ>({q0__}, Q, Q_), q0__, combine(S, S_),
          combine<Td>({{q0__, {{Symbol::epsilon, {q0, q0_}}}}}, d, d_),
          combine(F, F_)};
}

// kleene star
Nfa Nfa::operator*() const {
  State q0_;
  auto d_ = d;
  d_[q0_][Symbol::epsilon].insert(q0);
  for (const auto &state : F) {
    d_[state][Symbol::epsilon].insert(q0);
  }

  return {combine(Q, {q0_}), q0_, S, d_, combine(F, {q0_})};
}

String nfa::repr(const Nfa &value) {
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

Set<Nfa::State> Nfa::reachable(Set<State> states) const {
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
