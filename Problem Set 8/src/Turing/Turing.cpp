#include "Turing.h"
#include "TuringScanner.h"
#include "TuringParser.h"
#include "StrUtils/StrUtils.h"
#include "Utilities/Unicode.h"
#include <deque>
#include <sstream>
#include <iostream>
#include <string>
#include <set>
#include <climits>
using namespace std;

namespace Turing {
    /* Visitor implementations */
    void Label::accept(Visitor& v) {
        v.visit(*this);
    }
    void Write::accept(Visitor& v) {
        v.visit(*this);
    }
    void Move::accept(Visitor& v) {
        v.visit(*this);
    }
    void Return::accept(Visitor& v) {
        v.visit(*this);
    }
    void Goto::accept(Visitor& v) {
        v.visit(*this);
    }
    void If::accept(Visitor& v) {
        v.visit(*this);
    }

    /* Pretty-printers. */
    namespace {
        /* Character -> String, in a pretty-printy way. */
        std::string toString(char32_t ch) {
            if (ch == Turing::kBlankSymbol) return "Blank";

            return "'" + toUTF8(ch) + "'";
        }

        /* Direction -> String */
        std::string toString(Direction d) {
            if (d == Direction::LEFT) return "Left";
            if (d == Direction::RIGHT) return "Right";
            return "<unknown direction>";
        }

        /* Expression -> String, in a pretty-printy way. */
        std::string toString(std::shared_ptr<Statement> stmt) {
            class Printer: public Calculator<string> {
                string handle(Label& l) override {
                    return l.label() + ":";
                }
                string handle(Move& m) override {
                    return "Move " + toString(m.direction());
                }
                string handle(Return& h) override {
                    return h.isAccepting()? "Accept" : "Reject";
                }
                string handle(Goto& g) override {
                    return "Goto " + g.label();
                }
                string handle(Write& p) override {
                    return "Print " + toString(p.ch());
                }
                string handle(If& i, const string& expr) override {
                    return string("If ") + (i.isNegated()? "Not " : "") + toString(i.ch()) + " " + expr;
                }
            };

            return Printer().calculate(*stmt);
        }
    }

    /* Parsing. */

    Program::Program(istream& in) {
        parse(in);
        semanticAnalyze();
    }

    Program::Program(istream&& in) : Program(in) {

    }

    namespace {
        /* Removes comments and leading/trailing whitespace. */
        void cleanLine(string& line) {
            size_t comment = line.find('#');
            if (comment != string::npos) {
                line.erase(comment);
            }

            line = Utilities::trim(line);
        }
    }

    /* Read the program one line at a time, stripping out comments and handing off the
     * line to the parser for AST assembly.
     */
    void Program::parse(istream& in) {
        for (string line; getline(in, line); ) {
            /* Store the line for later. */
            rawLines_.push_back(line);

            /* Remove comments and whitespace, if any. */
            cleanLine(line);

            /* Blank? Skip it. */
            if (line.empty()) continue;

            /* Parse the line. If that fails, record the error message. */
            try {
                auto stmt = Turing::parse(scan(line));

                /* Stash it for later. */
                statements_[rawLines_.size() - 1] = stmt;
            } catch (const exception& e) {
                errors_[rawLines_.size() - 1] = e.what();
            }
        }
    }

    /* Semantic analysis phase. We need to ensure that all labels are known, that no duplicated
     * labels exist, and that the label targets are all valid.
     */
    void Program::semanticAnalyze() {
        /* Find all labels and write them down. */
        for (const auto& entry: statements_) {
            if (auto label = dynamic_pointer_cast<Label>(entry.second)) {
                /* Add it, failing if it already exists. */
                auto result = labels_.insert(make_pair(label->label(), entry.first));
                if (!result.second) {
                    errors_[entry.first] = "Duplicate label; this was first defined at line " + std::to_string(result.first->second);
                }
            }
        }

        /* Make sure there's a start label. */
        if (!labels_.count(kStartLabel)) {
            /* Associate the error with Line 0, if there isn't already an error there. */
            if (!errors_.count(0)) {
                errors_[0] = "This program needs a " + string(kStartLabel) + " label so we know where to begin.";
            }
        }

        /* Find all used labels and make sure they exist. */

        /* Empty string corresponds to "no label." */
        class LabelsUsed: public Calculator<string> {
            string handle(Goto& g) override {
                return g.label();
            }
            string handle(If&, const string& expr) override {
                return expr;
            }
        };
        set<string> jumpTargets;
        for (const auto& entry: statements_) {
            string label = LabelsUsed().calculate(*entry.second);
            if (label != "" && !labels_.count(label)) {
                errors_[entry.first] = "Goto statement references undefined label '" + label + "'.";
            }

            if (label != "") {
                jumpTargets.insert(label);
            }
        }

        /* TODO: Do we want to validate whether all labels are jumped to? */
        #if 0
            /* Make sure all labels are a jump target, except possibly Start. */
            for (const auto& entry: labels_) {
                string label = entry.first;
                if (label != kStartLabel && !jumpTargets.count(label)) {
                    /* If there's no error here, add one. */
                    if (!errors_.count(entry.second)) {
                        errors_[entry.second] = "No Goto statement jumps to this label.";
                    }
                }
            }
        #endif

        /* TODO: Do you want to require the last line of each program to either be
         * Accept, Reject, or Goto?
         */
        #if 0
            /* Confirm that the last line is either an Accept, a Reject, or a Goto. */
            if (!statements_.empty()) {
                /* Last statement. */
                auto last     = statements_.rbegin();
                auto lastStmt = last->second;

                if (dynamic_pointer_cast<Halt>(lastStmt) == nullptr &&
                    dynamic_pointer_cast<Goto>(lastStmt) == nullptr) {
                    /* If there wasn't already an error here, report one. */
                    if (!errors_.count(last->first)) {
                        errors_[last->first] = "The last statement in a Turing program must be either Accept, Reject, or Goto.";
                    }
                }
            }
        #endif
    }

