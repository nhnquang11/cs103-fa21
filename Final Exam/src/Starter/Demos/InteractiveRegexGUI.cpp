#include "../GUI/MiniGUI.h"
#include "Utilities/JSON.h"
#include "../FormalLanguages/RegexParser.h"
#include "../FormalLanguages/Automaton.h"
#include "../FileParser/FileParser.h"
#include "gbrowserpane.h"
#include "filelib.h"
#include "strlib.h"
#include <vector>
#include <unordered_map>
using namespace std;

namespace {
    /* Information about a regex. */
    struct RegexInfo {
        string filename;
        string sectionHeader;
        Languages::Alphabet alphabet;
    };

    string toString(const RegexInfo& regex) {
        return regex.filename + "/" + regex.sectionHeader;
    }

    Languages::Alphabet toAlphabet(const string& str) {
        return { str.begin(), str.end() };
    }

    const vector<RegexInfo> kRegexes = {
        { "FlightlessBirds.regexes", "[Flightless Birds]",   toAlphabet("moa") },
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

    class InteractiveRegexGUI: public ProblemHandler {
    public:
        InteractiveRegexGUI(GWindow& window);

        void changeOccurredIn(GObservable* source) override;

    private:
        /* Central console. */
        Temporary<GBrowserPane> console;

        /* Side panels for input */
        Temporary<GLabel>    sidePanelLabel;
        Temporary<GTextArea> sidePanel;
        Temporary<GLabel>    sidePanelNoLabel;
        Temporary<GTextArea> sidePanelNo;

        /* Regex selector at the bottom. */
        Temporary<GComboBox> selector;

        /* Current automaton, or nullptr if there was an error */
        shared_ptr<Automata::NFA> nfa;

        /* Display message - either an error or the actual regex. */
        string messageHTML;

        /* Currently-selected regex. */
        RegexInfo currRegex;

        /* Saved test cases. */
        unordered_map<string, string> pastTestCases;

        void updateRegex(bool firstTime);
        void updateDisplay();
        vector<TestCase> testCases();
        void saveTests();
        void loadTests();
    };

    InteractiveRegexGUI::InteractiveRegexGUI(GWindow& window) : ProblemHandler(window) {
        /* Combo box. */
        GComboBox* options = new GComboBox();
        for (const auto& option: kRegexes) {
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

        /* Configure display for the current (default) regex. */
        updateRegex(true);
    }

    /* Basic HTML format. */
    const string kHTMLTemplate =
R"(<html>
    <head>
    </head>
    <body style="color:black;background-color:white;font-size:%spt;">
    <table cellpadding="3" cellspacing="0" align="center">
    <tr>
      <th colspan="2">Interactive Regex Tester</th>
    </tr>
    <tr>
    <td colspan="2">
      Enter test cases into the text area to the right, with one test case per line.
      Each test case can either be a single string, or a string followed by a space and
      then the word <tt>yes</tt> or <tt>no</tt> to indicate whether it should be matched
      by the regex.
    <td>
    </tr>
    <tr>
      <th colspan="2">%s</th> <!-- Regex -->
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

    /* Type representing a parsed regex, or an error message about a regex. */
    struct RegexOrError {
        Regex::Regex regex;
        string       error;
    };

    /* Loads the regex given by the description. */
    RegexOrError loadRegex(const RegexInfo& info) {
        shared_ptr<istream> stream;
        try {
            stream = parseFile("res/" + info.filename).at(info.sectionHeader);
        } catch (const exception& e) {
            return { nullptr, "Could not find " + info.sectionHeader + " in file " + info.filename };
        }

        /* Duplicate the stream so we have a way of knowing what the raw text is. */
        stringstream clone;
        clone << stream->rdbuf();

        /* Read the regex. */
        Regex::Regex result;
        try {
            result = Regex::parse(Regex::scan(clone));
        } catch (const exception& e) {
            return { nullptr, e.what() };
        }

        /* Make sure the alphabet works here. */
        for (char32_t ch: Regex::coreAlphabetOf(result)) {
            if (!info.alphabet.count(ch)) {
                return { nullptr, "Regex uses character '" + toUTF8(ch) + "' as a character, but this isn't in the alphabet for this problem." };
            }
        }

        return { result, clone.str() };
    }

    /* Renders a regular expression in a snazzy style, or says why it can't. */
    const string kRegexError = R"(<span style="color:#800000"><b><i>%s</i></b></span>)";
    string styleRegex(const RegexOrError& r) {
        if (r.regex == nullptr) {
            return format(kRegexError, r.error);
        }

        ostringstream builder;
        builder << r.regex;
        return builder.str();
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
    string styleTestRow(const Automata::NFA& nfa, const TestCase& test, int row) {
        /* Confirm this string works with the alphabet. */
        if (test.input != "ε") {
            istringstream input(test.input);
            while (input.peek() != EOF) {
                char32_t ch = readChar(input);
                if (!nfa.alphabet.count(ch)) {
                    return format(kTestRow, styleFor(row), test.input, "Illegal character: \"" + toUTF8(ch) + "\"");
                }
            }
        }

        /* Run the tests. */
        auto result = Automata::accepts(nfa, test.input == "ε" ? "" : test.input);

        /* Report results. */
        if ((result && test.expected == Expected::FALSE) || (!result && test.expected == Expected::TRUE)) {
            return format(kTestRow, styleFor(row), test.input, format(kFailedResult, toString(result), toString(test.expected)));
        }  else {
            return format(kTestRow, styleFor(row), test.input, toString(result));
        }
    }

    /* Runs the tests, styling the results. */
    string styleResults(shared_ptr<Automata::NFA> nfa, const vector<TestCase>& testCases) {
        /* Could be that there was an error loading things. If so, do nothing. */
        if (!nfa) return "";

        string result;
        int row = 0;
        for (const auto& entry: testCases) {
            result += styleTestRow(*nfa, entry, row);
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

    vector<TestCase> toTestCases(istream& input) {
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

    /* Returns all the test cases typed in by the user. */
    vector<TestCase> InteractiveRegexGUI::testCases() {
        /* Read all the text from the side panel one line at a time, building up the result. */
        istringstream input(sidePanel->getText());
        return toTestCases(input);
    }

    void InteractiveRegexGUI::updateDisplay() {
        stringstream content;
        content << format(kHTMLTemplate,
                          std::to_string(kFontSize),
                          messageHTML,
                          styleResults(nfa, testCases()));

        console->readTextFromFile(content);
    }

    RegexInfo selectedRegex(const string& selected) {
        /* Tokenize filename/section. */
        auto tokens = stringSplit(selected, "/");
        if (tokens.size() != 2) error("Internal error: Expected two tokens, found " + to_string(tokens.size()));

        /* Find the one we want. */
        auto result = find_if(kRegexes.begin(), kRegexes.end(), [&](const RegexInfo& r) {
            return r.filename == tokens[0] && r.sectionHeader == tokens[1];
        });
        if (result == kRegexes.end()) error("Internal error: Couldn't find regex.");
        return *result;
    }

    void InteractiveRegexGUI::updateRegex(bool firstTime) {
        /* Save the user's tests. */
        if (!firstTime) {
            pastTestCases[toString(currRegex)] = sidePanel->getText();
        }

        currRegex = selectedRegex(selector->getSelectedItem());
        auto result = loadRegex(currRegex);

        messageHTML = styleRegex(result);
        if (result.regex != nullptr) {
            nfa = make_shared<Automata::NFA>(Automata::fromRegex(result.regex, currRegex.alphabet));
        } else {
            nfa.reset();
        }

        /* Load tests for this section. */
        sidePanel->setText(pastTestCases[toString(currRegex)]);

        updateDisplay();
    }

    void InteractiveRegexGUI::changeOccurredIn(GObservable* source) {
        if (source == sidePanel) {
            saveTests();
            updateDisplay();
        } else if (source == selector) {
            saveTests();
            updateRegex(false);
        }
    }

    void saveTests(const unordered_map<string, string>& testCases) {
        vector<JSON> entries;
        for (const auto& entry: testCases) {
            entries.push_back(JSON::array(entry.first, entry.second));
        }

        ofstream out("res/tests/saved-regex-tests");
        out << JSON(entries);
    }

    /* File format: a JSON of [[sectionName, tests], ...] */
    void InteractiveRegexGUI::saveTests() {
        /* Stash the current tests. */
        pastTestCases[toString(currRegex)] = sidePanel->getText();

        ::saveTests(pastTestCases);
    }

    unordered_map<string, string> loadTests() {
        try {
            /* Read the JSON data. */
            ifstream in("res/tests/saved-regex-tests");
            JSON data = JSON::parse(in);
            if (data.type() != JSON::Type::ARRAY) error("JSON isn't an array.");

            /* Confirm each section header is legit. */
            unordered_map<string, string> result;
            for (JSON entry: data) {
                if (entry.type() != JSON::Type::ARRAY) error("JSON entry isn't an array.");
                if (entry.size() != 2) error("JSON entry isn't an array of size two.");

                string section = entry[0].asString();
                if (find_if(kRegexes.begin(), kRegexes.end(), [&](const RegexInfo& i) {
                    return toString(i) == section;
                }) == kRegexes.end()) {
                    error("Section is not a regex.");
                }

                if (result.count(entry[0].asString())) error("Duplicate section.");

                result[entry[0].asString()] = entry[1].asString();
            }

            return result;
        } catch (const exception& e) {
            /* Gracefully degrade to doing nothing. */
            return {};
        }
    }

    void InteractiveRegexGUI::loadTests() {
        pastTestCases = ::loadTests();
    }
}

GRAPHICS_HANDLER("Regex Tester", GWindow& window) {
    return make_shared<InteractiveRegexGUI>(window);
}

namespace {
    /* Does NOT include the res/ prefix. */
    vector<string> allRegexFiles() {
        vector<string> result;
        for (const auto& entry: kRegexes) {
            result.push_back(entry.filename + "/" + entry.sectionHeader);
        }
        return result;
    }

    struct AllDoneNow{};

    /* Arguments to a REPL command. */
    struct REPLData {
        /* Keys here do NOT include the res/ prefix. */

        /* All tests. */
        unordered_map<string, string> testCases;

        Regex::Regex regex;

        /* The current regex. */
        string currRegex;

        /* Its alphabet. */
        Languages::Alphabet alphabet;
    };

    /* REPL command. */
    struct Command {
        string name;
        string desc;
        int arity;
        function<void(REPLData&, const Vector<string>&)> command;
    };

    void helpFN(REPLData&, const Vector<string>&);

    void quitFN(REPLData&, const Vector<string>&) {
        throw AllDoneNow();
    }

    void runFN(REPLData& data, const Vector<string>&) {
        /* Get all local tests. */
        istringstream input(data.testCases[data.currRegex]);
        auto tests = toTestCases(input);

        auto nfa = Automata::fromRegex(data.regex, data.alphabet);

        cout << "There " << (tests.size() == 1? "is one custom test case" : "are " + to_string(tests.size()) + " custom test cases") << " for this automaton." << endl;
        for (const auto& test: tests) {
            /* Convert epsilons back to empty strings. */
            string input = test.input;
            if (input == "ε") input = "";

            auto result = Automata::accepts(nfa, input);

            cout << "Input:   " << test.input << endl;
            cout << "Matched? " << boolalpha << result << endl;

            /* See if we have a mismatch. */
            bool isError = (test.expected == Expected::TRUE  && !result) ||
                           (test.expected == Expected::FALSE && result);

            if (isError) {
                cerr << "  Error: The regex should have " << (!result? "matched" : "not matched") << " this input." << endl;
            }
        }
    }

    /* Converts a list of test cases into a string. */
    string fromTestCases(const vector<TestCase>& testCases) {
        ostringstream builder;

        for (const auto& test: testCases) {
            /* Convert epsilons back to empty strings. */
            string input = test.input;
            if (input == "ε") input = "";

            builder << input << " ";
            if (test.expected == Expected::FALSE) {
                builder << "no";
            } else if (test.expected == Expected::TRUE) {
                builder << "yes";
            }
            builder << endl;
        }

        return builder.str();
    }

    void newTestFN(REPLData& data, const Vector<string>&) {
        string input = getLine("Enter the string you would like to use as the new test case. To test the regex on the empty string, just hit ENTER. ");

        /* Validate input. */
        for (char32_t ch: utf8Reader(input)) {
            if (!data.alphabet.count(ch)) {
                cerr << "Error: Character " << toUTF8(ch) << " is not in this regex's alphabet." << endl;
                return;
            }
        }

        /* Make sure this test isn't a duplicate. */
        istringstream in(data.testCases[data.currRegex]);
        auto tests = toTestCases(in);

        for (const auto& test: tests) {
            if (test.input == input) {
                cerr << "There is already a test for this string." << endl;
                return;
            }
        }

        /* Okay, it's a legit test case. Fill in the details. */
        bool accepts = getYesOrNo("Should the automaton accept this string? ");
        tests.push_back({ input, accepts? Expected::TRUE : Expected::FALSE });

        data.testCases[data.currRegex] = fromTestCases(tests);
    }

    void delTestFN(REPLData& data, const Vector<string>&) {
        istringstream input(data.testCases[data.currRegex]);
        auto cases = toTestCases(input);

        if (cases.empty()) {
            cerr << "There are no test cases to remove." << endl;
            return;
        }

        vector<string> options = { "(Cancel)" };
        for (const auto& test: cases) {
            options.push_back(test.input == ""? "ε" : test.input);
        }

        int choice = makeSelectionFrom("Choose which test to remove: ", options);
        if (choice == 0) {
            cout << "Option cancelled; nothing removed." << endl;
            return;
        }

        cases.erase(cases.begin() + (choice - 1));
        data.testCases[data.currRegex] = fromTestCases(cases);

        cout << "Removed test case " << options[choice] << endl;
    }

    void printFN(REPLData& data, const Vector<string>&) {
        istringstream input(data.testCases[data.currRegex]);
        for (const auto& test: toTestCases(input)) {
            cout << "Input:    " << (test.input == ""? "ε" : test.input) << endl;
            cout << "Expected: ";
            if (test.expected == Expected::TRUE) {
                cout << "Match" << endl;
            } else if (test.expected == Expected::FALSE) {
                cout << "Don't Match" << endl;
            } else {
                cout << "No expected behavior." << endl;
            }
        }
    }

    const vector<Command> kCommands = {
        { "help",          "help: Displays the help menu.",  0, helpFN },
        { "quit",          "quit: Exits the tester.",         0, quitFN },
        { "print",         "print: List, but don't run, all tests", 0, printFN },
        { "run",           "run: Runs your custom tests.",  0, runFN  },
        { "newtest",       "newtest: Prompts you to enter a new test case.", 0, newTestFN },
        { "deltest",       "deltest: Prompts you to delete a test case.", 0, delTestFN }
    };

    void helpFN(REPLData&, const Vector<string>&){
        for (const auto& command: kCommands) {
            cout << command.desc << endl;
        }
    }

    void regexREPL(const RegexInfo& info, Regex::Regex regex, unordered_map<string, string>& testCases) {
        REPLData data;
        data.testCases = testCases;
        data.regex = regex;
        data.currRegex = info.filename + "/" + info.sectionHeader;
        data.alphabet = info.alphabet;

        try {
            cout << "Type 'help' for a list of commands." << endl;
            cout << "Your changes will be saved when you type 'quit.' If you exit the program "
                 << "manually, your changes will not be saved." << endl;
            while (true) {
                Vector<string> command = stringSplit(trim(getLine("Enter command: ")), " ");
                if (!command.isEmpty()) {
                    string verb = toLowerCase(command[0]);
                    Vector<string> args = command.subList(1);

                    auto itr = find_if(kCommands.begin(), kCommands.end(), [&](const Command& c) {
                        return c.name == verb;
                    });

                    if (itr != kCommands.end()) {
                        if (itr->arity == args.size()) {
                            itr->command(data, args);
                        } else {
                            cerr << "Command '" << verb << "' requires " << pluralize(itr->arity, "argument") << "; you provided " << args.size() << endl;
                        }
                    } else {
                        cerr << "Unknown command: " << command[0] << endl;
                    }
                }
            }
        } catch (const AllDoneNow&) {
            /* Normal exit. */
        }

        /* Copy this out. */
        testCases = data.testCases;
    }

    void textTestRegex(const RegexInfo& info, unordered_map<string, string>& testCases) {
        auto result = loadRegex(info);
        if (result.regex == nullptr) {
            cerr << "Error loading regex: " << result.error << endl;
            return;
        }

        regexREPL(info, result.regex, testCases);
        saveTests(testCases);
    }
}

CONSOLE_HANDLER("Regex Tester") {
    auto tests = loadTests();
    do {
        auto regexes = allRegexFiles();
        textTestRegex(kRegexes[makeSelectionFrom("Choose a regex: ", regexes)], tests);
    } while (getYesOrNo("Test another regex? "));
}
