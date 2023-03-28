#include "../GUI/MiniGUI.h"
#include "../CFGLoader.h"
#include "../FormalLanguages/CFG.h"
#include "CFGHtml.h"
#include "Utilities/JSON.h"
#include "gbrowserpane.h"
#include "filelib.h"
#include "strlib.h"
#include <vector>
#include <unordered_map>
using namespace std;

namespace {
    /* Information about a cfg. */
    struct CFGInfo {
        string sectionHeader;
        Languages::Alphabet alphabet;
    };

    string toString(const CFGInfo& cfg) {
        return cfg.sectionHeader;
    }

    const vector<CFGInfo> kCFGs = {
        { "Q1.i",   Languages::toAlphabet("abc") },
        { "Q1.ii",  Languages::toAlphabet("a.@") },
        { "Q1.iii", Languages::toAlphabet("ab") },
        { "Q1.iv",  Languages::toAlphabet("nuo,{}") },
        { "Q2.ii",  Languages::toAlphabet("1+=") },
        { "Q3.ii",  Languages::toAlphabet("ab") },
    };


    class CFGDeriveGUI: public ProblemHandler {
    public:
        CFGDeriveGUI(GWindow& window);

        void changeOccurredIn(GObservable* source) override;

    private:
        /* Central console. */
        Temporary<GBrowserPane> console;

        /* Bottom panel */
        Temporary<GLabel>    inputLabel;
        Temporary<GTextField> input;

        /* CFG selector at the bottom. */
        Temporary<GComboBox> selector;

        /* Current deriver, or nullptr if there was an error */
        CFG::Deriver deriver;

        /* Display message - either an error or the actual CFG. */
        string messageHTML;

        /* Currently-selected CFG. */
        CFGInfo currCFG;

        void updateCFG();
        void updateDisplay();
    };

    CFGDeriveGUI::CFGDeriveGUI(GWindow& window) : ProblemHandler(window) {
        /* Combo box. */
        GComboBox* options = new GComboBox();
        for (const auto& option: kCFGs) {
            options->addItem(toString(option));
        }
        options->setEditable(false);
        selector = Temporary<GComboBox>(options, window, "SOUTH");

        /* Side panels. */
        inputLabel = make_temporary<GLabel>(window, "SOUTH", "Input String: ");
        input      = make_temporary<GTextField>(window, "SOUTH");
        input->setPlaceholder("ε");

        /* Central panel. */
        console           = make_temporary<GBrowserPane>(window, "CENTER");

        /* Configure display for the current (default) CFG. */
        updateCFG();
    }

    /* Basic HTML format. */
    const string kHTMLTemplate =
R"(<html>
    <head>
    </head>
    <body style="color:black;background-color:white;font-size:%spt;">
    <h1>CFG Derivation Viewer</h1>
    <p>
      This tool shows how your CFGs derive strings in their language. Select a CFG using the
      dropdown menu, then enter a string below to see one of its derivations.
    </p>
    <p>
      There may be multiple derivations for a string. For simplicity, this tool will only
      show one of them.
    </p>
    <table cellpadding="3" cellspacing="0" align="center">
    <tr>
      <th colspan="3">%s</th> <!-- CFG -->
    </tr>
    %s <!-- Test Results -->
    </table>
    </body>
    </html>)";

    /* Global font size. */
    const size_t kFontSize = 18;

    /* Type representing a parsed CFG, or an error message about a CFG. */
    struct CFGOrError {
        shared_ptr<CFG::CFG> cfg;
        string               error;
    };

    /* Loads the CFG given by the description. */
    CFGOrError loadStudentCFG(const CFGInfo& info) {
        try {
            auto cfg = make_shared<CFG::CFG>(loadCFG(info.sectionHeader, info.alphabet));
            return { cfg, "" };
        } catch (const exception& e) {
            return { {}, e.what() };
        }
    }

    /* Renders a CFG in a snazzy style, or says why it can't. */
    const string kCFGError = R"(<span style="color:#800000"><b><i>%s</i></b></span>)";
    string styleCFG(const CFGOrError& r) {
        if (r.cfg == nullptr) {
            return format(kCFGError, r.error);
        }

        return cfgToHTML(*r.cfg);
    }

    const string kRow = R"(<tr>
            <td>
                %s
            </td>
            <td>
                %s
            </td>
            <td>
                %s
            </td>
    </tr>)";

    const string kHeader = R"(
    <tr>
      <th>Rule</th>
      <th>Application</th>
      <th>Result</th>
    </tr>)";

    const string kDeriveError  = R"(<tr><th colspan="3" style="font-color:#800000"><b><i>%s</i></b></th></tr>)";
    const string kNoDerivation = R"(<tr><th colspan="3"><i>%s</i></th></tr>)";

    string prettyString(const string& s) {
        return s.empty()? "&epsilon;" : s;
    }

    /* Highlights the given string of symbols. */
    string highlight(const vector<CFG::Symbol>& input, size_t start, size_t end) {
        ostringstream builder;
        for (size_t i = 0; i < input.size(); i++) {
            if (i < start || i >= end) builder << symbolToHTML(input[i], RenderType::FADE);
            else builder << symbolToHTML(input[i], RenderType::HIGHLIGHT);
        }
        return builder.str();
    }

    string styleResults(CFG::Deriver deriver, const string& input) {
        /* Don't do anything if the CFG failed to load. */
        if (!deriver) return "";

        try {
            auto derivation = deriver(input);
            if (derivation.empty()) return format(kNoDerivation, "Grammar does not derive " + prettyString(input));

            /* There is a derivation. Trace it out. */
            ostringstream result;
            result << kHeader;

            /* Derivation start. We can read the start symbol from the first step in the
             * derivation.
             */
            vector<CFG::Symbol> str = { CFG::nonterminal(derivation[0].first.nonterminal) };
            result << format(kRow, "", "Start", highlight(str, 0, 1));

            /* Trace the rest. */
            for (const auto& item: derivation) {
                string rule  = productionToHTML(item.first);
                string where = highlight(str, item.second, item.second + 1);

                /* Do the replacement. */
                str.erase(str.begin() + item.second);
                str.insert(str.begin() + item.second, item.first.replacement.begin(), item.first.replacement.end());

                string next  = highlight(str, item.second, item.second + item.first.replacement.size());
                result << format(kRow, rule, where, next);
            }

            return result.str();


        } catch (const exception& e) {
            return format(kDeriveError, string("Error: ") + e.what());
        }
    }

    void CFGDeriveGUI::updateDisplay() {
        stringstream content;
        content << format(kHTMLTemplate,
                          std::to_string(kFontSize),
                          messageHTML,
                          styleResults(deriver, input->getText()));

        console->readTextFromFile(content);
    }

    CFGInfo selectedCFG(const string& selected) {
        /* Find the one we want. */
        auto result = find_if(kCFGs.begin(), kCFGs.end(), [&](const CFGInfo& r) {
            return r.sectionHeader == selected;
        });
        if (result == kCFGs.end()) error("Internal error: Couldn't find CFG.");
        return *result;
    }

    void CFGDeriveGUI::updateCFG() {
        currCFG = selectedCFG(selector->getSelectedItem());
        auto result = loadStudentCFG(currCFG);

        messageHTML = styleCFG(result);
        if (result.cfg != nullptr) {
            deriver = CFG::deriverFor(*result.cfg);
        } else {
            deriver = nullptr;
        }

        updateDisplay();
    }

    void CFGDeriveGUI::changeOccurredIn(GObservable* source) {
        if (source == input) {
            updateDisplay();
        } else if (source == selector) {
            updateCFG();
        }
    }
}

