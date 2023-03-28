#include "FOLParser.h"
#include "FOLScanner.h"
#include <queue>
#include <stack>
#include <vector>
#include <map>
#include <functional>
#include <iostream>
using namespace std;

#define PARSER_IS_VERBOSE (false)

namespace FOL {
    namespace {
      template <typename T> T popFrom(stack<T>& s) {
        if (s.empty()) throw runtime_error("Pop from empty stack.");
        T result = std::move(s.top());
        s.pop();
        return result;
      }

      enum class Nonterminal {
        ARGLIST,
    ARGS,
    CALL,
    FORMULA,
    OBJECT,
    OPTDOT,
    _parserInternalStart,

      };

      /* Type representing an item that can be on the parsing stack (either a
       * terminal or a nonterminal).
       */
      struct Symbol {
        bool isToken;
        union {
          TokenType   tokenType;
          Nonterminal nonterminalType;
        };

        Symbol(TokenType t) {
          isToken = true;
          tokenType = t;
        }
        Symbol(Nonterminal t) {
          isToken = false;
          nonterminalType = t;
        }
      };

      bool operator< (const Symbol& lhs, const Symbol& rhs) {
        if (lhs.isToken != rhs.isToken) return lhs.isToken;
        if (lhs.isToken) return lhs.tokenType < rhs.tokenType;
        return lhs.nonterminalType < rhs.nonterminalType;
      }

      /* Type representing auxiliary data to store in each stack entry. */
      struct AuxData {
        std::shared_ptr<ASTNode> field0;
    std::vector<std::shared_ptr<ASTNode>> field1;

      };

      /* Type representing data aggregated so far. */
      struct StackData {
        Token     token;  // Only active if the item is a terminal
        AuxData   data;   // Only active if the item is a nonterminal
      };

      /* Type representing an item on the stack. */
      struct StackItem {
        size_t    state;
        StackData data;
      };

      /* Base type for actions. */
      struct Action {
        virtual ~Action() = default;
      };

      struct ShiftAction: Action {
        size_t target;

        ShiftAction(int target) : target(target) {}
      };

      struct HaltAction: Action {

      };

      struct ReduceAction: Action {
        Nonterminal reduceTo;

        /* Does the reduction. */
        virtual void reduce(stack<StackItem>& s) = 0;

        ReduceAction(Nonterminal t) : reduceTo(t) {}
      };

      /* Helper template functions that fire off callbacks with the arguments
       * expanded out into a list.
       */
      template <size_t N> struct DoReduction {
        template <typename Callback, typename... Args>
        static AuxData invoke(stack<StackItem>& s, Callback c, const Args&... args) {
          /* Build arguments up in reverse order, since items are coming off of the stack! */
          auto nextArg = popFrom(s);
          return DoReduction<N - 1>::invoke(s, c, nextArg.data, args...);
        }
      };
      template <> struct DoReduction<0> {
        template <typename Callback, typename... Args>
        static AuxData invoke(stack<StackItem>&, Callback c, const Args&... args) {
          return c(args...);
        }
      };

      /* Utility metafunction that maps from a number N to a callback type that accepts
       * N arguments.
       */
      template <size_t M> struct CallbackType {
        template <size_t N, typename... Args> struct Helper {
          using type = typename Helper<N - 1, Args..., StackData>::type;
        };
        template <typename... Args> struct Helper<0, Args...> {
          using type = std::function<AuxData (Args...)>;
        };
        using type = typename Helper<M>::type;
      };

      /* Use template system so we know how many arguments to forward. */
      template <size_t N> struct ReduceActionN: ReduceAction {
        typename CallbackType<N>::type callback; // Function to invoke with reduced items

        void reduce(stack<StackItem>& s) override;

        ReduceActionN(Nonterminal n, typename CallbackType<N>::type c) : ReduceAction(n), callback(c) {}
      };

      /* Unused argument type. */
      struct _unused_ {};

      std::shared_ptr<ASTNode> reduce_CALL_from_IDENTIFIER_LPAREN_ARGS_RPAREN(const std::string& _parserArg1, const std::string&, std::vector<std::shared_ptr<ASTNode>> _parserArg3, const std::string&);
  std::shared_ptr<ASTNode> reduce_FORMULA_from_EXISTS_IDENTIFIER_OPTDOT_FORMULA(const std::string&, const std::string& _parserArg2, _unused_, std::shared_ptr<ASTNode> _parserArg4);
  std::shared_ptr<ASTNode> reduce_FORMULA_from_FALSE(const std::string&);
  std::shared_ptr<ASTNode> reduce_FORMULA_from_FORALL_IDENTIFIER_OPTDOT_FORMULA(const std::string&, const std::string& _parserArg2, _unused_, std::shared_ptr<ASTNode> _parserArg4);
  std::shared_ptr<ASTNode> reduce_FORMULA_from_FORMULA_AND_FORMULA(std::shared_ptr<ASTNode> _parserArg1, const std::string&, std::shared_ptr<ASTNode> _parserArg3);
  std::shared_ptr<ASTNode> reduce_FORMULA_from_FORMULA_IFF_FORMULA(std::shared_ptr<ASTNode> _parserArg1, const std::string&, std::shared_ptr<ASTNode> _parserArg3);
  std::shared_ptr<ASTNode> reduce_FORMULA_from_FORMULA_IMPLIES_FORMULA(std::shared_ptr<ASTNode> _parserArg1, const std::string&, std::shared_ptr<ASTNode> _parserArg3);
  std::shared_ptr<ASTNode> reduce_FORMULA_from_FORMULA_OR_FORMULA(std::shared_ptr<ASTNode> _parserArg1, const std::string&, std::shared_ptr<ASTNode> _parserArg3);
  std::shared_ptr<ASTNode> reduce_FORMULA_from_LPAREN_FORMULA_RPAREN(const std::string&, std::shared_ptr<ASTNode> _parserArg2, const std::string&);
  std::shared_ptr<ASTNode> reduce_FORMULA_from_NOT_FORMULA(const std::string&, std::shared_ptr<ASTNode> _parserArg2);
  std::shared_ptr<ASTNode> reduce_FORMULA_from_OBJECT(std::shared_ptr<ASTNode> _parserArg1);
  std::shared_ptr<ASTNode> reduce_FORMULA_from_OBJECT_EQUALS_OBJECT(std::shared_ptr<ASTNode> _parserArg1, const std::string&, std::shared_ptr<ASTNode> _parserArg3);
  std::shared_ptr<ASTNode> reduce_FORMULA_from_OBJECT_NOTEQUALS_OBJECT(std::shared_ptr<ASTNode> _parserArg1, const std::string&, std::shared_ptr<ASTNode> _parserArg3);
  std::shared_ptr<ASTNode> reduce_FORMULA_from_TRUE(const std::string&);
  std::shared_ptr<ASTNode> reduce_OBJECT_from_CALL(std::shared_ptr<ASTNode> _parserArg1);
  std::shared_ptr<ASTNode> reduce_OBJECT_from_IDENTIFIER(const std::string& _parserArg1);
  std::vector<std::shared_ptr<ASTNode>> reduce_ARGLIST_from_OBJECT(std::shared_ptr<ASTNode> _parserArg1);
  std::vector<std::shared_ptr<ASTNode>> reduce_ARGLIST_from_OBJECT_COMMA_ARGLIST(std::shared_ptr<ASTNode> _parserArg1, const std::string&, std::vector<std::shared_ptr<ASTNode>> _parserArg3);
  std::vector<std::shared_ptr<ASTNode>> reduce_ARGS_from();
  std::vector<std::shared_ptr<ASTNode>> reduce_ARGS_from_ARGLIST(std::vector<std::shared_ptr<ASTNode>> _parserArg1);


