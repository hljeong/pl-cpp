#pragma once

#include <cassert>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "lib/cpp_utils/hlj/hlj.h"

namespace nfa {

using namespace hlj;

static constexpr struct Epsilon {
} epsilon;

using Symbol = OneOf<Epsilon, char>;

class Nfa {
public:
  class State {
  public:
    State() : id(next_id++) {}
    bool operator<(State other) const { return id < other.id; }
    bool operator==(State other) const { return id == other.id; }
    inline String repr() const { return format("q{}", id); }

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

  using CTQr = const TQ &;
  using CTq0r = const Tq0 &;
  using CTSr = const TS &;
  using CTdr = const Td &;
  using CTFr = const TF &;

  Nfa(CTQr Q_, CTq0r q0_, CTSr S_, CTdr d_, CTFr F_);
  bool accepts(const String &s) const;
  operator Tuple<CTQr, CTq0r, CTSr, CTdr, CTFr>() const;
  Nfa operator+(const Nfa &other) const;
  Nfa operator|(const Nfa &other) const;
  Nfa operator*() const;
  String repr(const Nfa &value);

private:
  TQ Q;
  Tq0 q0;
  TS S;
  Td d;
  TF F;

  Set<State> reachable(Set<State> states) const;
};

} // namespace nfa
