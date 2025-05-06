// "sum type"

#ifndef SUM_H
#define SUM_H

#include <cstring>
#include <memory>
#include <stdexcept>
#include <variant>

#include "lib/cpp_utils/cpy/cpy.h"
#include "lib/cpp_utils/fmt/fmt.h"

using namespace cpy;
using namespace fmt;

namespace sum {

// template <typename... Ts> struct OneOf;
//
// template <typename T, typename... Ts> struct OneOf<T, Ts...> : OneOf<Ts...> {
//   std::unique_ptr<T> value;
//   const bool is_T;
//
//   OneOf() : value(nullptr), is_T(false) {
//     // print("discard constructof for {}, {} types left", type_name<T>(),
//     // sizeof...(Ts));
//   }
//
//   OneOf(const T &value_)
//       : OneOf<Ts...>(), value(std::make_unique<T>(value_)), is_T(true) {
//     // print("matching construtor for {}, {} types left", type_name<T>(),
//     //       sizeof...(Ts));
//   }
//
//   template <typename U>
//   OneOf(const U &value_) : OneOf<Ts...>(value_), value(nullptr), is_T(false)
//   {
//     // print("passed in {} => non-matching construtor for {}, {} types left",
//     //       type_name<U>(), type_name<T>(), sizeof...(Ts));
//   }
//
//   template <typename U> bool is() const {
//     // print("inside is<{}>: std::is_same_v<{}, {}> => {}, is_T = {}",
//     //       type_name<U>(), type_name<U>(), type_name<T>(),
//     std::is_same_v<U,
//     //       T>, is_T);
//     if constexpr (std::is_same_v<U, T>) {
//       return is_T;
//     } else {
//       return OneOf<Ts...>::template is<U>();
//     }
//   };
//
//   template <typename U> const U &get() const {
//     if (!is<U>()) {
//       throw std::invalid_argument(
//           format("cannot get<{}>(): variant does not hold {}",
//           type_name<U>(),
//                  type_name<U>()));
//     }
//     if constexpr (std::is_same_v<U, T>) {
//       return *value;
//     } else {
//       return OneOf<Ts...>::template get<U>();
//     }
//   }
// };
//
// template <> struct OneOf<> {
//   OneOf() {}
//
//   // todo: error at compile time?
//   template <typename T> OneOf(const T &) {
//     throw std::invalid_argument(
//         format("attempting to construct variant from {} which is not one of "
//                "the types of the variant",
//                type_name<T>()));
//   }
// };
//
// template <typename T, typename... Ts>
// inline String repr(const OneOf<T, Ts...> &value) {
//   // print("testing {} for {}: {}", type_name<decltype(value)>(),
//   // type_name<T>(),
//   //       value.template is<T>());
//   if (value.template is<T>()) {
//     return format("{} {}", type_name<T>(), value.template get<T>());
//   } else {
//     return repr(static_cast<const OneOf<Ts...> &>(value));
//   }
// }
//
// inline String repr(const OneOf<> &value) { return "nothing"; }

using byte = uint8_t;

template <size_t I, typename... Ts> struct nth_t;

template <typename T, typename... Ts> struct nth_t<0, T, Ts...> {
  using type = T;
};

template <size_t I, typename T, typename... Ts>
struct nth_t<I, T, Ts...> : nth_t<I - 1, Ts...> {};

constexpr struct Discard {
} discard;

template <typename... Ts> struct OneOfInternal;

template <> struct OneOfInternal<> {
  template <typename T> OneOfInternal(const T &, Discard) {}

  OneOfInternal(const OneOfInternal &, const byte *) {}

  // intentionally omit OneOfInternal(const T &), is<T>(), as<T>() operator==(),
  // operator<()

  String repr() const { return "nothing"; }
};

template <typename T, typename... Ts>
struct OneOfInternal<T, Ts...> : OneOfInternal<Ts...> {
  using Base = OneOfInternal<Ts...>;

  const T &value;
  bool valid;

