#pragma once

#include <memory>
#include <type_traits>

// #include "lib/cpp_utils/cpy/cpy.h"
// #include "lib/cpp_utils/fmt/fmt.h"
//
// using namespace cpy;
// using namespace fmt;

#include "lib/cpp_utils/hlj/hlj.h"

using namespace hlj;

namespace sum {

template <typename T, typename... Ts> struct index_t;

template <typename T, typename... Ts> struct index_t<T, T, Ts...> {
  static constexpr size_t value = 0;
};

template <typename T, typename U, typename... Ts> struct index_t<T, U, Ts...> {
  static constexpr size_t value = 1 + index_t<T, Ts...>::value;
};

template <typename T, typename... Ts>
static constexpr size_t index = index_t<T, Ts...>::value;

// template <typename F, typename... Ts, size_t... Is>
// auto map_(F f, const Tuple<Ts...> &t, std::index_sequence<Is...>) {
//   return std::make_tuple(
//       f(std::get<Is>(std::forward<const Tuple<Ts...> &>(t)))...);
// }
//
// template <typename F, typename... Ts> auto map(F f, const Tuple<Ts...> &t) {
//   return map_(std::forward<F>(f), std::forward<const Tuple<Ts...> &>(t),
//               std::make_index_sequence<sizeof...(Ts)>());
// }
//
// template <typename T> std::unique_ptr<T> copy(const std::unique_ptr<T> &p) {
//   return p ? std::make_unique<T>(*p) : nullptr;
// }

// static constexpr bool debug_print = true;
//
// template <typename T> void *copy(const void *value) {
//   void *p = new T{*reinterpret_cast<const T *>(value)};
//   if (debug_print) {
//     print("copied to {} {} @ {}", type_name<T>(),
//           repr(*reinterpret_cast<T *>(p)), long(p));
//   }
//   return p;
// }
//
// template <typename T> void *move(const void *value) {
//   void *p = new T{std::move(*reinterpret_cast<const T *>(value))};
//   if (debug_print) {
//     print("moved to {} {} @ {}", type_name<T>(),
//           repr(*reinterpret_cast<T *>(p)), long(p));
//   }
//   return p;
// }
//
// template <typename T> void destroy(void *value) {
//   if (debug_print) {
//     print("destroying {} {} @ {}", type_name<T>(),
//           repr(*reinterpret_cast<T *>(value)), long(value));
//   }
//   delete reinterpret_cast<T *>(value);
//   value = nullptr;
//   if (debug_print) {
//     print("done destroying");
//   }
// }
//
// template <typename... Ts> class OneOf {
// public:
//   template <typename T>
//   OneOf(const T &value)
//       : copy{sum::copy<T>}, move{sum::move<T>}, destroy{sum::destroy<T>},
//         repr_{[](void *p) {
//           using fmt::repr;
//           return repr(*reinterpret_cast<T *>(p));
//         }},
//         which{index<T, Ts...>}, store{copy(&value)} {
//     if (debug_print) {
//       print("value construct");
//     }
//   }
//
//   OneOf(const OneOf &value)
//       : copy{value.copy}, move{value.move}, destroy{value.destroy},
//         repr_{value.repr_}, which{value.which}, store{copy(value.store)} {
//     if (debug_print) {
//       print("copy construct");
//     }
//   }
//
//   template <typename T> OneOf &operator=(const T &other) {
//     if (debug_print) {
//       print("value copy assign");
//     }
//     destroy(store);
//
//     copy = sum::copy<T>;
//     move = sum::move<T>;
//     destroy = sum::destroy<T>;
//     repr_ = [](void *p) {
//       using fmt::repr;
//       return repr(*reinterpret_cast<T *>(p));
//     };
//     which = index<T, Ts...>;
//     store = copy(other.store);
//   }
//
//   OneOf &operator=(const OneOf &other) {
//     if (debug_print) {
//       print("copy assign");
//     }
//     destroy(store);
//
//     copy = other.copy;
//     move = other.move;
//     destroy = other.destroy;
//     repr_ = other.repr_;
//     which = other.which;
//     store = copy(other.store);
//
//     return *this;
//   }
//
//   OneOf(OneOf &&value)
//       : copy{value.copy}, move{value.move}, destroy{value.destroy},
//         repr_{value.repr_}, which{value.which}, store{move(value.store)} {
//     if (debug_print) {
//       print("move construct");
//     }
//     value.destroy(value.store);
//     value.which = N;
//     value.destroy = [](auto) {};
//     value.move = [](auto) { return nullptr; };
//     value.copy = [](auto) { return nullptr; };
//   }
//
//   template <typename T> OneOf &operator=(T &&other) {
//     if (debug_print) {
//       print("value move assign");
//     }
//     destroy(store);
//
//     copy = sum::copy<T>;
//     move = sum::move<T>;
//     destroy = sum::destroy<T>;
//     repr_ = [](void *p) {
//       using fmt::repr;
//       return repr(*reinterpret_cast<T *>(p));
//     };
//     which = index<T, Ts...>;
//     store = move(other.store);
//   }
//
//   OneOf &operator=(OneOf &&other) {
//     if (debug_print) {
//       print("move assign");
//     }
//     destroy(store);
//
//     copy = other.copy;
//     move = other.move;
//     destroy = other.destroy;
//     repr_ = other.repr_;
//     which = other.which;
//     store = move(other.store);
//
//     other.destroy(other.store);
//     other.which = N;
//     other.destroy = [](auto) {};
//     other.move = [](auto) { return nullptr; };
//     other.copy = [](auto) { return nullptr; };
//
//     return *this;
//   }
//
//   ~OneOf() {
//     if (debug_print) {
//       print("destructor ({})", which == N ? "nothing" : ::repr(which));
//     }
//     destroy(store);
//   }
//
//   template <typename T> bool is() const { return which == index<T, Ts...>; }
//
//   template <typename T> const T &as() const {
//     if (!is<T>()) {
//       throw std::invalid_argument(
//           format("cannot as<{}>()", type_name<T>(), type_name<T>()));
//     } else {
//       return *reinterpret_cast<const T *>(store);
//     }
//   }
//
//   template <typename T> bool operator==(const T &other) const {
//     return (which == index<T, Ts...>) && (as<T>() == other);
//   }
//
//   template <typename T> bool operator<(const T &other) const {
//     return (which < index<T, Ts...>) ||
//            ((which == index<T, Ts...>) && (as<T>() < other));
//   }
//
//   String repr() const { return repr_(store); }
//
// private:
//   static constexpr size_t N = sizeof...(Ts);
//   // using Store = std::aligned_union_t<N, Ts...>;
//
//   std::function<void *(const void *)> copy;
//   std::function<void *(const void *)> move;
//   std::function<void(void *)> destroy;
//   std::function<String(void *)> repr_;
//
//   // const static std::array<std::function<void *(const void *)>, N> copy = {
//   //     sum::copy<Ts>...,
//   // };
//   //
//   // const static std::array<std::function<void *(const void *)>, N> move = {
//   //     sum::move<Ts>...,
//   // };
//   //
//   // const static std::array<std::function<void(const void *)>, N> destroy =
//   {
//   //     sum::destroy<Ts>...,
//   // };
//
//   // const std::array<std::function<void(const void *)>, N + 1> copy = {
//   //     [this](const void *value) {
//   //       print("start copying");
//   //       // print("copying {} {}", type_name<Ts>(),
//   //       //       repr(*reinterpret_cast<const Ts *>(value)));
//   //       store = new Ts{*reinterpret_cast<const Ts *>(value)};
//   //       // print("result: {}", repr(*reinterpret_cast<const Ts
//   //       // *>(store)));
//   //       print("done copying");
//   //     }...,
//   //     [](auto) { print("copying nothing"); }};
//   //
//   // const std::array<std::function<void(const void *)>, N + 1> move = {
//   //     [this](const void *value) {
//   //       print("moving {}", type_name<Ts>());
//   //       store = new Ts{std::move(*reinterpret_cast<const Ts *>(value))};
//   //       print("result: {}", repr(*reinterpret_cast<const Ts *>(store)));
//   //     }...,
//   //     [](auto) { print("moving nothing"); }};
//   //
//   // const std::array<std::function<void()>, N + 1> destroy = {
//   //     [this]() {
//   //       print("destroying {}", type_name<Ts>);
//   //       delete reinterpret_cast<Ts *>(store);
//   //       store = nullptr;
//   //     }...,
//   //     []() { print("destroying nothing"); }};
//
//   size_t which = N;
//   void *store = nullptr;
//   // Store store;
//   //
//   // const std::vector<std::function<void(const void *)>> copy = {
//   //     [this](const void *value) {
//   //       new (&store) Ts{*reinterpret_cast<const Ts *>(value)};
//   //     }...,
//   //     [](auto) {}};
//   //
//   // const std::vector<std::function<void(const void *)>> move = {
//   //     [this](const void *value) {
//   //       *reinterpret_cast<Ts *>(&store) =
//   //           std::move(*reinterpret_cast<const Ts *>(value));
//   //     }...,
//   //     [](auto) {}};
//   //
//   // const std::vector<std::function<void()>> destroy = {
//   //     [this]() { reinterpret_cast<Ts *>(&store)->~Ts(); }..., []() {}};
// };

// template <typename... Ts> String repr(const OneOf<Ts...> &value) {
//   return value.repr();
// }

using ::repr;

template <typename T, typename> struct has_repr_t : std::false_type {};

template <typename T>
struct has_repr_t<T, std::void_t<decltype(std::declval<T>().repr())>>
    : std::is_same<decltype(std::declval<T>().repr()), String> {};

template <typename T> constexpr bool has_repr = has_repr_t<T, void>::value;

template <typename T, std::enable_if_t<has_repr<T>, bool> = true>
String repr(const T &value) {
  return value.repr();
}

template <typename T, typename> struct reprable_t : std::false_type {
  static const String wtf;
};

template <typename T>
struct reprable_t<T, std::void_t<decltype(repr(std::declval<T>()))>>
    : std::is_same<decltype(repr(std::declval<T>())), String> {};

template <typename T> constexpr bool reprable = reprable_t<T, void>::value;

struct B {
  virtual ~B() = 0;
  virtual UPtr<B> copy() const = 0;
  virtual const String &get_type_name() const = 0;
  virtual String get_repr() const = 0;
};

inline B::~B() {}

template <typename T> struct D : B {
  const T value;
  static const String type_name;

  D(const T &value_) : value{value_} {}
  ~D() override = default;

  UPtr<B> copy() const final { return make_unique<D<T>>(value); }

  const String &get_type_name() const final { return type_name; }

  String get_repr() const final {
    if constexpr (reprable<T>) {
      return repr(value);
    } else {
      return format("<unreprable {}>", type_name);
    }
  }
};

template <typename T> inline const String D<T>::type_name = ::type_name<T>;

struct Whatever {
  UPtr<B> store;

