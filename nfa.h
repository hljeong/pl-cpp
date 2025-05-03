#pragma once

#include <cassert>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "lib/cpp_utils/cpy/cpy.h"
#include "lib/cpp_utils/fmt/fmt.h"

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
  friend std::string repr(const Symbol &value);

private:
  std::optional<Epsilon> e{std::nullopt};
  std::optional<char> c{std::nullopt};
};

class Nfa {
public:
  class State {
  public:
    State() : id(next_id++) {}
    bool operator<(State other) const { return id < other.id; }
    bool operator==(State other) const { return id == other.id; }
    friend std::string repr(const State &value);

  private:
    static uint next_id;
    uint id;
  };

  using Alphabet = std::set<char>;
  using StateTransition = std::map<Symbol, std::set<State>>;
  using Transition = std::map<State, StateTransition>;

  using TQ = std::set<State>;
  using Tq0 = State;
  using TS = Alphabet;
  using Td = Transition;
  using TF = std::set<State>;

  using CTQr = const std::set<State> &;
  using CTq0r = const State &;
  using CTSr = const Alphabet &;
  using CTdr = const Transition &;
  using CTFr = const std::set<State> &;

  Nfa(CTQr Q_, CTq0r q0_, CTSr S_, CTdr d_, CTFr F_);
  bool accepts(const std::string &s) const;
  operator std::tuple<CTQr, CTq0r, CTSr, CTdr, CTFr>() const;
  Nfa operator+(const Nfa &other) const;
  Nfa operator|(const Nfa &other) const;
  Nfa operator*() const;
  friend std::string repr(const Nfa &value);

private:
  TQ Q;
  Tq0 q0;
  TS S;
  Td d;
  TF F;

  std::set<State> reachable(std::set<State> states) const;
};

std::string repr(const Symbol &value);
std::string repr(const Nfa::State &value);
std::string repr(const Nfa &value);

} // namespace nfa
