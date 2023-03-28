#include "PLParser.h"
#include "PLScanner.h"
#include <queue>
#include <stack>
#include <vector>
#include <map>
#include <functional>
#include <iostream>
using namespace std;

#define PARSER_IS_VERBOSE (false)

namespace PL {
    namespace {
      template <typename T> T popFrom(stack<T>& s) {
        if (s.empty()) throw runtime_error("Pop from empty stack.");
        T result = std::move(s.top());
        s.pop();
        return result;
      }

      enum class Nonterminal {
        FORMULA,
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
        std::shared_ptr<Expression> field0;

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

      std::shared_ptr<Expression> reduce_FORMULA_from_FALSE(const std::string&);
  std::shared_ptr<Expression> reduce_FORMULA_from_FORMULA_AND_FORMULA(std::shared_ptr<Expression> _parserArg1, const std::string&, std::shared_ptr<Expression> _parserArg3);
  std::shared_ptr<Expression> reduce_FORMULA_from_FORMULA_IFF_FORMULA(std::shared_ptr<Expression> _parserArg1, const std::string&, std::shared_ptr<Expression> _parserArg3);
  std::shared_ptr<Expression> reduce_FORMULA_from_FORMULA_IMPLIES_FORMULA(std::shared_ptr<Expression> _parserArg1, const std::string&, std::shared_ptr<Expression> _parserArg3);
  std::shared_ptr<Expression> reduce_FORMULA_from_FORMULA_OR_FORMULA(std::shared_ptr<Expression> _parserArg1, const std::string&, std::shared_ptr<Expression> _parserArg3);
  std::shared_ptr<Expression> reduce_FORMULA_from_IDENTIFIER(const std::string& _parserArg1);
  std::shared_ptr<Expression> reduce_FORMULA_from_LPAREN_FORMULA_RPAREN(const std::string&, std::shared_ptr<Expression> _parserArg2, const std::string&);
  std::shared_ptr<Expression> reduce_FORMULA_from_NOT_FORMULA(const std::string&, std::shared_ptr<Expression> _parserArg2);
  std::shared_ptr<Expression> reduce_FORMULA_from_TRUE(const std::string&);


