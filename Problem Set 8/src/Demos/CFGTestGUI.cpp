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

    /* Each test case either expects true, expects false, or doesn't expect anything. */
    enum class Expected {
        TRUE,
        FALSE,
        NOTHING
    };

    /* Type representing a single test case. */
    struct TestCase {
        string   input;    // The test case
        Expected expected; // What we expect to find
    };

    class CFGDeriveGUI: public ProblemHandler {
    public:
        CFGDeriveGUI(GWindow& window);

        void changeOccurredIn(GObservable* source) override;

    private:
        /* Central console. */
        Temporary<GBrowserPane> console;

        /* Side panels for input */
        Temporary<GLabel>    sidePanelLabel;
        Temporary<GTextArea> sidePanel;
        Temporary<GLabel>    sidePanelNoLabel;
        Temporary<GTextArea> sidePanelNo;

        /* CFG selector at the bottom. */
        Temporary<GComboBox> selector;

        /* Current matcher, or nullptr if there was an error */
        CFG::Matcher matcher;
        Languages::Alphabet alphabet;

        /* Display message - either an error or the actual CFG. */
        string messageHTML;

        /* Currently-selected CFG. */
        CFGInfo currCFG;

        /* Saved test cases. */
        unordered_map<string, string> pastTestCases;

        void updateCFG(bool firstTime);
        void updateDisplay();
        vector<TestCase> testCases();
        void saveTests();
        void loadTests();
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
        sidePanelLabel = make_temporary<GLabel>(window, "WEST", "Test Strings");
        sidePanel      = make_temporary<GTextArea>(window, "WEST");

        /* Central panel. */
        console           = make_temporary<GBrowserPane>(window, "CENTER");

        /* Load stored tests. */
        loadTests();

        /* Configure display for the current (default) CFG. */
        updateCFG(true);
    }

    /* Basic HTML format. */
    const string kHTMLTemplate =