  Whatever() : store{nullptr} {}

  template <typename T>
  Whatever(const T &value) : store{make_unique<D<std::decay_t<T>>>(value)} {}

  Whatever(const Whatever &other)
      : store{other.store ? other.store->copy() : nullptr} {}

  Whatever &operator=(const Whatever &other) {
    store = other.store ? other.store->copy() : nullptr;
    return *this;
  }

  Whatever(Whatever &&other) : store{std::move(other.store)} {}

  Whatever &operator=(Whatever &&other) {
    store = std::move(other.store);
    return *this;
  }

  template <typename T> const T &as() const {
    return reinterpret_cast<D<T> *>(store.get())->value;
  }

  String repr() const {
    return store ? format("{} {}", store->get_type_name(), store->get_repr())
                 : "nothing";
  }
};

// inline String repr(const Whatever &value) {
//   return value.store ? format("{} {}", value.store->get_type_name(),
//                               value.store->get_repr())
//                      : "nothing";
// }

template <typename... Ts> struct OneOf {
  Whatever store;
  size_t which;

  template <typename T, size_t I = index<T, Ts...>>
  OneOf(const T &value) : store{value}, which{I} {}

  template <typename T, size_t I = index<T, Ts...>> bool is() const {
    return which == I;
  }

  template <typename T, size_t = index<T, Ts...>> const T &as() const {
    return store.as<T>();
  }

  String repr() const { return store.repr(); }
};

} // namespace sum
