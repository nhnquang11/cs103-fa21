/******************************************************************************
 * Turing programs. Many independent mathematicians have developed systems like
 * this one: Emil Post, Hao Wang, Martin Davis, etc. We're calling them
 * "Turing programs" because they involve the basic idea of maintaining an
 * infinite tape, of which only one cell is "in the machine" at a time, and
 * executing a sequence of commands that move the tape head left and right
 * and change what's marked on the tape.
 *
 * Programs consist of several lines. Each line can be one of the following:
 *
 * Label:
 * Move Left
 * Move Right
 * Write [ch]
 * Goto Label
 * Accept
 * Reject
 * If [ch] [non-if-statement]
 * If Not [ch] [non-if-statement]
 *
 * Here, [ch] represents a character and can be one of 'single-letter' or Blank.
 *
 * Execution begins at the label Start. The last line of the program must be
 * either Accept, Reject, or Goto, so that there's no ambiguity about what
 * happens if we fall off the end of the program.
 */
#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <istream>
#include <deque>

namespace Turing {
    class Statement;
    class Interpreter;

    /* Blank character. */
    const char32_t kBlankSymbol = 0;

    /* Result of running the interpreter. */
    enum class Result {
        ACCEPT, REJECT, RUNNING
    };

    constexpr char kStartLabel[] = "Start";

    /* Type representing a Turing program. The constructor parses an input stream that
     * contains the relevant program and provides access to the underlying program and
     * access to the errors that occurred during parsing, if any.
     *
     * If the program is valid, it can be interpreted using an Interpreter object.
     */
    class Program {
    public:
        /* Parses the given input stream into a program. */
        Program(std::istream& in);
        Program(std::istream&& in);

        /* Underlying program accessors. */
        size_t numLines() const;
        std::string line(size_t lineNo) const;

        /* Whether the program has any errors. */
        bool isValid() const;

        /* The error associated with a given line, or the empty string if
         * that line has no errors.
         */
        std::string errorAtLine(size_t lineNo) const;

    private /* state */:
        /* Raw lines of the program. */
        std::vector<std::string> rawLines_;

        /* Map from lines to statements. */
        std::map<size_t, std::shared_ptr<Statement>> statements_;

        /* Map from labels to line numbers. */
        std::map<std::string, size_t> labels_;

        /* Map from lines to errors. */
        std::map<size_t, std::string> errors_;

        friend class Interpreter;

    private /* helpers */:
        void parse(std::istream& in);
        void semanticAnalyze();
    };

    /* Turing interpreter. This class functions like a universal TM: you give it
     * a program and an input, and it sets up a simulation that you can then use
     * to run the program.
     */
    class Interpreter {
    public:
        Interpreter(const Program& p, const std::vector<char32_t>& input);

        /* Current line number. */
        size_t lineNumber() const {
            return lineNumber_;
        }

        /* Returns the position of the tape head. The tape head's initial
         * position is defined to be 0, and this measures the number of steps
         * taken from there.
         */
        int64_t tapeHeadPos() const;

        /* Returns the tape character at the indicated position, which may
         * be the blank symbol if it's out of range.
         */
        char32_t tapeAt(int64_t pos) const;

        /* Current state of the interpreter. This is initially RUNNING but
         * may change to ACCEPT or REJECT based on how the program runs.
         */
        Result state() const;

        /* Advances one step forward. Calling this function when the state
         * is ACCEPT or REJECT is a no-op.
         */
        void step();

    private /* data */:
        /* Reference program. */
        const Program* p_;

        /* Current state. */
        Result state_ = Result::RUNNING;

        /* Which line to execute next. */
        size_t lineNumber_;

        /* Tape contents. We use a deque to make things easier. */
        std::deque<char32_t> tape_;

        /* The tape position. It's encoded as an index into the deque. This is what's
         * used internally. The external interface expects this number to be relative
         * to the starting position, so we keep track of a second counter that tracks
         * how far before 0 the start of the tape is.
         */
        size_t tapePos_ = 0;