  OneOfInternal(const T &value_)
      : Base(value_, discard), value(value_), valid(true) {
    // print("[{}] => {}: {}", sizeof...(Ts), type_name<T>(), repr(value));
  }

  template <typename U>
  OneOfInternal(const U &value_)
      : Base(value_), value(*reinterpret_cast<const T *>(&value_)),
        valid(false) {
    // print("[{}] =/> {}", sizeof...(Ts), type_name<T>());
  }

  template <typename U, std::enable_if_t<!std::is_same_v<U, T>, bool> = true>
  OneOfInternal(const U &value_, Discard)
      : Base(value_, discard), value(*reinterpret_cast<const T *>(&value)),
        valid(false) {
    // print("[{}] => discard", sizeof...(Ts));
  }

  OneOfInternal(const OneOfInternal &) = delete;

  OneOfInternal(const OneOfInternal &other, const byte *value_)
      : Base(other, value_), value(*reinterpret_cast<const T *>(&value_)),
        valid(other.valid) {}

  size_t index() const { return valid ? 0 : 1 + Base::index(); }

  template <typename U> bool is() const {
    // print("inside is<{}>: std::is_same_v<{}, {}> => {}, is_T = {}",
    //       type_name<U>(), type_name<U>(), type_name<T>(), std::is_same_v<U,
    //       T>, valid);
    if constexpr (std::is_same_v<U, T>) {
      return valid;
    } else {
      return Base::template is<U>();
    }
  };

  template <typename U> const U &as() const {
    if constexpr (std::is_same_v<U, T>) {
      if (!valid) {
        throw std::invalid_argument(
            format("cannot extract {}", type_name<U>()));
      }
      return value;
    } else {
      return Base::template as<U>();
    }
  }

  template <typename U> bool operator==(const U &other) const {
    if constexpr (std::is_same_v<U, T>) {
      return valid && (value == other);
    } else {
      return Base::operator==(other);
    }
  }

  template <typename U> bool operator<(const U &other) const {
    if constexpr (std::is_same_v<U, T>) {
      return valid && (value < other);
    } else {
      return Base::operator<(other);
    }
  }

  String repr() const {
    if (valid) {
      return format("{} {}", type_name<T>(), value);
    } else {
      return Base::repr();
    }
  }

  // template <typename U> operator const U &() const {
  //   if constexpr (std::is_same_v<U, T>) {
  //     if (!valid) {
  //       throw std::invalid_argument(
  //           format("cannot cast to {}", type_name<U>()));
  //     }
  //     return value;
  //   } else {
  //     return static_cast<const U &>(static_cast<const Base &>(*this));
  //   }
  // }
};

template <typename... Ts> struct OneOf {
  // static constexpr uint N = (sizeof(Ts) | ...); // poor man's max
  // might have to change this to dynamically allocated if N cannot be resolved
  // inside circular dependency
  // byte value[N]{0};
  size_t size;
  byte *value;
  OneOfInternal<Ts...> internal;

  template <typename T>
  OneOf(const T &value_)
      : size(sizeof(T)), value(new byte[size]),
        internal(*reinterpret_cast<const T *>(value)) {
    new (value) T{value_};
  }

  OneOf(const OneOf<Ts...> &other)
      : size(other.size), value(new byte[size]),
        internal(other.internal, value) {}

  OneOf(OneOf<Ts...> &&) = delete;

  ~OneOf() { delete[] value; }

  size_t index() const { return internal.index(); }

  template <typename T> bool is() const { return internal.template is<T>(); }

  template <typename T> const T &as() const {
    return internal.template as<T>();
  }

  template <typename T> bool operator==(const T &other) const {
    return internal == other;
  }

  template <typename T> bool operator<(const T &other) const {
    return internal == other;
  }

  String repr() const { return internal.repr(); }

  // template <typename T> operator const T &() const {
  //   return static_cast<const T &>(internal);
  // }
};

template <typename... Ts> inline String repr(const OneOf<Ts...> &value) {
  return value.repr();
}

} // namespace sum

#endif
