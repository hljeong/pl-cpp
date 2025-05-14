#include <memory>
#include <stdexcept>

#include "lib/cpp_utils/hlj/hlj.h"

using namespace hlj;

struct Pipe {};
struct Asterisk {};
struct LParenthesis {};
struct RParenthesis {};

struct Token : OneOf<Pipe, Asterisk, LParenthesis, RParenthesis, char> {
  Index position;
};

inline String repr(const Pipe &value) { return "'|'"; }
inline String repr(const Asterisk &value) { return "'*'"; }
inline String repr(const LParenthesis &value) { return "'('"; }
inline String repr(const RParenthesis &value) { return "')'"; }

struct TokenStream : List<Token> {
  const String source;
};

inline TokenStream lex(const String &source) {
  const size_t n = source.size();
  size_t i = 0;
  TokenStream r{.source = source};
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

template <typename T, typename E> struct Result : OneOf<T, E> {
  using Base = OneOf<T, E>;

  Result(const T &value) : Base{value} {}

  Result(const E &value) : Base{value} {}

  bool ok() const { return Base::template is<T>(); }

  const T &operator*() const {
    if (!ok()) {
      throw std::invalid_argument("cannot get result value from error");
    } else {
      return Base::template as<T>();
    }
  }

  const T *operator->() const {
    if (!ok()) {
      throw std::invalid_argument("cannot get result value from error");
    } else {
      return &Base::template as<T>();
    }
  }

  const E &error() const {
    if (ok()) {
      throw std::invalid_argument("cannot get error from valid result");
    } else {
      return Base::template as<E>();
    }
  }
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
  struct Error {
    const String &source;
    Index position;
    String message;

    String repr() const {
      return join(
          "\n",
          {message.size() ? format("parse error: {}", message) : "parse error",
           indent(join("\n",
                       {source, indent("^", {.stop = position, .size = 1})}))});
    }
  };

  template <typename T> using Result = ::Result<T, Error>;

  Parser(const TokenStream &token_stream_) : token_stream(token_stream_) {}

  Result<Regex> parse() {
    auto regex = parse_regex();
    if (!regex.ok()) {
      return regex.error();
    }
    if (idx != token_stream.size()) {
      return Error{token_stream.source, idx, "did not parse to end"};
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

  inline Error error(const String &message = "") {
    return {.source = token_stream.source, .position = idx, .message = message};
  }

  inline Result<Token> consume() {
    if (idx < token_stream.size()) {
      return token_stream[idx++];
    } else {
      return error("expected token here");
    }
  }

  template <typename T> Result<T> expect() {
    Checkpoint checkpoint(idx);
    auto token = consume();
    if (!token.ok()) {
      return token.error();
    }
    if (!token->is<T>()) {
      // todo: ew
      checkpoint.~Checkpoint();
      return error(
          format("expected {}, got {}", type_name<T>, token->type_name()));
    }
    checkpoint.discard();
    return token->as<T>();
  }
};

inline Parser::Result<Regex> Parser::parse_regex() {
  auto expr = parse_union_expr();
  if (!expr.ok()) {
    return expr.error();
  }
  return Regex{*expr};
}

inline Parser::Result<UnionExpr> Parser::parse_union_expr() {
  auto first = parse_concat_expr();
  if (!first.ok()) {
    return first.error();
  }

  if (!expect<Pipe>().ok()) {
    return UnionExpr{*first};
  }

  auto rest = parse_union_expr();
  if (!rest.ok()) {
    return rest.error();
  }

  return {UnionExpr0{*first, *rest}};
}

inline Parser::Result<ConcatExpr> Parser::parse_concat_expr() {
  auto first = parse_repeat_expr();
  if (!first.ok()) {
    return first.error();
  }

  auto rest = parse_concat_expr();
  return rest.ok() ? ConcatExpr{ConcatExpr0{*first, *rest}}
                   : ConcatExpr{*first};
}

inline Parser::Result<RepeatExpr> Parser::parse_repeat_expr() {
  auto expr = parse_base_expr();
  if (!expr.ok()) {
    return expr.error();
  }

  return expect<Asterisk>().ok() ? RepeatExpr{RepeatExpr0{*expr}}
                                 : RepeatExpr{*expr};
}

inline Parser::Result<BaseExpr> Parser::parse_base_expr() {
  Checkpoint checkpoint{idx};

  if (expect<LParenthesis>().ok()) {
    Result<Regex> regex = parse_regex();
    if (!regex.ok()) {
      return regex.error();
    }
    auto rparen = expect<RParenthesis>();
    if (!rparen.ok()) {
      return rparen.error();
    }
    checkpoint.discard();
    return {*regex};
  } else {
    auto token = expect<char>();
    if (!token.ok()) {
      return token.error();
    }
    checkpoint.discard();
    return {*token};
  }
}

inline Parser::Result<Regex> parse(const TokenStream &token_stream) {
  return Parser(token_stream).parse();
}
