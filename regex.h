#include <memory>
#include <regex>
#include <stdexcept>

#include "lib/cpp_utils/hlj/hlj.h"

using namespace hlj;

struct Pipe {};
struct Asterisk {};
struct LParenthesis {};
struct RParenthesis {};

using Token = OneOf<Pipe, Asterisk, LParenthesis, RParenthesis, char>;

inline String repr(const Pipe &value) { return "'|'"; }
inline String repr(const Asterisk &value) { return "'*'"; }
inline String repr(const LParenthesis &value) { return "'('"; }
inline String repr(const RParenthesis &value) { return "')'"; }

using TokenStream = List<Token>;

inline TokenStream lex(const String &source) {
  const size_t n = source.size();
  size_t i = 0;
  TokenStream r;
  while (i < n) {
    while (i < n && std::isspace(source[i])) {
      i++;
    }

    if (i >= n) {
      break;
    }

    switch (source[i]) {
    case '|':
      r.push_back({Pipe{}});
      break;
    case '*':
      r.push_back({Asterisk{}});
      break;
    case '(':
      r.push_back({LParenthesis{}});
      break;
    case ')':
      r.push_back({RParenthesis{}});
      break;
    default:
      r.push_back({source[i]});
      break;
    }

    i++;
  }
  return r;
}

struct Regex;
struct Quantifier {};
struct UnionExpr0;
struct ConcatExpr0;
struct RepeatExpr0;

using BaseExpr = OneOf<Regex, char>;
using RepeatExpr = OneOf<RepeatExpr0, BaseExpr>;
using ConcatExpr = OneOf<ConcatExpr0, RepeatExpr>;
using UnionExpr = OneOf<UnionExpr0, ConcatExpr>;

struct Regex {
  UnionExpr expr;
};

struct UnionExpr0 {
  ConcatExpr first;
  UnionExpr rest;
};

struct ConcatExpr0 {
  RepeatExpr first;
  ConcatExpr rest;
};

struct RepeatExpr0 {
  BaseExpr expr;
  Quantifier quantifier;
};

static constexpr struct Error {
} error;

template <typename T> struct Result : OneOf<Error, T> {
  using Base = OneOf<Error, T>;

  Result(Error value) : Base{value} {}

  Result(const T &value) : Base{value} {}

  operator bool() const { return Base::template is<T>(); }

  const T &operator*() const {
    if (!*this) {
      throw std::invalid_argument("cannot get result value from error");
    } else {
      return Base::template as<T>();
    }
  }

  const T *operator->() const { return &Base::template as<T>(); }
};

/*
 * Regex -> UnionExpr
 * UnionExpr -> ConcatExpr "|" UnionExpr | ConcatExpr
 * ConcatExpr -> RepeatExpr ConcatExpr | RepeatExpr
 * RepeatExpr -> BaseExpr "*" | BaseExpr
 * BaseExpr -> "(" Regex ")" | char
 */

class Parser {
public:
  Parser(const TokenStream &token_stream_) : token_stream(token_stream_) {}

  Result<Regex> parse() {
    auto regex = parse_regex();
    if (idx != token_stream.size()) {
      return error;
    }
    return regex;
  }

private:
  const TokenStream &token_stream;
  uint idx = 0;

  Result<Regex> parse_regex();
  Result<UnionExpr> parse_union_expr();
  Result<ConcatExpr> parse_concat_expr();
  Result<RepeatExpr> parse_repeat_expr();
  Result<BaseExpr> parse_base_expr();

  class Checkpoint {
  public:
    Checkpoint(uint &idx_) : idx(idx_), checkpoint(idx_) {}

    void discard() { discarded = true; }

    ~Checkpoint() {
      if (!discarded) {
        idx = checkpoint;
      }
    }

  private:
    uint &idx;
    const uint checkpoint;
    bool discarded = false;
  };

  inline Result<Token> consume() {
    if (idx < token_stream.size()) {
      return token_stream[idx++];
    } else {
      return error;
    }
  }

  template <typename T> inline Result<T> expect() {
    Checkpoint checkpoint(idx);
    auto token = consume();
    if (!token || !token->is<T>()) {
      return error;
    }
    checkpoint.discard();
    return token->as<T>();
  }
};

inline Result<Regex> Parser::parse_regex() {
  auto expr = parse_union_expr();
  if (!expr) {
    return error;
  }
  return Regex{*expr};
}

inline Result<UnionExpr> Parser::parse_union_expr() {
  auto first = parse_concat_expr();
  if (!first) {
    return error;
  }

  if (!expect<Pipe>()) {
    return UnionExpr{*first};
  }

  auto rest = parse_union_expr();
  if (!rest) {
    return error;
  }

  return {UnionExpr0{*first, *rest}};
}

inline Result<ConcatExpr> Parser::parse_concat_expr() {
  auto first = parse_repeat_expr();
  if (!first) {
    return error;
  }

  auto rest = parse_concat_expr();
  return rest ? ConcatExpr{ConcatExpr0{*first, *rest}} : ConcatExpr{*first};
}

inline Result<RepeatExpr> Parser::parse_repeat_expr() {
  auto expr = parse_base_expr();
  if (!expr) {
    return error;
  }

  return expect<Asterisk>() ? RepeatExpr{RepeatExpr0{*expr}}
                            : RepeatExpr{*expr};
}

inline Result<BaseExpr> Parser::parse_base_expr() {
  Checkpoint checkpoint{idx};

  if (expect<LParenthesis>()) {
    Result<Regex> regex = parse_regex();
    if (!regex || !expect<RParenthesis>()) {
      return error;
    }
    checkpoint.discard();
    return {*regex};
  } else {
    auto token = expect<char>();
    if (!token) {
      return error;
    }
    checkpoint.discard();
    return {*token};
  }
}