      AuxData reduce_ARGLIST_from_OBJECT_COMMA_ARGLIST__thunk(StackData a0, StackData a1, StackData a2) {
    AuxData result;
    result.field1 = reduce_ARGLIST_from_OBJECT_COMMA_ARGLIST(a0.data.field0, a1.token.data, a2.data.field1);
    return result;
  }

  AuxData reduce_ARGLIST_from_OBJECT__thunk(StackData a0) {
    AuxData result;
    result.field1 = reduce_ARGLIST_from_OBJECT(a0.data.field0);
    return result;
  }

  AuxData reduce_ARGS_from_ARGLIST__thunk(StackData a0) {
    AuxData result;
    result.field1 = reduce_ARGS_from_ARGLIST(a0.data.field1);
    return result;
  }

  AuxData reduce_ARGS_from__thunk() {
    AuxData result;
    result.field1 = reduce_ARGS_from();
    return result;
  }

  AuxData reduce_CALL_from_IDENTIFIER_LPAREN_ARGS_RPAREN__thunk(StackData a0, StackData a1, StackData a2, StackData a3) {
    AuxData result;
    result.field0 = reduce_CALL_from_IDENTIFIER_LPAREN_ARGS_RPAREN(a0.token.data, a1.token.data, a2.data.field1, a3.token.data);
    return result;
  }

  AuxData reduce_FORMULA_from_EXISTS_IDENTIFIER_OPTDOT_FORMULA__thunk(StackData a0, StackData a1, StackData, StackData a3) {
    AuxData result;
    result.field0 = reduce_FORMULA_from_EXISTS_IDENTIFIER_OPTDOT_FORMULA(a0.token.data, a1.token.data, {}, a3.data.field0);
    return result;
  }

  AuxData reduce_FORMULA_from_FALSE__thunk(StackData a0) {
    AuxData result;
    result.field0 = reduce_FORMULA_from_FALSE(a0.token.data);
    return result;
  }

  AuxData reduce_FORMULA_from_FORALL_IDENTIFIER_OPTDOT_FORMULA__thunk(StackData a0, StackData a1, StackData, StackData a3) {
    AuxData result;
    result.field0 = reduce_FORMULA_from_FORALL_IDENTIFIER_OPTDOT_FORMULA(a0.token.data, a1.token.data, {}, a3.data.field0);
    return result;
  }

  AuxData reduce_FORMULA_from_FORMULA_AND_FORMULA__thunk(StackData a0, StackData a1, StackData a2) {
    AuxData result;
    result.field0 = reduce_FORMULA_from_FORMULA_AND_FORMULA(a0.data.field0, a1.token.data, a2.data.field0);
    return result;
  }

  AuxData reduce_FORMULA_from_FORMULA_IFF_FORMULA__thunk(StackData a0, StackData a1, StackData a2) {
    AuxData result;
    result.field0 = reduce_FORMULA_from_FORMULA_IFF_FORMULA(a0.data.field0, a1.token.data, a2.data.field0);
    return result;
  }

  AuxData reduce_FORMULA_from_FORMULA_IMPLIES_FORMULA__thunk(StackData a0, StackData a1, StackData a2) {
    AuxData result;
    result.field0 = reduce_FORMULA_from_FORMULA_IMPLIES_FORMULA(a0.data.field0, a1.token.data, a2.data.field0);
    return result;
  }

  AuxData reduce_FORMULA_from_FORMULA_OR_FORMULA__thunk(StackData a0, StackData a1, StackData a2) {
    AuxData result;
    result.field0 = reduce_FORMULA_from_FORMULA_OR_FORMULA(a0.data.field0, a1.token.data, a2.data.field0);
    return result;
  }

  AuxData reduce_FORMULA_from_LPAREN_FORMULA_RPAREN__thunk(StackData a0, StackData a1, StackData a2) {
    AuxData result;
    result.field0 = reduce_FORMULA_from_LPAREN_FORMULA_RPAREN(a0.token.data, a1.data.field0, a2.token.data);
    return result;
  }

  AuxData reduce_FORMULA_from_NOT_FORMULA__thunk(StackData a0, StackData a1) {
    AuxData result;
    result.field0 = reduce_FORMULA_from_NOT_FORMULA(a0.token.data, a1.data.field0);
    return result;
  }