GRAPHICS_HANDLER("See Derivations", GWindow& window) {
    return make_shared<CFGDeriveGUI>(window);
}

namespace {
    vector<string> allGrammars() {
        vector<string> result;
        for (const auto& entry: kCFGs) {
            result.push_back(entry.sectionHeader);
        }
        return result;
    }

    bool isInAlphabet(const string& input, const Languages::Alphabet& alphabet) {
        for (char32_t ch: utf8Reader(input)) {
            if (!alphabet.count(ch)) return false;
        }
        return true;
    }

    void printStep(const vector<char32_t>& sentence, size_t whereApplied) {
        for (size_t i = 0; i < sentence.size(); i++) {
            if (i == whereApplied) {
                cout << '[';
            }

            cout << toUTF8(sentence[i]);

            if (i == whereApplied) {
                cout << ']';
            }
        }
        cout << endl;
    }

    void printProduction(const CFG::Production& p) {
        cout << toUTF8(p.nonterminal) << " -> ";
        for (const auto& symbol: p.replacement) {
            cout << toUTF8(symbol.ch);
        }
        cout << endl;
    }

    void showDerivation(const CFG::Derivation& derivation) {
        if (derivation.empty()) error("Logic error: Can't show a nonexistent derivation.");

        cout << "Printing a derivation of that string. The nonterminal in brackets at each step "
             << "is the one the production rule is applied to." << endl;

        vector<char32_t> sentence = { derivation[0].first.nonterminal };
        for (size_t i = 0; i < derivation.size(); i++) {
            cout << "Current string: ";
            printStep(sentence, derivation[i].second);

            cout << "Applying production ";
            printProduction(derivation[i].first);

            /* Actually apply the production. */
            vector<char32_t> replacement;
            for (const auto& symbol: derivation[i].first.replacement) {
                replacement.push_back(symbol.ch);
            }

            sentence.erase(sentence.begin() + derivation[i].second);
            sentence.insert(sentence.begin() + derivation[i].second, replacement.begin(), replacement.end());
        }

        cout << "Final string: ";
        for (char32_t ch: sentence) {
            cout << toUTF8(ch);
        }
        cout << endl;
    }

    void deriveREPL(const Languages::Alphabet& alphabet, CFG::Deriver deriver) {
        do {
            string input = getLine("Enter a string: ");
            input.erase(remove(input.begin(), input.end(), ' '), input.end());

            if (!isInAlphabet(input, alphabet)) {
                cerr << "That input contains characters not found in the alphabet." << endl;
                continue;
            }

            auto derivation = deriver(input);
            if (derivation.empty()) {
                cout << "That string cannot be generated by this grammar." << endl;
            } else {
                showDerivation(derivation);
            }

        } while (getYesOrNo("Derive another string? "));
    }
}

CONSOLE_HANDLER("See Derivations") {
    cout << "This tool lets you load a grammar, enter test strings, and then see "
            "whether those strings are derivable from the start symbol of the grammar. "
            "If so, the tool will show you one possible derivation of the string." << endl;
    do {
        auto options = allGrammars();
        int selection = makeSelectionFrom("Choose a CFG: ", options);

        auto cfg = loadStudentCFG(kCFGs[selection]);
        if (cfg.cfg == nullptr) {
            cerr << "Error loading CFG: " << cfg.error << endl;
        } else {
            auto deriver = CFG::deriverFor(*cfg.cfg);
            deriveREPL(kCFGs[selection].alphabet, deriver);
        }
    } while (getYesOrNo("See derivations from another CFG? "));
}
