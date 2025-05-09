#pragma once

#include <memory>
#include <type_traits>

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

// struct B {
//   virtual ~B() = 0;
//   virtual UPtr<B> copy() const = 0;
//   virtual const String &get_type_name() const = 0;
//   virtual String get_repr() const = 0;
// };
//
// inline B::~B() {}
//
// template <typename T> struct D : B {
//   const T value;
//   static const String type_name;
//
//   D(const T &value_) : value{value_} {}
//   ~D() override = default;
//
//   UPtr<B> copy() const final { return make_unique<D<T>>(value); }
//
//   const String &get_type_name() const final { return type_name; }
//
//   String get_repr() const final {
//     if constexpr (reprable<T>) {
//       return repr(value);
//     } else {
//       return format("<unreprable {}>", type_name);
//     }
//   }
// };
//
// template <typename T> inline const String D<T>::type_name = ::type_name<T>;

// struct Whatever {
//   UPtr<B> store;
//
//   Whatever() : store{nullptr} {}
//
//   template <typename T>
//   Whatever(const T &value) : store{make_unique<D<std::decay_t<T>>>(value)} {}
//
//   Whatever(const Whatever &other)
//       : store{other.store ? other.store->copy() : nullptr} {}
//
//   Whatever &operator=(const Whatever &other) {
//     store = other.store ? other.store->copy() : nullptr;
//     return *this;
//   }
//
//   Whatever(Whatever &&other) : store{std::move(other.store)} {}
//
//   Whatever &operator=(Whatever &&other) {
//     store = std::move(other.store);
//     return *this;
//   }
//
//   template <typename T> const T &as() const {
//     return reinterpret_cast<D<T> *>(store.get())->value;
//   }
//
//   String repr() const {
//     return store ? format("{} {}", store->get_type_name(), store->get_repr())
//                  : "nothing";
//   }
// };

template <typename T> String always_repr(const T &value) {
  if constexpr (reprable<T>) {
    return repr(value);
  } else {
    return format("<unreprable {}>", type_name<T>);
  }
}

class Whatever {
public:
  Whatever() : store{nullptr} {}

  template <typename T>
  Whatever(const T &value)
      : store{make_unique<Concrete<std::decay_t<T>>>(value)} {}

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
    return reinterpret_cast<Concrete<T> *>(store.get())->value;
  }

  String repr() const {
    return store ? format("{} {}", store->get_type_name(), store->repr())
                 : "nothing";
  }

private:
  struct Store {
    virtual ~Store() = 0;
    virtual UPtr<Store> copy() const = 0;
    virtual const String &get_type_name() const = 0;
    virtual String repr() const = 0;
  };

  template <typename T> struct Concrete : Store {
    const T value;
    static const String type_name;

    Concrete(const T &value_) : value{value_} {}
    ~Concrete() override = default;

    UPtr<Store> copy() const final { return make_unique<Concrete<T>>(value); }

    const String &get_type_name() const final { return type_name; }

    String repr() const final { return always_repr(value); }
  };

  UPtr<Store> store;
};

inline Whatever::Store::~Store() {};

template <typename T>
inline const String Whatever::Concrete<T>::type_name = ::type_name<T>;

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