  AuxData reduce_FORMULA_from_OBJECT_EQUALS_OBJECT__thunk(StackData a0, StackData a1, StackData a2) {
    AuxData result;
    result.field0 = reduce_FORMULA_from_OBJECT_EQUALS_OBJECT(a0.data.field0, a1.token.data, a2.data.field0);
    return result;
  }

  AuxData reduce_FORMULA_from_OBJECT_NOTEQUALS_OBJECT__thunk(StackData a0, StackData a1, StackData a2) {
    AuxData result;
    result.field0 = reduce_FORMULA_from_OBJECT_NOTEQUALS_OBJECT(a0.data.field0, a1.token.data, a2.data.field0);
    return result;
  }

  AuxData reduce_FORMULA_from_OBJECT__thunk(StackData a0) {
    AuxData result;
    result.field0 = reduce_FORMULA_from_OBJECT(a0.data.field0);
    return result;
  }

  AuxData reduce_FORMULA_from_TRUE__thunk(StackData a0) {
    AuxData result;
    result.field0 = reduce_FORMULA_from_TRUE(a0.token.data);
    return result;
  }

  AuxData reduce_OBJECT_from_CALL__thunk(StackData a0) {
    AuxData result;
    result.field0 = reduce_OBJECT_from_CALL(a0.data.field0);
    return result;
  }

  AuxData reduce_OBJECT_from_IDENTIFIER__thunk(StackData a0) {
    AuxData result;
    result.field0 = reduce_OBJECT_from_IDENTIFIER(a0.token.data);
    return result;
  }

  AuxData reduce_OPTDOT_from_DOT__thunk(StackData) {
    return {};
  }

  AuxData reduce_OPTDOT_from__thunk() {
    return {};
  }



