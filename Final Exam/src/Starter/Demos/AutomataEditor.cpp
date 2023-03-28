#include "AutomataEditor.h"
#include "../GUI/MiniGUI.h"
#include "Utilities/Unicode.h"
#include "../GraphEditor/GVector.h"
#include "../FormalLanguages/Automaton.h"
#include <cmath>
#include <set>
#include <unordered_map>
#include <sstream>
using namespace std;

namespace Editor {
    namespace {
        /* Label to display on an empty transition. Do not include spaces here;
         * use nonbreaking spaces if needed.
         */
        const string kEmptyTransitionLabel = "(select chars)";

        const double kAcceptingRadius  = GraphEditor::kNodeRadius * 0.8;
        const double kStartArrowLength = GraphEditor::kNodeRadius;

        /* Map from numerals to their superscript equivalents. */
        const unordered_map<char, string> kDigitToSubscript = {
            { '0', "₀" },
            { '1', "₁" },
            { '2', "₂" },
            { '3', "₃" },
            { '4', "₄" },
            { '5', "₅" },
            { '6', "₆" },
            { '7', "₇" },
            { '8', "₈" },
            { '9', "₉" },
        };

        /* Given a start state, returns the points of the arrow that should be drawn
         * to indicate that it's a start state.
         */
        pair<GPoint, GPoint> startArrowPointsFor(State* state) {
            /* Scoot to the border of the state, then back up by the arrow size. */
            GPoint to   = state->position() + GVector{-1, 0} * GraphEditor::kNodeRadius;
            GPoint from = to + GVector{-1, 0} * kStartArrowLength;
            return make_pair(from, to);
        }

        /* Utility function to convert a number into a subscript
         * string.
         */
        string toSubscript(size_t value) {
            string result;
            for (char ch: to_string(value)) {
                result += kDigitToSubscript.at(ch);
            }
            return result;
        }
    }

    Automata::NFA Automaton::toNFA() {
        Automata::NFA result;
        result.alphabet = mAlphabet;

        /* Convert the states. */
        unordered_map<GraphEditor::Node*, Automata::State*> translation;
        forEachNode([&](State* state) {
            translation[state] = result.newState(state->label(), state->start(), state->accepting());
        });

        /* Add the transitions. */
        forEachEdge([&](Transition* transition) {
            for (char32_t ch: transition->chars()) {
                translation[transition->from()]->transitions.insert(make_pair(ch, translation[transition->to()]));
            }
        });

        return result;
    }

    namespace {
        string labelFor(const Languages::Alphabet& alphabet, const set<char32_t>& labels) {
            /* Edge case: If there's nothing here, that's bad, and we should
             * report an error.
             */
            if (labels.empty()) {
                return kEmptyTransitionLabel;
            }

            /* If this has a transition on all characters, use sigma instead of listing
             * them all.
             */
            if (Languages::isSubsetOf(alphabet, labels)) {
                return labels.count(Automata::EPSILON_TRANSITION)? "Σ, ε" : "Σ";
            }

            /* Form the string to display. */
            vector<char32_t> chars(labels.begin(), labels.end());
            sort(chars.begin(), chars.end(), [&](char32_t lhs, char32_t rhs) {
                /* Epsilon transitions always lose. */
                if (lhs == Automata::EPSILON_TRANSITION) return false;
                if (rhs == Automata::EPSILON_TRANSITION) return true;
                return lhs < rhs;
            });

            string label;
            for (size_t i = 0; i < chars.size(); i++) {
                label += (chars[i] == Automata::EPSILON_TRANSITION? "ε" : toUTF8(chars[i]));
                if (i + 1 != chars.size()) label += ", ";
            }

            return label;
        }
    }

    /* * * * * State Implementation * * * * */
    State::State(GraphEditor::ViewerBase* base,
                 const GraphEditor::NodeArgs& args,
                 JSON aux) : GraphEditor::Node(base, args) {
        /* Override label with computed label. */
        this->label("q" + toSubscript(index()));

        /* Accepting / start. */
        if (aux.type() != JSON::Type::NULLPTR_T) {
            mStart     = aux["start"].asBoolean();
            mAccepting = aux["accepting"].asBoolean();
        }
    }

    void State::draw(GraphEditor::ViewerBase* viewer, GCanvas* canvas, const GraphEditor::NodeStyle& style) {
        /* If this is a start state, draw an arrow next to it. */
        if (start()) {
            auto points = startArrowPointsFor(this);
            viewer->drawArrow(canvas, points.first, points.second, GraphEditor::kEdgeWidth, GraphEditor::kEdgeColor);
        }

        /* Do the normal state rendering. */
        Node::draw(viewer, canvas, style);

        /* If we're an accepting state, draw a smaller copy of ourselves inside
         * the state.
         */
        if (accepting()) {
            GraphEditor::NodeStyle newStyle;
            newStyle.radius = kAcceptingRadius;
            newStyle.fillColor = style.fillColor;
            Node::draw(viewer, canvas, newStyle);
        }
    }

    JSON State::toJSON() {
        return JSON::object({
            { "start",     start()     },
            { "accepting", accepting() }
        });
    }