R"(<html>
    <head>
    </head>
    <body style="color:black;background-color:white;font-size:%spt;">
    <h1>
        Interactive CFG Tester
    </h1>
    <p>
        Enter test cases into the two text areas to the right, with one test case per line.
        Each test case can either be a single string, or a string followed by a space and
        then the word <tt>yes</tt> or <tt>no</tt> to indicate whether it should be derivable
        by the grammar.
    </p>
    <p>
        If you would like to see how your CFG derives a particular string, choose the "See Derivations"
        option from the top menu.
    </p>
    <table cellpadding="3" cellspacing="0" align="center" style="width:100%">
    <tr>
      <th colspan="2">%s</th> <!-- CFG -->
    </tr>
    <tr>
      <th>String</th>
      <th>Matched</th>
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

    string toString(Expected e) {
        switch (e) {
            case Expected::TRUE:    return "true";
            case Expected::FALSE:   return "false";
            case Expected::NOTHING: return "nothing";
            default: error("Unknown Expected.");
        }
    }

    /* Style for a row - makes things look all zebralike! */
    string styleFor(int row) {
        return format("background-color:%s;border: 3px solid black; border-collapse:collapse;",
                      row % 2 == 0? "#ffff80" : "white");
    }

    const string kTestRow = R"(<tr style="%s">
            <td>
                %s
            </td>
            <td>
                %s
            </td>
    </tr>)";

    const string kFailedResult = R"(%s <span style="color="#404040;">(expected %s)</span>)";

    string toString(bool b) {
        return b? "true" : "false";
    }

    /* Runs one test, styling the result. */
    string styleTestRow(CFG::Matcher matcher, const Languages::Alphabet& alphabet, const TestCase& test, int row) {
        /* Confirm this string works with the alphabet. */
        if (test.input != "ε") {
            istringstream input(test.input);
            while (input.peek() != EOF) {
                char32_t ch = readChar(input);
                if (!alphabet.count(ch)) {
                    return format(kTestRow, styleFor(row), test.input, "Illegal character: \"" + toUTF8(ch) + "\"");
                }
            }
        }

        /* Run the tests. */
        auto result = matcher(test.input == "ε" ? "" : test.input);

        /* Report results. */
        if ((result && test.expected == Expected::FALSE) || (!result && test.expected == Expected::TRUE)) {
            return format(kTestRow, styleFor(row), test.input, format(kFailedResult, toString(result), toString(test.expected)));
        }  else {
            return format(kTestRow, styleFor(row), test.input, toString(result));
        }
    }

    /* Runs the tests, styling the results. */
    string styleResults(CFG::Matcher matcher, const Languages::Alphabet& alphabet, const vector<TestCase>& testCases) {
        /* Could be that there was an error loading things. If so, do nothing. */
        if (!matcher) return "";

        string result;
        int row = 0;
        for (const auto& entry: testCases) {
            result += styleTestRow(matcher, alphabet, entry, row);
            ++row;
        }
        return result;
    }

    string translateToTest(string input) {
        /* Begin by stripping out whitespace. */
        input.erase(remove(input.begin(), input.end(), ' '), input.end());

        /* If the test is just the empty string, then replace it with epsilon. */
        if (input.empty()) {
            input = "ε";
        }
        /* Otherwise, if it's literally some flavor of epsilon, replace it with the
         * canonical epsilon.
         */
        else if (input == "ε" || input == "ϵ") {
            input = "ε";
        }
        return input;
    }

    /* Returns all the test cases typed in by the user. */
    vector<TestCase> CFGDeriveGUI::testCases() {
        /* Read all the text from the side panel one line at a time, building up the result. */
        istringstream input(sidePanel->getText());
        vector<TestCase> result;

        for (string line; getline(input, line); ) {
            /* Don't skip blank lines - that could correspond to the empty string! */

            /* If there's a space, it might indicate that there's an expected result at the end.
             * To address this, we look for the LAST space, which is the one that would do the
             * separating.
             */
            size_t lastSpace = line.rfind(' ');
            if (lastSpace == string::npos) {
                result.push_back({ translateToTest(line), Expected::NOTHING });
            } else {
                string last = toLowerCase(line.substr(lastSpace + 1));
                if (last == "y" || last == "yes" || last == "true" || last == "t" || last == "accept" || last == "match") {
                    result.push_back({ translateToTest(line.substr(0, lastSpace)), Expected::TRUE });
                } else if (last == "n" || last == "no" || last == "false" || last == "f" || last == "reject") {
                    result.push_back({ translateToTest(line.substr(0, lastSpace)), Expected::FALSE });
                } else {
                    result.push_back({ translateToTest(line), Expected::NOTHING });
                }
            }
        }

        return result;
    }

    void CFGDeriveGUI::updateDisplay() {
        stringstream content;
        content << format(kHTMLTemplate,
                          std::to_string(kFontSize),
                          messageHTML,
                          styleResults(matcher, alphabet, testCases()));

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

    void CFGDeriveGUI::updateCFG(bool firstTime) {
        /* Save the user's tests. */
        if (!firstTime) {
            pastTestCases[toString(currCFG)] = sidePanel->getText();
        }

        currCFG = selectedCFG(selector->getSelectedItem());
        auto result = loadStudentCFG(currCFG);

        messageHTML = styleCFG(result);
        if (result.cfg != nullptr) {
            matcher = CFG::matcherFor(*result.cfg);
            alphabet = result.cfg->alphabet;
        } else {
            matcher = nullptr;
            alphabet = {};
        }

        /* Load tests for this section. */
        sidePanel->setText(pastTestCases[toString(currCFG)]);

        updateDisplay();
    }

    void CFGDeriveGUI::changeOccurredIn(GObservable* source) {
        if (source == sidePanel) {
            saveTests();
            updateDisplay();
        } else if (source == selector) {
            saveTests();
            updateCFG(false);
        }
    }

    /* File format: a JSON of [[sectionName, tests], ...] */
    void CFGDeriveGUI::saveTests() {
        /* Stash the current tests. */
        pastTestCases[toString(currCFG)] = sidePanel->getText();

        vector<JSON> entries;
        for (const auto& entry: pastTestCases) {
            entries.push_back(JSON::array(entry.first, entry.second));
        }

        ofstream out("res/tests/saved-cfg-tests");
        out << JSON(entries);
    }

    void CFGDeriveGUI::loadTests() {
        try {
            /* Read the JSON data. */
            ifstream in("res/tests/saved-cfg-tests");
            JSON data = JSON::parse(in);
            if (data.type() != JSON::Type::ARRAY) error("JSON isn't an array.");

            /* Confirm each section header is legit. */
            unordered_map<string, string> result;
            for (JSON entry: data) {
                if (entry.type() != JSON::Type::ARRAY) error("JSON entry isn't an array.");
                if (entry.size() != 2) error("JSON entry isn't an array of size two.");

                string section = entry[0].asString();
                if (find_if(kCFGs.begin(), kCFGs.end(), [&](const CFGInfo& i) {
                    return toString(i) == section;
                }) == kCFGs.end()) {
                    error("Section is not a CFG.");
                }

                if (result.count(entry[0].asString())) error("Duplicate section.");

                result[entry[0].asString()] = entry[1].asString();
            }

            pastTestCases = result;
        } catch (const exception& e) {
            /* Gracefully degrade to doing nothing. */
            return;
        }
    }
}

GRAPHICS_HANDLER("CFG Tester", GWindow& window) {
    return make_shared<CFGDeriveGUI>(window);
}