      AuxData reduce_FORMULA_from_FALSE__thunk(StackData a0) {
    AuxData result;
    result.field0 = reduce_FORMULA_from_FALSE(a0.token.data);
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

  AuxData reduce_FORMULA_from_IDENTIFIER__thunk(StackData a0) {
    AuxData result;
    result.field0 = reduce_FORMULA_from_IDENTIFIER(a0.token.data);
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

  AuxData reduce_FORMULA_from_TRUE__thunk(StackData a0) {
    AuxData result;
    result.field0 = reduce_FORMULA_from_TRUE(a0.token.data);
    return result;
  }



      /* Action table. */
      const vector<map<Symbol, Action*>> kActionTable = {
      {
  {    TokenType::FALSE, new ShiftAction{15} },
  {    Nonterminal::FORMULA, new ShiftAction{17} },
  {    TokenType::IDENTIFIER, new ShiftAction{4} },
  {    TokenType::LPAREN, new ShiftAction{3} },
  {    TokenType::NOT, new ShiftAction{2} },
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
  {    TokenType::FALSE, new ShiftAction{15} },
  {    Nonterminal::FORMULA, new ShiftAction{16} },
  {    TokenType::IDENTIFIER, new ShiftAction{4} },
  {    TokenType::LPAREN, new ShiftAction{3} },
  {    TokenType::NOT, new ShiftAction{2} },
  {    TokenType::TRUE, new ShiftAction{1} },
},
{
  {    TokenType::FALSE, new ShiftAction{15} },
  {    Nonterminal::FORMULA, new ShiftAction{5} },
  {    TokenType::IDENTIFIER, new ShiftAction{4} },
  {    TokenType::LPAREN, new ShiftAction{3} },
  {    TokenType::NOT, new ShiftAction{2} },
  {    TokenType::TRUE, new ShiftAction{1} },
},
{
  {    TokenType::AND, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_IDENTIFIER__thunk) },
  {    TokenType::IFF, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_IDENTIFIER__thunk) },
  {    TokenType::IMPLIES, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_IDENTIFIER__thunk) },
  {    TokenType::OR, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_IDENTIFIER__thunk) },
  {    TokenType::RPAREN, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_IDENTIFIER__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<1>(Nonterminal::FORMULA, reduce_FORMULA_from_IDENTIFIER__thunk) },
},
{
  {    TokenType::AND, new ShiftAction{13} },
  {    TokenType::IFF, new ShiftAction{11} },
  {    TokenType::IMPLIES, new ShiftAction{9} },
  {    TokenType::OR, new ShiftAction{7} },
  {    TokenType::RPAREN, new ShiftAction{6} },
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
  {    TokenType::FALSE, new ShiftAction{15} },
  {    Nonterminal::FORMULA, new ShiftAction{8} },
  {    TokenType::IDENTIFIER, new ShiftAction{4} },
  {    TokenType::LPAREN, new ShiftAction{3} },
  {    TokenType::NOT, new ShiftAction{2} },
  {    TokenType::TRUE, new ShiftAction{1} },
},
{
  {    TokenType::AND, new ShiftAction{13} },
  {    TokenType::IFF, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_OR_FORMULA__thunk) },
  {    TokenType::IMPLIES, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_OR_FORMULA__thunk) },
  {    TokenType::OR, new ShiftAction{7} },
  {    TokenType::RPAREN, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_OR_FORMULA__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_OR_FORMULA__thunk) },
},
{
  {    TokenType::FALSE, new ShiftAction{15} },
  {    Nonterminal::FORMULA, new ShiftAction{10} },
  {    TokenType::IDENTIFIER, new ShiftAction{4} },
  {    TokenType::LPAREN, new ShiftAction{3} },
  {    TokenType::NOT, new ShiftAction{2} },
  {    TokenType::TRUE, new ShiftAction{1} },
},
{
  {    TokenType::AND, new ShiftAction{13} },
  {    TokenType::IFF, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_IMPLIES_FORMULA__thunk) },
  {    TokenType::IMPLIES, new ShiftAction{9} },
  {    TokenType::OR, new ShiftAction{7} },
  {    TokenType::RPAREN, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_IMPLIES_FORMULA__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_IMPLIES_FORMULA__thunk) },
},
{
  {    TokenType::FALSE, new ShiftAction{15} },
  {    Nonterminal::FORMULA, new ShiftAction{12} },
  {    TokenType::IDENTIFIER, new ShiftAction{4} },
  {    TokenType::LPAREN, new ShiftAction{3} },
  {    TokenType::NOT, new ShiftAction{2} },
  {    TokenType::TRUE, new ShiftAction{1} },
},
{
  {    TokenType::AND, new ShiftAction{13} },
  {    TokenType::IFF, new ShiftAction{11} },
  {    TokenType::IMPLIES, new ShiftAction{9} },
  {    TokenType::OR, new ShiftAction{7} },
  {    TokenType::RPAREN, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_IFF_FORMULA__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_IFF_FORMULA__thunk) },
},
{
  {    TokenType::FALSE, new ShiftAction{15} },
  {    Nonterminal::FORMULA, new ShiftAction{14} },
  {    TokenType::IDENTIFIER, new ShiftAction{4} },
  {    TokenType::LPAREN, new ShiftAction{3} },
  {    TokenType::NOT, new ShiftAction{2} },
  {    TokenType::TRUE, new ShiftAction{1} },
},
{
  {    TokenType::AND, new ShiftAction{13} },
  {    TokenType::IFF, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_AND_FORMULA__thunk) },
  {    TokenType::IMPLIES, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_AND_FORMULA__thunk) },
  {    TokenType::OR, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_AND_FORMULA__thunk) },
  {    TokenType::RPAREN, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_AND_FORMULA__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<3>(Nonterminal::FORMULA, reduce_FORMULA_from_FORMULA_AND_FORMULA__thunk) },
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
  {    TokenType::AND, new ReduceActionN<2>(Nonterminal::FORMULA, reduce_FORMULA_from_NOT_FORMULA__thunk) },
  {    TokenType::IFF, new ReduceActionN<2>(Nonterminal::FORMULA, reduce_FORMULA_from_NOT_FORMULA__thunk) },
  {    TokenType::IMPLIES, new ReduceActionN<2>(Nonterminal::FORMULA, reduce_FORMULA_from_NOT_FORMULA__thunk) },
  {    TokenType::OR, new ReduceActionN<2>(Nonterminal::FORMULA, reduce_FORMULA_from_NOT_FORMULA__thunk) },
  {    TokenType::RPAREN, new ReduceActionN<2>(Nonterminal::FORMULA, reduce_FORMULA_from_NOT_FORMULA__thunk) },
  {    TokenType::SCAN_EOF, new ReduceActionN<2>(Nonterminal::FORMULA, reduce_FORMULA_from_NOT_FORMULA__thunk) },
},
{
  {    TokenType::AND, new ShiftAction{13} },
  {    TokenType::IFF, new ShiftAction{11} },
  {    TokenType::IMPLIES, new ShiftAction{9} },
  {    TokenType::OR, new ShiftAction{7} },
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

      std::shared_ptr<Expression> reduce_FORMULA_from_FALSE(const std::string&) {
    std::shared_ptr<Expression> _parserArg0;
    _parserArg0 = make_shared<FalseExpression>();
    return _parserArg0;
  }

  std::shared_ptr<Expression> reduce_FORMULA_from_FORMULA_AND_FORMULA(std::shared_ptr<Expression> _parserArg1, const std::string&, std::shared_ptr<Expression> _parserArg3) {
    std::shared_ptr<Expression> _parserArg0;
    _parserArg0 = make_shared<AndExpression>(_parserArg1, _parserArg3);
    return _parserArg0;
  }

  std::shared_ptr<Expression> reduce_FORMULA_from_FORMULA_IFF_FORMULA(std::shared_ptr<Expression> _parserArg1, const std::string&, std::shared_ptr<Expression> _parserArg3) {
    std::shared_ptr<Expression> _parserArg0;
    _parserArg0 = make_shared<IffExpression>(_parserArg1, _parserArg3);
    return _parserArg0;
  }

  std::shared_ptr<Expression> reduce_FORMULA_from_FORMULA_IMPLIES_FORMULA(std::shared_ptr<Expression> _parserArg1, const std::string&, std::shared_ptr<Expression> _parserArg3) {
    std::shared_ptr<Expression> _parserArg0;
    _parserArg0 = make_shared<ImpliesExpression>(_parserArg1, _parserArg3);
    return _parserArg0;
  }

  std::shared_ptr<Expression> reduce_FORMULA_from_FORMULA_OR_FORMULA(std::shared_ptr<Expression> _parserArg1, const std::string&, std::shared_ptr<Expression> _parserArg3) {
    std::shared_ptr<Expression> _parserArg0;
    _parserArg0 = make_shared<OrExpression>(_parserArg1, _parserArg3);
    return _parserArg0;
  }

  std::shared_ptr<Expression> reduce_FORMULA_from_IDENTIFIER(const std::string& _parserArg1) {
    std::shared_ptr<Expression> _parserArg0;
    _parserArg0 = make_shared<VariableExpression>(_parserArg1);
    return _parserArg0;
  }

  std::shared_ptr<Expression> reduce_FORMULA_from_LPAREN_FORMULA_RPAREN(const std::string&, std::shared_ptr<Expression> _parserArg2, const std::string&) {
    std::shared_ptr<Expression> _parserArg0;
    _parserArg0 = _parserArg2;
    return _parserArg0;
  }

  std::shared_ptr<Expression> reduce_FORMULA_from_NOT_FORMULA(const std::string&, std::shared_ptr<Expression> _parserArg2) {
    std::shared_ptr<Expression> _parserArg0;
    _parserArg0 = make_shared<NotExpression>(_parserArg2);
    return _parserArg0;
  }

  std::shared_ptr<Expression> reduce_FORMULA_from_TRUE(const std::string&) {
    std::shared_ptr<Expression> _parserArg0;
    _parserArg0 = make_shared<TrueExpression>();
    return _parserArg0;
  }


    }

    /* Public parsing routine. */
    std::shared_ptr<Expression> parse(queue<Token>& q) {
      return parseInternal(q).field0;
    }
    std::shared_ptr<Expression> parse(queue<Token>&& q) {
      return parseInternal(q).field0;
    }
}
