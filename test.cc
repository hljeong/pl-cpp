#include "nfa.h"

// using namespace nfa;

#include "regex.h"

struct Nil {};
struct Cons;
struct Lispt : OneOf<Cons, Nil> {};

struct Cons {
  int value;
  Lispt next;
};

String repr(const Nil &) { return "nil"; }

String repr(const Cons &cons) {
  return format("{} -> {}", cons.value, cons.next);
}

String repr(const Lispt &value) {
  print("here");
  Lispt cur = value;
  print("there");
  List<String> r;
  while (cur.is<Cons>()) {
    const Cons &cons = cur.as<Cons>();
    r.push_back(repr(cons.value));
    cur = cons.next;
  }
  return bracket(join(", ", r));
}

// struct S {};
//
// struct T : OneOf<S> {};
//
// static OneOf<S> os{S{}};
//
// static String s = repr(os);
//
// String repr(const S &) { return "S"; }

namespace ns {
struct Choice1;
struct Choice2;
struct S : OneOf<Choice1, Choice2> {};

struct Choice1 {
  int x;
  S s;
};

struct Choice2 {
  int x;
};

// String repr(Choice1 c1) { return format("{} | {}", c1.x, c1.s); }
//
// String repr(Choice2 c2) { return ::repr(c2.x); }
} // namespace ns

int main() {
  ns::S s{ns::Choice1{1, ns::Choice2{2}}};
  printo(s);

  // S s = Choice1{1, Choice2{2}};
  // printo(T{S{}});

  // Nfa::Alphabet S{'a', 'b'};
  // Nfa::State q0, q1;
  // Nfa n1{{q0}, q0, S, {{q0, {{'a', {q0}}}}}, {q0}};
  // Nfa n2{{q1}, q1, S, {{q1, {{'b', {q1}}}}}, {q1}};
  // Nfa n3 = n1 + n2;
  // Nfa n4 = n1 | n2;
  // Nfa n5 = *(n1 + n2);
  //
  // List<String> strings{"", "a", "aa", "ab", "bb", "aabb", "ba", "baa"};
  // List<Pair<Nfa, List<Pair<String, bool>>>> tests;
  // tests.push_back({n1,
  //                  {{"", true},
  //                   {"a", true},
  //                   {"aa", true},
  //                   {"ab", false},
  //                   {"bb", false},
  //                   {"aabb", false},
  //                   {"ba", false},
  //                   {"baa", false}}});
  // tests.push_back({n2,
  //                  {{"", true},
  //                   {"a", false},
  //                   {"aa", false},
  //                   {"ab", false},
  //                   {"bb", true},
  //                   {"aabb", false},
  //                   {"ba", false},
  //                   {"baa", false}}});
  // tests.push_back({n3,
  //                  {{"", true},
  //                   {"a", true},
  //                   {"aa", true},
  //                   {"ab", true},
  //                   {"bb", true},
  //                   {"aabb", true},
  //                   {"ba", false},
  //                   {"baa", false}}});
  // tests.push_back({n4,
  //                  {{"", true},
  //                   {"a", true},
  //                   {"aa", true},
  //                   {"ab", false},
  //                   {"bb", true},
  //                   {"aabb", false},
  //                   {"ba", false},
  //                   {"baa", false}}});
  // tests.push_back({n5,
  //                  {{"", true},
  //                   {"a", true},
  //                   {"aa", true},
  //                   {"ab", true},
  //                   {"bb", true},
  //                   {"aabb", true},
  //                   {"ba", true},
  //                   {"baa", true}}});
  // for (const auto &[idx, nfa_tests] : enumerate(tests)) {
  //   const auto &[nfa, tests_] = nfa_tests;
  //   for (const auto &[s, expected] : tests_) {
  //     bool result = nfa.accepts(s);
  //     if (result != expected) {
  //       print("n{}.accepts({}) => {} (expected: {})", idx + 1, repr(s),
  //       result,
  //             expected);
  //       printo(nfa);
  //       return 1;
  //     }
  //   }
  // }

  // using namespace sum;
  // OneOf<int, char> x = 3;
  // OneOf<int, char> y = 'h';
  // printo(x.is<int>(), x.is<char>());
  // printo(y.is<int>(), y.is<char>());
  // printo(x.as<int>(), y.as<char>());
  // printo(x);

  // Lispt l1{Cons{3, Nil{}}};
  // print("start print");
  // printo(l1);

  Lispt l{Cons{1, Cons{2, Nil{}}}};
  printo(l);
  // OneOf<OneOf<int, char>, bool> x = OneOf<int, char>{1};
  // OneOf<int> x = 3;
  // OneOf<int> y = std::move(x);
  // OneOf<int> z = std::move(x);
  // print("done");

  Whatever x = 3;
  printo(x);

  x = 'c';
  printo(x);

  x = String{"hi"};
  printo(x);

  Whatever y = std::move(x);
  printo(x, y);

  x = Whatever{y};
  printo(x);

  Whatever z;
  printo(z);

  printo(x.as<String>());

  x = 3;
  printo(x.as<int>());

  // struct B {
  //   B(int) {}
  // };
  //
  // struct D : B {};
  //
  // B b = 3;
  // D d = {3};
  //
  // struct S : OneOf<int, char> {};
  //
  // S sx = {3};
  // printo(sx.as<int>());

  // printo(x, y);
  // printo(y.OneOf<char>::is_T);
  // printo(std::is_same_v<decltype(repr(std::declval<char>())), String>);
  // printo(type_name<decltype(repr(std::declval<char>()))>, reprable<char>);
  printo(lex("(a*b*)"));
}