    bool State::start() {
        return mStart;
    }
    void State::start(bool isStart) {
        mStart = isStart;
    }

    bool State::accepting() {
        return mAccepting;
    }
    void State::accepting(bool accepting) {
        mAccepting = accepting;
    }

    /* * * * * Transition Implementation * * * * */
    Transition::Transition(GraphEditor::ViewerBase* owner,
                                   const GraphEditor::EdgeArgs& args,
                                   JSON aux) : GraphEditor::Edge(owner, args) {
        mOwner = static_cast<Automaton*>(owner);

        /* Loading from the past? Decode the characters on this transition. */
        if (aux.type() != JSON::Type::NULLPTR_T) {
            mChars = Languages::toAlphabet(aux.asString());
        }
        /* New transition? Set the label to the empty label. We cannot use
         * updateLabel() here because during a deserialize the base object
         * hasn't yet been constructed.
         */
        else {
            this->label(kEmptyTransitionLabel);
        }
    }

    JSON Transition::toJSON() {
        return Languages::toString(chars());
    }

    const set<char32_t>& Transition::chars() {
        return mChars;
    }

    void Transition::add(char32_t ch) {
        if (mChars.insert(ch).second) {
            updateLabel();
        }
    }

    void Transition::remove(char32_t ch) {
        if (mChars.erase(ch)) {
            updateLabel();
        }
    }

    void Transition::updateLabel() {
        label(labelFor(mOwner->alphabet(), mChars));
    }

    /* Automaton aux data is the alphabet, whether it's supposed to be a DFA, and
     * the automaton itself.
     */
    string toString(const Automata::NFA& nfa) {
        ostringstream result;
        result << nfa;
        return result.str();
    }

    JSON Automaton::auxData() {
        return JSON::object({
            { "alphabet",  Languages::toString(alphabet()) },
            { "isDFA",     isDFA()                         },
            { "automaton", toString(toNFA())               }
        });
    }

    Automaton::Automaton(JSON j) : GraphEditor::Viewer<State, Transition>(j) {
        mAlphabet = Languages::toAlphabet(j["aux"]["alphabet"].asString());
        mIsDFA    = j["aux"]["isDFA"].asBoolean();
    }

    Languages::Alphabet Automaton::alphabet() {
        return mAlphabet;
    }

    bool Automaton::isDFA() {
        return mIsDFA;
    }




    /***** Global Helpers *****/


    namespace {
        /* Adds commas into a set. */
        string commafy(const Languages::Alphabet& alphabet, const string& conjunction = "and") {
            if (alphabet.size() == 1) return toUTF8(*alphabet.begin());
            if (alphabet.size() == 2) return toUTF8(*alphabet.begin()) + " " + conjunction + " " + toUTF8(*next(alphabet.begin()));

            /* Form the string x1, x2, x3, ..., conjunction xn. */
            string result;
            for (auto itr = alphabet.begin(); itr != alphabet.end();  ++itr) {
                if (itr != alphabet.begin()) result += ", ";
                if (next(itr) == alphabet.end()) result += conjunction + " ";
                result += toUTF8(*itr);
            }

            return result;
        }
    }

    vector<string> Automaton::checkValidity() {
        vector<string> errors;

        /* Make sure there's a start state. */
        Editor::State* start = nullptr;
        forEachNode([&](State* state) {
            if (state->start()) start = state;
        });

        if (start == nullptr) {
            errors.push_back("Automaton has no start state");
        }

        /* Report any bad transitions. */
        forEachEdge([&](Transition* t) {
            if (t->chars().empty()) {
                errors.push_back("Transition from " + t->from()->label() + " to " + t->to()->label() + " has no label");
            }
        });

        /* If we're a DFA, validate that all transitions appear once and exactly once. */
        if (isDFA()) {
            /* State -> ch -> frequency */
            map<GraphEditor::Node*, map<char32_t, size_t>> frequencies;
            forEachEdge([&](Transition* transition) {
                for (char32_t ch: transition->chars()) {
                    frequencies[transition->from()][ch]++;
                }
            });

            /* Confirm these are all valid. Use a map to keep things sorted. */
            map<size_t, vector<string>> transitionErrors;
            forEachNode([&](State* node) {
                Languages::Alphabet missing;
                Languages::Alphabet multiple;

                for (char32_t ch: alphabet()) {
                    if (frequencies[node][ch] == 0) missing.insert(ch);
                    if (frequencies[node][ch] >= 2) multiple.insert(ch);
                }

                vector<string> bad;
                if (!missing.empty()) {
                    bad.push_back("State " + node->label() + " has no transition on " + commafy(missing, "or"));
                }
                if (!multiple.empty()) {
                    bad.push_back("State " + node->label() + " has multiple transitions on " + commafy(multiple));
                }

                if (!bad.empty()) {
                    transitionErrors[node->index()] = bad;
                }
            });

            /* Build the error string from this. */
            for (const auto& entry: transitionErrors) {
                for (const auto& error: entry.second) {
                    errors.push_back(error);
                }
            }
        }

        return errors;
    }
}