    /* The program is valid if there are no errors. */
    bool Program::isValid() const {
        return errors_.empty();
    }

    /* See if we have a known error at the given line. */
    string Program::errorAtLine(size_t line) const {
        auto itr = errors_.find(line);
        return itr == errors_.end()? "" : itr->second;
    }

    /* As advertised. */
    size_t Program::numLines() const {
        return rawLines_.size();
    }

    string Program::line(size_t i) const {
        return rawLines_.at(i);
    }

    /* Interpreter setup. We need to do the following:
     *
     * 1. Set up the tape by copying over the initial contents.
     * 2. Figure out our starting line number by locating the Start label.
     */
    Interpreter::Interpreter(const Program& p, const vector<char32_t>& input) :
        p_(&p), tape_(input.begin(), input.end()) {
        /* If there are any errors in the program, abort because we can't run the
         * program.
         */
        if (!p.isValid()) {
            throw invalid_argument("Cannot interpret a program that contains errors.");
        }

        /* Place an extra blank cell at the end of the tape. This avoids edge cases by ensuring
         * that the tape position always points at something.
         */
        tape_.push_back(kBlankSymbol);

        /* Look up the start symbol and begin there. */
        lineNumber_ = p.labels_.at(kStartLabel);
    }

    /* As advertised. */
    Result Interpreter::state() const {
        return state_;
    }

    /* Change from deque coordinates to world coordinates. */
    int64_t Interpreter::tapeHeadPos() const {
        return int64_t(tapePos_) + dequeBase_;
    }

    /* Jump to the given label. */
    void Interpreter::jumpTo(const string& label) {
        lineNumber_ = p_->labels_.at(label);
    }

    /* Skip to next executable line. */
    void Interpreter::toNextLine() {
        /* Keep incrementing the line number until we walk off the end or find
         * a statement.
         */
        do {
            lineNumber_++;
        } while (lineNumber_ < p_->rawLines_.size() && !p_->statements_.count(lineNumber_));
    }

    char32_t Interpreter::tapeAt(int64_t index) const {
        /* Convert to deque index by subtracting out the deque base. */
        index -= dequeBase_;

        if (index >= 0 && size_t(index) < tape_.size()) {
            return tape_[size_t(index)];
        } else {
            return kBlankSymbol;
        }
    }

    /* Advance the simulation forward one step. */
    void Interpreter::step() {
        /* If we're not running, don't do anything. */
        if (state() != Result::RUNNING) return;

        /* Cache the current line; we'll need to do this to execute jumps. */
        size_t lineBefore = lineNumber_;
        auto stmt = p_->statements_.at(lineNumber_);

        /* Run the statement. If it terminated the program, stop. */
        auto result = execute(*stmt);
        if (result != Result::RUNNING) {
            state_ = result;
            return;
        }

        /* If we didn't execute a jump, move to the next line. */
        if (lineBefore == lineNumber_) {
            toNextLine();
        }

        /* If we're past the end of the program, reject.
         *
         * TODO: You may be able to safely remove this if you add in a requirement that
         * all programs must end with Accept, Reject, or Goto.
         */
        if (lineNumber_ == p_->numLines()) {
            state_ = Result::REJECT;
        }
    }

    /* Visitor that executes each command. */
    class StatementExecutor: public Visitor {
    public:
        StatementExecutor(Interpreter& me, Result& result) : me_(me), result_(result) {}

        /* Print writes a character. */
        void visit(Write& p) override {
            me_.tape_[me_.tapePos_] = p.ch();
        }

        /* Move moves the tape head, handling boundaries as appropriate. */
        void visit(Move& m) override {
            if (m.direction() == Direction::RIGHT) {
                me_.tapePos_++;
                if (me_.tapePos_ == me_.tape_.size()) {
                    me_.tape_.push_back(kBlankSymbol);
                }
            } else if (m.direction() == Direction::LEFT) {
                /* Something before me? Back up. */
                if (me_.tapePos_ > 0) {
                    me_.tapePos_ --;
                }
                /* Nothing before me. Shift something new in. */
                else {
                    me_.tape_.push_front(kBlankSymbol);

                    /* The deque's starting position is now lower. */
                    me_.dequeBase_--;
                }
            } else throw runtime_error("Unknown direction?");
        }

        /* Goto executes a jump. */
        void visit(Goto& g) override {
            me_.jumpTo(g.label());
        }

        /* Halt stops the program. */
        void visit(Return& h) override {
            result_ = h.isAccepting()? Result::ACCEPT : Result::REJECT;
        }

        /* If statements check the condition and react appropriately. */
        void visit(If& h) override {
            /* Check whether the tape symbol matches/doesn't match, then fire off
             * the command if we're supposed to.
             */
            if ((me_.tape_[me_.tapePos_] == h.ch()) != h.isNegated()) {
                h.stmt()->accept(*this);
            }
        }

    private /* data */:
        Interpreter& me_;
        Result& result_;
    };

    /* Execute a single command. */
    Result Interpreter::execute(Statement& stmt) {
        Result result = Result::RUNNING;

        StatementExecutor runner(*this, result);
        stmt.accept(runner);
        return result;
    }
}