      /* Action table. */
      const vector<map<Symbol, Action*>> kActionTable = {
      {
  {    Nonterminal::CALL, new ShiftAction{9} },
  {    TokenType::EXISTS, new ShiftAction{33} },
  {    TokenType::FALSE, new ShiftAction{32} },
  {    TokenType::FORALL, new ShiftAction{28} },
  {    Nonterminal::FORMULA, new ShiftAction{39} },
  {    TokenType::IDENTIFIER, new ShiftAction{5} },
  {    TokenType::LPAREN, new ShiftAction{17} },
  {    TokenType::NOT, new ShiftAction{16} },
  {    Nonterminal::OBJECT, new ShiftAction{2} },
  {    TokenType::TRUE, new ShiftAction{1} },
},
{
  {    TokenType::AND, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_TRUE__thunk) },
  {    TokenType::IFF, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_TRUE__thunk) },
  {    TokenType::IMPLIES, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_TRUE__thunk) },
  {    TokenType::OR, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_TRUE__thunk) },
  {    TokenType::RPAREN, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_TRUE__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_TRUE__thunk) },
},
{
  {    TokenType::AND, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_OBJECT__thunk) },
  {    TokenType::EQUALS, new ShiftAction{14} },
  {    TokenType::IFF, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_OBJECT__thunk) },
  {    TokenType::IMPLIES, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_OBJECT__thunk) },
  {    TokenType::NOTEQUALS, new ShiftAction{3} },
  {    TokenType::OR, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_OBJECT__thunk) },
  {    TokenType::RPAREN, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_OBJECT__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_OBJECT__thunk) },
},
{
  {    Nonterminal::CALL, new ShiftAction{9} },
  {    TokenType::IDENTIFIER, new ShiftAction{5} },
  {    Nonterminal::OBJECT, new ShiftAction{4} },
},
{
  {    TokenType::AND, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_OBJECT_NOTEQUALS_OBJECT__thunk) },
  {    TokenType::IFF, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_OBJECT_NOTEQUALS_OBJECT__thunk) },
  {    TokenType::IMPLIES, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_OBJECT_NOTEQUALS_OBJECT__thunk) },
  {    TokenType::OR, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_OBJECT_NOTEQUALS_OBJECT__thunk) },
  {    TokenType::RPAREN, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_OBJECT_NOTEQUALS_OBJECT__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_OBJECT_NOTEQUALS_OBJECT__thunk) },
},
{
  {    TokenType::AND, new ReduceActionN<1>(Nonterminal::OBJECT, reduce_OBJECT_from_IDENTIFIER__thunk) },
  {    TokenType::COMMA, new ReduceActionN<1>(Nonterminal::OBJECT, reduce_OBJECT_from_IDENTIFIER__thunk) },
  {    TokenType::EQUALS, new ReduceActionN<1>(Nonterminal::OBJECT, reduce_OBJECT_from_IDENTIFIER__thunk) },
  {    TokenType::IFF, new ReduceActionN<1>(Nonterminal::OBJECT, reduce_OBJECT_from_IDENTIFIER__thunk) },
  {    TokenType::IMPLIES, new ReduceActionN<1>(Nonterminal::OBJECT, reduce_OBJECT_from_IDENTIFIER__thunk) },
  {    TokenType::LPAREN, new ShiftAction{6} },
  {    TokenType::NOTEQUALS, new ReduceActionN<1>(Nonterminal::OBJECT, reduce_OBJECT_from_IDENTIFIER__thunk) },
  {    TokenType::OR, new ReduceActionN<1>(Nonterminal::OBJECT, reduce_OBJECT_from_IDENTIFIER__thunk) },
  {    TokenType::RPAREN, new ReduceActionN<1>(Nonterminal::OBJECT, reduce_OBJECT_from_IDENTIFIER__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<1>(Nonterminal::OBJECT, reduce_OBJECT_from_IDENTIFIER__thunk) },
},
{
  {    Nonterminal::ARGLIST, new ShiftAction{13} },
  {    Nonterminal::ARGS, new ShiftAction{11} },
  {    Nonterminal::CALL, new ShiftAction{9} },
  {    TokenType::IDENTIFIER, new ShiftAction{5} },
  {    Nonterminal::OBJECT, new ShiftAction{7} },
  {    TokenType::RPAREN, new ReduceActionN<0>(Nonterminal::ARGS, reduce_ARGS_from__thunk) },
},
{
  {    TokenType::COMMA, new ShiftAction{8} },
  {    TokenType::RPAREN, new ReduceActionN<1>(Nonterminal::ARGLIST, reduce_ARGLIST_from_OBJECT__thunk) },
},
{
  {    Nonterminal::ARGLIST, new ShiftAction{10} },
  {    Nonterminal::CALL, new ShiftAction{9} },
  {    TokenType::IDENTIFIER, new ShiftAction{5} },
  {    Nonterminal::OBJECT, new ShiftAction{7} },
},
{
  {    TokenType::AND, new ReduceActionN<1>(Nonterminal::OBJECT, reduce_OBJECT_from_CALL__thunk) },
  {    TokenType::COMMA, new ReduceActionN<1>(Nonterminal::OBJECT, reduce_OBJECT_from_CALL__thunk) },
  {    TokenType::EQUALS, new ReduceActionN<1>(Nonterminal::OBJECT, reduce_OBJECT_from_CALL__thunk) },
  {    TokenType::IFF, new ReduceActionN<1>(Nonterminal::OBJECT, reduce_OBJECT_from_CALL__thunk) },
  {    TokenType::IMPLIES, new ReduceActionN<1>(Nonterminal::OBJECT, reduce_OBJECT_from_CALL__thunk) },
  {    TokenType::NOTEQUALS, new ReduceActionN<1>(Nonterminal::OBJECT, reduce_OBJECT_from_CALL__thunk) },
  {    TokenType::OR, new ReduceActionN<1>(Nonterminal::OBJECT, reduce_OBJECT_from_CALL__thunk) },
  {    TokenType::RPAREN, new ReduceActionN<1>(Nonterminal::OBJECT, reduce_OBJECT_from_CALL__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<1>(Nonterminal::OBJECT, reduce_OBJECT_from_CALL__thunk) },
},
{
  {    TokenType::RPAREN, new ReduceActionN<3>(Nonterminal::ARGLIST, reduce_ARGLIST_from_OBJECT_COMMA_ARGLIST__thunk) },
},
{
  {    TokenType::RPAREN, new ShiftAction{12} },
},
{
  {    TokenType::AND, new ReduceActionN<4>(Nonterminal::CALL, reduce_CALL_from_IDENTIFIER_LPAREN_ARGS_RPAREN__thunk) },
  {    TokenType::COMMA, new ReduceActionN<4>(Nonterminal::CALL, reduce_CALL_from_IDENTIFIER_LPAREN_ARGS_RPAREN__thunk) },
  {    TokenType::EQUALS, new ReduceActionN<4>(Nonterminal::CALL, reduce_CALL_from_IDENTIFIER_LPAREN_ARGS_RPAREN__thunk) },
  {    TokenType::IFF, new ReduceActionN<4>(Nonterminal::CALL, reduce_CALL_from_IDENTIFIER_LPAREN_ARGS_RPAREN__thunk) },
  {    TokenType::IMPLIES, new ReduceActionN<4>(Nonterminal::CALL, reduce_CALL_from_IDENTIFIER_LPAREN_ARGS_RPAREN__thunk) },
  {    TokenType::NOTEQUALS, new ReduceActionN<4>(Nonterminal::CALL, reduce_CALL_from_IDENTIFIER_LPAREN_ARGS_RPAREN__thunk) },
  {    TokenType::OR, new ReduceActionN<4>(Nonterminal::CALL, reduce_CALL_from_IDENTIFIER_LPAREN_ARGS_RPAREN__thunk) },
  {    TokenType::RPAREN, new ReduceActionN<4>(Nonterminal::CALL, reduce_CALL_from_IDENTIFIER_LPAREN_ARGS_RPAREN__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<4>(Nonterminal::CALL, reduce_CALL_from_IDENTIFIER_LPAREN_ARGS_RPAREN__thunk) },
},
{
  {    TokenType::RPAREN, new ReduceActionN<1>(Nonterminal::ARGS, reduce_ARGS_from_ARGLIST__thunk) },
},
{
  {    Nonterminal::CALL, new ShiftAction{9} },
  {    TokenType::IDENTIFIER, new ShiftAction{5} },
  {    Nonterminal::OBJECT, new ShiftAction{15} },
},
{
  {    TokenType::AND, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_OBJECT_EQUALS_OBJECT__thunk) },
  {    TokenType::IFF, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_OBJECT_EQUALS_OBJECT__thunk) },
  {    TokenType::IMPLIES, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_OBJECT_EQUALS_OBJECT__thunk) },
  {    TokenType::OR, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_OBJECT_EQUALS_OBJECT__thunk) },
  {    TokenType::RPAREN, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_OBJECT_EQUALS_OBJECT__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_OBJECT_EQUALS_OBJECT__thunk) },
},
{
  {    Nonterminal::CALL, new ShiftAction{9} },
  {    TokenType::EXISTS, new ShiftAction{33} },
  {    TokenType::FALSE, new ShiftAction{32} },
  {    TokenType::FORALL, new ShiftAction{28} },
  {    Nonterminal::FORMULA, new ShiftAction{38} },
  {    TokenType::IDENTIFIER, new ShiftAction{5} },
  {    TokenType::LPAREN, new ShiftAction{17} },
  {    TokenType::NOT, new ShiftAction{16} },
  {    Nonterminal::OBJECT, new ShiftAction{2} },
  {    TokenType::TRUE, new ShiftAction{1} },
},
{
  {    Nonterminal::CALL, new ShiftAction{9} },
  {    TokenType::EXISTS, new ShiftAction{33} },
  {    TokenType::FALSE, new ShiftAction{32} },
  {    TokenType::FORALL, new ShiftAction{28} },
  {    Nonterminal::FORMULA, new ShiftAction{18} },
  {    TokenType::IDENTIFIER, new ShiftAction{5} },
  {    TokenType::LPAREN, new ShiftAction{17} },
  {    TokenType::NOT, new ShiftAction{16} },
  {    Nonterminal::OBJECT, new ShiftAction{2} },
  {    TokenType::TRUE, new ShiftAction{1} },
},
{
  {    TokenType::AND, new ShiftAction{26} },
  {    TokenType::IFF, new ShiftAction{24} },
  {    TokenType::IMPLIES, new ShiftAction{22} },
  {    TokenType::OR, new ShiftAction{20} },
  {    TokenType::RPAREN, new ShiftAction{19} },
},
{
  {    TokenType::AND, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_LPAREN_FORMULA_RPAREN__thunk) },
  {    TokenType::IFF, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_LPAREN_FORMULA_RPAREN__thunk) },
  {    TokenType::IMPLIES, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_LPAREN_FORMULA_RPAREN__thunk) },
  {    TokenType::OR, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_LPAREN_FORMULA_RPAREN__thunk) },
  {    TokenType::RPAREN, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_LPAREN_FORMULA_RPAREN__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_LPAREN_FORMULA_RPAREN__thunk) },
},
{
  {    Nonterminal::CALL, new ShiftAction{9} },
  {    TokenType::EXISTS, new ShiftAction{33} },
  {    TokenType::FALSE, new ShiftAction{32} },
  {    TokenType::FORALL, new ShiftAction{28} },
  {    Nonterminal::FORMULA, new ShiftAction{21} },
  {    TokenType::IDENTIFIER, new ShiftAction{5} },
  {    TokenType::LPAREN, new ShiftAction{17} },
  {    TokenType::NOT, new ShiftAction{16} },
  {    Nonterminal::OBJECT, new ShiftAction{2} },
  {    TokenType::TRUE, new ShiftAction{1} },
},
{
  {    TokenType::AND, new ShiftAction{26} },
  {    TokenType::IFF, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_OR_FORMULA__thunk) },
  {    TokenType::IMPLIES, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_OR_FORMULA__thunk) },
  {    TokenType::OR, new ShiftAction{20} },
  {    TokenType::RPAREN, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_OR_FORMULA__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_OR_FORMULA__thunk) },
},
{
  {    Nonterminal::CALL, new ShiftAction{9} },
  {    TokenType::EXISTS, new ShiftAction{33} },
  {    TokenType::FALSE, new ShiftAction{32} },
  {    TokenType::FORALL, new ShiftAction{28} },
  {    Nonterminal::FORMULA, new ShiftAction{23} },
  {    TokenType::IDENTIFIER, new ShiftAction{5} },
  {    TokenType::LPAREN, new ShiftAction{17} },
  {    TokenType::NOT, new ShiftAction{16} },
  {    Nonterminal::OBJECT, new ShiftAction{2} },
  {    TokenType::TRUE, new ShiftAction{1} },
},
{
  {    TokenType::AND, new ShiftAction{26} },
  {    TokenType::IFF, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_IMPLIES_FORMULA__thunk) },
  {    TokenType::IMPLIES, new ShiftAction{22} },
  {    TokenType::OR, new ShiftAction{20} },
  {    TokenType::RPAREN, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_IMPLIES_FORMULA__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_IMPLIES_FORMULA__thunk) },
},
{
  {    Nonterminal::CALL, new ShiftAction{9} },
  {    TokenType::EXISTS, new ShiftAction{33} },
  {    TokenType::FALSE, new ShiftAction{32} },
  {    TokenType::FORALL, new ShiftAction{28} },
  {    Nonterminal::FORMULA, new ShiftAction{25} },
  {    TokenType::IDENTIFIER, new ShiftAction{5} },
  {    TokenType::LPAREN, new ShiftAction{17} },
  {    TokenType::NOT, new ShiftAction{16} },
  {    Nonterminal::OBJECT, new ShiftAction{2} },
  {    TokenType::TRUE, new ShiftAction{1} },
},
{
  {    TokenType::AND, new ShiftAction{26} },
  {    TokenType::IFF, new ShiftAction{24} },
  {    TokenType::IMPLIES, new ShiftAction{22} },
  {    TokenType::OR, new ShiftAction{20} },
  {    TokenType::RPAREN, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_IFF_FORMULA__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_IFF_FORMULA__thunk) },
},
{
  {    Nonterminal::CALL, new ShiftAction{9} },
  {    TokenType::EXISTS, new ShiftAction{33} },
  {    TokenType::FALSE, new ShiftAction{32} },
  {    TokenType::FORALL, new ShiftAction{28} },
  {    Nonterminal::FORMULA, new ShiftAction{27} },
  {    TokenType::IDENTIFIER, new ShiftAction{5} },
  {    TokenType::LPAREN, new ShiftAction{17} },
  {    TokenType::NOT, new ShiftAction{16} },
  {    Nonterminal::OBJECT, new ShiftAction{2} },
  {    TokenType::TRUE, new ShiftAction{1} },
},
{
  {    TokenType::AND, new ShiftAction{26} },
  {    TokenType::IFF, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_AND_FORMULA__thunk) },
  {    TokenType::IMPLIES, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_AND_FORMULA__thunk) },
  {    TokenType::OR, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_AND_FORMULA__thunk) },
  {    TokenType::RPAREN, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_AND_FORMULA__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_AND_FORMULA__thunk) },
},
{
  {    TokenType::IDENTIFIER, new ShiftAction{29} },
},
{
  {    TokenType::DOT, new ShiftAction{37} },
  {    TokenType::EXISTS, new ReduceActionN<0>(Nonterminal::OPTDOT, reduce_OPTDOT_from__thunk) },
  {    TokenType::FALSE, new ReduceActionN<0>(Nonterminal::OPTDOT, reduce_OPTDOT_from__thunk) },
  {    TokenType::FORALL, new ReduceActionN<0>(Nonterminal::OPTDOT, reduce_OPTDOT_from__thunk) },
  {    TokenType::IDENTIFIER, new ReduceActionN<0>(Nonterminal::OPTDOT, reduce_OPTDOT_from__thunk) },
  {    TokenType::LPAREN, new ReduceActionN<0>(Nonterminal::OPTDOT, reduce_OPTDOT_from__thunk) },
  {    TokenType::NOT, new ReduceActionN<0>(Nonterminal::OPTDOT, reduce_OPTDOT_from__thunk) },
  {    Nonterminal::OPTDOT, new ShiftAction{30} },
  {    TokenType::TRUE, new ReduceActionN<0>(Nonterminal::OPTDOT, reduce_OPTDOT_from__thunk) },
},
{
  {    Nonterminal::CALL, new ShiftAction{9} },
  {    TokenType::EXISTS, new ShiftAction{33} },
  {    TokenType::FALSE, new ShiftAction{32} },
  {    TokenType::FORALL, new ShiftAction{28} },
  {    Nonterminal::FORMULA, new ShiftAction{31} },
  {    TokenType::IDENTIFIER, new ShiftAction{5} },
  {    TokenType::LPAREN, new ShiftAction{17} },
  {    TokenType::NOT, new ShiftAction{16} },
  {    Nonterminal::OBJECT, new ShiftAction{2} },
  {    TokenType::TRUE, new ShiftAction{1} },
},
{
  {    TokenType::AND, new ReduceActionN<4>(Nonterminal::FORMULA, reduce_FORMULA_from_FORALL_IDENTIFIER_OPTDOT_FORMULA__thunk) },
  {    TokenType::IFF, new ReduceActionN<4>(Nonterminal::FORMULA, reduce_FORMULA_from_FORALL_IDENTIFIER_OPTDOT_FORMULA__thunk) },
  {    TokenType::IMPLIES, new ReduceActionN<4>(Nonterminal::FORMULA, reduce_FORMULA_from_FORALL_IDENTIFIER_OPTDOT_FORMULA__thunk) },
  {    TokenType::OR, new ReduceActionN<4>(Nonterminal::FORMULA, reduce_FORMULA_from_FORALL_IDENTIFIER_OPTDOT_FORMULA__thunk) },
  {    TokenType::RPAREN, new ReduceActionN<4>(Nonterminal::FORMULA, reduce_FORMULA_from_FORALL_IDENTIFIER_OPTDOT_FORMULA__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<4>(Nonterminal::FORMULA, reduce_FORMULA_from_FORALL_IDENTIFIER_OPTDOT_FORMULA__thunk) },
},
{
  {    TokenType::AND, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_FALSE__thunk) },
  {    TokenType::IFF, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_FALSE__thunk) },
  {    TokenType::IMPLIES, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_FALSE__thunk) },
  {    TokenType::OR, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_FALSE__thunk) },
  {    TokenType::RPAREN, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_FALSE__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_FALSE__thunk) },
},
{
  {    TokenType::IDENTIFIER, new ShiftAction{34} },
},
{
  {    TokenType::DOT, new ShiftAction{37} },
  {    TokenType::EXISTS, new ReduceActionN<0>(Nonterminal::OPTDOT, reduce_OPTDOT_from__thunk) },
  {    TokenType::FALSE, new ReduceActionN<0>(Nonterminal::OPTDOT, reduce_OPTDOT_from__thunk) },
  {    TokenType::FORALL, new ReduceActionN<0>(Nonterminal::OPTDOT, reduce_OPTDOT_from__thunk) },
  {    TokenType::IDENTIFIER, new ReduceActionN<0>(Nonterminal::OPTDOT, reduce_OPTDOT_from__thunk) },
  {    TokenType::LPAREN, new ReduceActionN<0>(Nonterminal::OPTDOT, reduce_OPTDOT_from__thunk) },
  {    TokenType::NOT, new ReduceActionN<0>(Nonterminal::OPTDOT, reduce_OPTDOT_from__thunk) },
  {    Nonterminal::OPTDOT, new ShiftAction{35} },
  {    TokenType::TRUE, new ReduceActionN<0>(Nonterminal::OPTDOT, reduce_OPTDOT_from__thunk) },
},
{
  {    Nonterminal::CALL, new ShiftAction{9} },
  {    TokenType::EXISTS, new ShiftAction{33} },
  {    TokenType::FALSE, new ShiftAction{32} },
  {    TokenType::FORALL, new ShiftAction{28} },
  {    Nonterminal::FORMULA, new ShiftAction{36} },
  {    TokenType::IDENTIFIER, new ShiftAction{5} },
  {    TokenType::LPAREN, new ShiftAction{17} },
  {    TokenType::NOT, new ShiftAction{16} },
  {    Nonterminal::OBJECT, new ShiftAction{2} },
  {    TokenType::TRUE, new ShiftAction{1} },
},
{
  {    TokenType::AND, new ReduceActionN<4>(Nonterminal::FORMULA, reduce_FORMULA_from_EXISTS_IDENTIFIER_OPTDOT_FORMULA__thunk) },
  {    TokenType::IFF, new ReduceActionN<4>(Nonterminal::FORMULA, reduce_FORMULA_from_EXISTS_IDENTIFIER_OPTDOT_FORMULA__thunk) },
  {    TokenType::IMPLIES, new ReduceActionN<4>(Nonterminal::FORMULA, reduce_FORMULA_from_EXISTS_IDENTIFIER_OPTDOT_FORMULA__thunk) },
  {    TokenType::OR, new ReduceActionN<4>(Nonterminal::FORMULA, reduce_FORMULA_from_EXISTS_IDENTIFIER_OPTDOT_FORMULA__thunk) },
  {    TokenType::RPAREN, new ReduceActionN<4>(Nonterminal::FORMULA, reduce_FORMULA_from_EXISTS_IDENTIFIER_OPTDOT_FORMULA__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<4>(Nonterminal::FORMULA, reduce_FORMULA_from_EXISTS_IDENTIFIER_OPTDOT_FORMULA__thunk) },
},
{
  {    TokenType::EXISTS, new ReduceActionN<1>(Nonterminal::OPTDOT, reduce_OPTDOT_from_DOT__thunk) },
  {    TokenType::FALSE, new ReduceActionN<1>(Nonterminal::OPTDOT, reduce_OPTDOT_from_DOT__thunk) },
  {    TokenType::FORALL, new ReduceActionN<1>(Nonterminal::OPTDOT, reduce_OPTDOT_from_DOT__thunk) },
  {    TokenType::IDENTIFIER, new ReduceActionN<1>(Nonterminal::OPTDOT, reduce_OPTDOT_from_DOT__thunk) },
  {    TokenType::LPAREN, new ReduceActionN<1>(Nonterminal::OPTDOT, reduce_OPTDOT_from_DOT__thunk) },
  {    TokenType::NOT, new ReduceActionN<1>(Nonterminal::OPTDOT, reduce_OPTDOT_from_DOT__thunk) },
  {    TokenType::TRUE, new ReduceActionN<1>(Nonterminal::OPTDOT, reduce_OPTDOT_from_DOT__thunk) },
},
{
  {    TokenType::AND, new ReduceActionN<2>(Nonterminal::FORMULA, reduce_FORMULA_from_NOT_FORMULA__thunk) },
  {    TokenType::IFF, new ReduceActionN<2>(Nonterminal::FORMULA, reduce_FORMULA_from_NOT_FORMULA__thunk) },
  {    TokenType::IMPLIES, new ReduceActionN<2>(Nonterminal::FORMULA, reduce_FORMULA_from_NOT_FORMULA__thunk) },
  {    TokenType::OR, new ReduceActionN<2>(Nonterminal::FORMULA, reduce_FORMULA_from_NOT_FORMULA__thunk) },
  {    TokenType::RPAREN, new ReduceActionN<2>(Nonterminal::FORMULA, reduce_FORMULA_from_NOT_FORMULA__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<2>(Nonterminal::FORMULA, reduce_FORMULA_from_NOT_FORMULA__thunk) },
},
{
  {    TokenType::AND, new ShiftAction{26} },
  {    TokenType::IFF, new ShiftAction{24} },
  {    TokenType::IMPLIES, new ShiftAction{22} },
  {    TokenType::OR, new ShiftAction{20} },
  {    TokenType::SCAN_EOF, new HaltAction() },
},

      };

      template <size_t N>
        void ReduceActionN<N>::reduce(stack<StackItem>& s) {
          #if PARSER_IS_VERBOSE
            cout << "  Should pop " << N << " items from the stack." << endl;
            cout << "    Before: " << s.size() << endl;
          #endif
          /* Invoke the callback on the proper arguments, getting the AuxData
           * to push onto the stack.
           */
          auto nextItem = DoReduction<N>::invoke(s, callback);
          #if PARSER_IS_VERBOSE
            cout << "    After: " << s.size() << endl;
          #endif

          int state = s.top().state;
          #if PARSER_IS_VERBOSE
            cout << "  Top state is " << state << endl;
          #endif

          /* Look up the shift destination for this nonterminal. */
          auto* action = static_cast<ShiftAction*>(kActionTable[state].at(reduceTo));
          #if PARSER_IS_VERBOSE
            cout << "  Should shift to state " << action->target << endl;
          #endif

          /* Push this onto the stack. */
          s.push({ action->target, { { }, nextItem } });
      }

      /* Internal parsing routine */
      AuxData parseInternal(queue<Token>& tokens) {
        stack<StackItem> s;

        /* Seed the stack with the initial state. */
        s.push({ 0, {} });

        /* Run the parser! */
        while (!tokens.empty()) {
          /* Look at the next token. We only consume it in a shift. */
          auto curr  = tokens.front();
          int  state = s.top().state;

          #if PARSER_IS_VERBOSE
            cout << "Current state: " << state << endl;
            cout << "  Symbol: " << curr.data << endl;
            cout << endl;
          #endif

          /* Look up the action to take; if there is no action, it's an error. */
          if (!kActionTable[state].count(curr.type)) {
            if (curr.type == TokenType::SCAN_EOF) {
              throw runtime_error("End of formula encountered unexpectedly. (Are you missing a close parenthesis?)");
            }
            throw runtime_error("Found \"" + to_string(curr) + "\" where it wasn't expected.");
          }

          /* See what action to take. */
          auto action = kActionTable[state].at(curr.type);
          if (auto* shift = dynamic_cast<ShiftAction*>(action)) {
            #if PARSER_IS_VERBOSE
              cout << "  Action: Shift to " << shift->target << endl;
            #endif
            s.push( { shift->target, {curr, {}} } ); // No special data.
            tokens.pop();
          } else if (auto* reduce = dynamic_cast<ReduceAction*>(action)) {
            #if PARSER_IS_VERBOSE
              cout << "  Action: Reduce" << endl;
            #endif
            reduce->reduce(s);
          } else if (dynamic_cast<HaltAction*>(action)) {
            #if PARSER_IS_VERBOSE
              cout << "  Action: Halt" << endl;
            #endif
            return s.top().data.data;
          } else {
            throw runtime_error("Unknown action.");
          }
        }

        throw runtime_error("Out of tokens, but parser hasn't finished.");
      }

      std::shared_ptr<ASTNode> reduce_CALL_from_IDENTIFIER_LPAREN_ARGS_RPAREN(const std::string& _parserArg1, const std::string&, std::vector<std::shared_ptr<ASTNode>> _parserArg3, const std::string&) {
    std::shared_ptr<ASTNode> _parserArg0;
    _parserArg0 = make_shared<InvokeNode>(_parserArg1, _parserArg3);
    return _parserArg0;
  }

  std::shared_ptr<ASTNode> reduce_FORMULA_from_EXISTS_IDENTIFIER_OPTDOT_FORMULA(const std::string&, const std::string& _parserArg2, _unused_, std::shared_ptr<ASTNode> _parserArg4) {
    std::shared_ptr<ASTNode> _parserArg0;
    _parserArg0 = make_shared<ExistentialNode>(_parserArg2, _parserArg4);
    return _parserArg0;
  }

  std::shared_ptr<ASTNode> reduce_FORMULA_from_FALSE(const std::string&) {
    std::shared_ptr<ASTNode> _parserArg0;
    _parserArg0 = make_shared<FOL::FalseNode>();
    return _parserArg0;
  }

  std::shared_ptr<ASTNode> reduce_FORMULA_from_FORALL_IDENTIFIER_OPTDOT_FORMULA(const std::string&, const std::string& _parserArg2, _unused_, std::shared_ptr<ASTNode> _parserArg4) {
    std::shared_ptr<ASTNode> _parserArg0;
    _parserArg0 = make_shared<UniversalNode>  (_parserArg2, _parserArg4);
    return _parserArg0;
  }

  std::shared_ptr<ASTNode> reduce_FORMULA_from_FORMULA_AND_FORMULA(std::shared_ptr<ASTNode> _parserArg1, const std::string&, std::shared_ptr<ASTNode> _parserArg3) {
    std::shared_ptr<ASTNode> _parserArg0;
    _parserArg0 = make_shared<AndNode>(_parserArg1, _parserArg3);
    return _parserArg0;
  }

  std::shared_ptr<ASTNode> reduce_FORMULA_from_FORMULA_IFF_FORMULA(std::shared_ptr<ASTNode> _parserArg1, const std::string&, std::shared_ptr<ASTNode> _parserArg3) {
    std::shared_ptr<ASTNode> _parserArg0;
    _parserArg0 = make_shared<IffNode>(_parserArg1, _parserArg3);
    return _parserArg0;
  }

  std::shared_ptr<ASTNode> reduce_FORMULA_from_FORMULA_IMPLIES_FORMULA(std::shared_ptr<ASTNode> _parserArg1, const std::string&, std::shared_ptr<ASTNode> _parserArg3) {
    std::shared_ptr<ASTNode> _parserArg0;
    _parserArg0 = make_shared<ImpliesNode>(_parserArg1, _parserArg3);
    return _parserArg0;
  }

  std::shared_ptr<ASTNode> reduce_FORMULA_from_FORMULA_OR_FORMULA(std::shared_ptr<ASTNode> _parserArg1, const std::string&, std::shared_ptr<ASTNode> _parserArg3) {
    std::shared_ptr<ASTNode> _parserArg0;
    _parserArg0 = make_shared<OrNode>(_parserArg1, _parserArg3);
    return _parserArg0;
  }

  std::shared_ptr<ASTNode> reduce_FORMULA_from_LPAREN_FORMULA_RPAREN(const std::string&, std::shared_ptr<ASTNode> _parserArg2, const std::string&) {
    std::shared_ptr<ASTNode> _parserArg0;
    _parserArg0 = _parserArg2;
    return _parserArg0;
  }

  std::shared_ptr<ASTNode> reduce_FORMULA_from_NOT_FORMULA(const std::string&, std::shared_ptr<ASTNode> _parserArg2) {
    std::shared_ptr<ASTNode> _parserArg0;
    _parserArg0 = make_shared<FOL::NotNode>(_parserArg2);
    return _parserArg0;
  }

  std::shared_ptr<ASTNode> reduce_FORMULA_from_OBJECT(std::shared_ptr<ASTNode> _parserArg1) {
    std::shared_ptr<ASTNode> _parserArg0;
    _parserArg0 = _parserArg1;
    return _parserArg0;
  }

  std::shared_ptr<ASTNode> reduce_FORMULA_from_OBJECT_EQUALS_OBJECT(std::shared_ptr<ASTNode> _parserArg1, const std::string&, std::shared_ptr<ASTNode> _parserArg3) {
    std::shared_ptr<ASTNode> _parserArg0;
    _parserArg0 = make_shared<EqualsNode>(_parserArg1, _parserArg3);
    return _parserArg0;
  }

  std::shared_ptr<ASTNode> reduce_FORMULA_from_OBJECT_NOTEQUALS_OBJECT(std::shared_ptr<ASTNode> _parserArg1, const std::string&, std::shared_ptr<ASTNode> _parserArg3) {
    std::shared_ptr<ASTNode> _parserArg0;
    _parserArg0 = make_shared<NotEqualsNode>(_parserArg1, _parserArg3);
    return _parserArg0;
  }

  std::shared_ptr<ASTNode> reduce_FORMULA_from_TRUE(const std::string&) {
    std::shared_ptr<ASTNode> _parserArg0;
    _parserArg0 = make_shared<FOL::TrueNode>();
    return _parserArg0;
  }

  std::shared_ptr<ASTNode> reduce_OBJECT_from_CALL(std::shared_ptr<ASTNode> _parserArg1) {
    std::shared_ptr<ASTNode> _parserArg0;
    _parserArg0 = _parserArg1;
    return _parserArg0;
  }

  std::shared_ptr<ASTNode> reduce_OBJECT_from_IDENTIFIER(const std::string& _parserArg1) {
    std::shared_ptr<ASTNode> _parserArg0;
    _parserArg0 = make_shared<VariableNode>(_parserArg1);
    return _parserArg0;
  }

  std::vector<std::shared_ptr<ASTNode>> reduce_ARGLIST_from_OBJECT(std::shared_ptr<ASTNode> _parserArg1) {
    std::vector<std::shared_ptr<ASTNode>> _parserArg0;
    _parserArg0 = {_parserArg1};
    return _parserArg0;
  }

  std::vector<std::shared_ptr<ASTNode>> reduce_ARGLIST_from_OBJECT_COMMA_ARGLIST(std::shared_ptr<ASTNode> _parserArg1, const std::string&, std::vector<std::shared_ptr<ASTNode>> _parserArg3) {
    std::vector<std::shared_ptr<ASTNode>> _parserArg0;
    _parserArg3.insert(_parserArg3.begin(), _parserArg1); _parserArg0 = _parserArg3;
    return _parserArg0;
  }

  std::vector<std::shared_ptr<ASTNode>> reduce_ARGS_from() {
    std::vector<std::shared_ptr<ASTNode>> _parserArg0;
    _parserArg0 = {};
    return _parserArg0;
  }

  std::vector<std::shared_ptr<ASTNode>> reduce_ARGS_from_ARGLIST(std::vector<std::shared_ptr<ASTNode>> _parserArg1) {
    std::vector<std::shared_ptr<ASTNode>> _parserArg0;
    _parserArg0 = _parserArg1;
    return _parserArg0;
  }


    }

    /* Public parsing routine. */
    std::shared_ptr<ASTNode> parse(queue<Token>& q) {
      return parseInternal(q).field0;
    }
    std::shared_ptr<ASTNode> parse(queue<Token>&& q) {
      return parseInternal(q).field0;
    }
}