        /* The index of the cell at the front of the deque. This is used to convert our
         * deque-relative coordinate system into an absolute coordinate system.
         */
        int64_t dequeBase_ = 0;

    private /* helpers */:
        /* Steps the program counter forward to the next line after the
         * current one, skipping comments.
         */
        void toNextLine();

        /* Executes the given command. */
        Result execute(Statement& stmt);

        /* Friendship with Program requires us to export these for
         * other types that need them.
         */
        void jumpTo(const std::string& label);

        friend class StatementExecutor;
    };


    class Visitor;

    class Statement {
    public:
        virtual ~Statement() = default;

        /* Executes this line of the program. */
        virtual void accept(Visitor& v) = 0;
    };

    /*** Statement Types ***/

    class Label: public Statement {
    public:
        Label(const std::string& label) : label_(label) {}

        std::string label() const {
            return label_;
        }

        virtual void accept(Visitor& v) override;

    private:
        std::string label_;
    };

    enum class Direction {
        LEFT, RIGHT
    };

    class Move: public Statement {
    public:
        Move(Direction direction) : direction_(direction) {}

        Direction direction() const {
            return direction_;
        }

        virtual void accept(Visitor& v) override;

    private:
        Direction direction_;
    };

    class Write: public Statement {
    public:
        Write(char32_t ch) : ch_(ch) {};

        char32_t ch() const {
            return ch_;
        }

        virtual void accept(Visitor& v) override;

    private:
        char32_t ch_;
    };

    class Goto: public Statement {
    public:
        Goto(const std::string& label) : label_(label) {}

        std::string label() const {
            return label_;
        }

        virtual void accept(Visitor& v) override;

    private:
        std::string label_;
    };

    class Return: public Statement {
    public:
        Return(bool isAccept) : isAccept_(isAccept) {}

        virtual void accept(Visitor& v) override;

        bool isAccepting() const {
            return isAccept_;
        }

    private:
        bool isAccept_;
    };

    class If: public Statement {
    public:
        If(bool isNegated, char32_t ch, std::shared_ptr<Statement> stmt) : isNegated_(isNegated), ch_(ch), stmt_(stmt) {}

        bool isNegated() const {
            return isNegated_;
        }

        char32_t ch() const {
            return ch_;
        }

        std::shared_ptr<Statement> stmt() const {
            return stmt_;
        }

        virtual void accept(Visitor& v) override;

    private:
        bool isNegated_;
        char32_t ch_;
        std::shared_ptr<Statement> stmt_;
    };

    /* Visitor type. */
    class Visitor {
    public:
        virtual ~Visitor() = default;

        virtual void visit(Label&) {}
        virtual void visit(Move&) {}
        virtual void visit(Write&) {}
        virtual void visit(Return&) {}
        virtual void visit(If&) {}
        virtual void visit(Goto&) {}
    };

    template <typename T> class Calculator: public Visitor {
    public:
        T calculate(Statement& stmt) {
            stmt.accept(*this);
            return result_;
        }

        virtual T handle(Label&) { return T(); }
        virtual T handle(Move&) { return T(); }
        virtual T handle(Write&) { return T(); }
        virtual T handle(Return&) { return T(); }
        virtual T handle(If&, const T&) { return T(); }
        virtual T handle(Goto&) { return T(); }

    private:
        virtual void visit(Label& l) override final {
            result_ = handle(l);
        }

        virtual void visit(Move& m) override final {
            result_ = handle(m);
        }
        virtual void visit(Write& p) override final {
            result_ = handle(p);
        }
        virtual void visit(Return& h) override final {
            result_ = handle(h);
        }
        virtual void visit(If& i) override final {
            i.stmt()->accept(*this);
            T temp = result_; // Avoid reference shenanigans
            result_ = handle(i, temp);
        }
        virtual void visit(Goto& g) override final {
            result_ = handle(g);
        }

        T result_;
    };
}
