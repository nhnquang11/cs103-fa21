#include "../GUI/MiniGUI.h"
#include "AutomataEditor.h"
#include "AutomataEditorCore.h"
#include "../FormalLanguages/Automaton.h"
#include "Utilities/JSON.h"
#include "Utilities/Unicode.h"
#include "gbrowserpane.h"
#include "filelib.h"
#include "strlib.h"
#include "gthread.h"
#include <vector>
#include <unordered_map>
using namespace std;

namespace {
    const string kBackgroundColor = "white";
    const string kTestCasesFilename = "res/tests/saved-automata-tests";

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

    class TestGUI: public ProblemHandler {
    public:
        TestGUI(GWindow& window);

        void changeOccurredIn(GObservable* source) override;
        void actionPerformed(GObservable* source) override;
        void windowResized() override;
        bool shuttingDown() override;

    protected:
        void repaint() override;

    private:
        /* Central console. */
        Temporary<GContainer> center;
          GBrowserPane* console;
          GCanvas*      automatonDisplay;

        /* Side panels for input */
        Temporary<GContainer> sideBox;
          GLabel* sidePanelLabel;
          GTextArea* sidePanel;

        /* Bottom controls. */
        Temporary<GButton>   loadButton;

        /* Current automaton, or nullptr if there was an error */
        shared_ptr<Editor::Automaton> editor; // TODO: Viewer!
        shared_ptr<Automata::NFA> nfa;
        string currFilename;

        /* Errors during loading. */
        vector<string> errors;

        /* Saved test cases.
         *
         * Filename -> Test Cases
         */
        unordered_map<string, string> pastTestCases;

        enum class UIState {
            DISABLED,
            ENABLED,
            BAD_AUTOMATON
        };
        UIState state = UIState::DISABLED;

        void resizeComponents();

        void updateDisplay();
        void guiSaveTests();
        void guiLoadTests();

        void userLoadAutomaton();
        void loadAutomaton(const string& filename);

        void setState(UIState state);
    };

    TestGUI::TestGUI(GWindow& window) : ProblemHandler(window) {
        GThread::runOnQtGuiThread([&] {
            /* Side panels. */
            sideBox = make_temporary<GContainer>(window, "WEST", GContainer::LAYOUT_FLOW_VERTICAL);
            sidePanelLabel = new GLabel("Test Strings");
            sidePanel      = new GTextArea();
            sideBox->add(sidePanelLabel);
            sideBox->add(sidePanel);

            /* Central panel. */
            center  = make_temporary<GContainer>(window, "CENTER", GContainer::LAYOUT_GRID);
            console = new GBrowserPane();
            automatonDisplay = new GCanvas();
            automatonDisplay->setRepaintImmediately(false);

            center->addToGrid(console, 0, 0);
            center->addToGrid(automatonDisplay, 1, 0);

            loadButton = make_temporary<GButton>(window, "SOUTH", "Load Automaton");

            resizeComponents();
        });

        /* Load stored tests. */
        guiLoadTests();

        if (Core::lastFilename() != "") {
            loadAutomaton(Core::lastFilename());
        } else {
            setState(UIState::DISABLED);
        }

        updateDisplay();
    }

    void TestGUI::setState(UIState state) {
        this->state = state;

        /* Nothing loaded? Then do nothing. */
        if (state == UIState::DISABLED) {
            sideBox->setEnabled(false);
            center->setEnabled(true);
        }
        /* Enabled? Turn everything on. */
        else if (state == UIState::ENABLED) {
            sideBox->setEnabled(true);
            center->setEnabled(true);
        }
        /* Bad automaton? Only enable the center. */
        else if (state == UIState::BAD_AUTOMATON) {
            sideBox->setEnabled(false);
            center->setEnabled(true);
        }
        /* Oops. */
        else {
            error("Unknown UIState.");
        }
    }

    /* Basic HTML format. */
    const string kHTMLTemplate =
R"(<html>
    <head>
    </head>
    <body style="color:black;background-color:white;font-size:%spt;"> <!-- Font size -->
    <table cellpadding="3" cellspacing="0" align="center">
    <tr>
      <th colspan="2">Automaton Tester</th>
    </tr>
    <tr>
    <td colspan="2">
      Enter test cases into the text area to the right, with one test case per line.
      Each test case can either be a single string, or a string followed by a space and
      then the word <tt>yes</tt> or <tt>no</tt> to indicate whether it should be accepted
      by the automaton.
    </td>
    </tr>
    <tr>
      <th>String</th>
      <th>Accepted</th>
    </tr>
    %s <!-- Test Results -->
    </table>
    </body>
    </html>)";

    const string kWelcomeMessage =
R"(<html>
       <head></head>
            <body style="color:black;background-color:white;font-size:%spt;"> <!-- Font size -->
                <table cellpadding="3" cellspacing="0" align="center">
                <tr>
                  <th colspan="2">Automaton Tester</th>
                </tr>
                <tr>
                    <td colspan="2">
                      Welcome to the automaton tester! This tool will let you see how your automaton
                      processes different input strings, which is helpful for better understanding
                      how your automaton works.
                    </td>
                </tr>
                <tr>
                    <td colspan="2">
                      Use the "Load Automaton" button to select an automaton to test.
                    </td>
                </tr>
            </table>
        </body>
   </html>)";

    const string kErrorHTMLTemplate =
R"(<html>
       <head></head>
       <body style="color:black;background-color:white;font-size:%spt;"> <!-- Font size -->
       <p>
            We can't run tests because this is not a valid automaton. Please correct the
            following errors in the editor:
       </p>
       <ul>
            %s
       </ul>
       </body>
   </html>)";

    /* Global font size. */
    const size_t kFontSize = 18;

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

    const string kSingleError = "<li>%s</li>";

    string formatErrors(const vector<string>& errors) {
        ostringstream result;
        for (string error: errors) {
            result << format(kSingleError, error) << endl;
        }
        return result.str();
    }

    void TestGUI::updateDisplay() {
        if (state == UIState::DISABLED) {
            stringstream content;
            content << format(kWelcomeMessage,
                              std::to_string(kFontSize));

            console->readTextFromFile(content);
        } else if (state == UIState::ENABLED) {
            stringstream content;

            istringstream input(sidePanel->getText());

            content << format(kHTMLTemplate,
                              std::to_string(kFontSize),
                              styleResults(nfa, toTestCases(input)));

            console->readTextFromFile(content);
        } else if (state == UIState::BAD_AUTOMATON) {
            stringstream content;
            content << format(kErrorHTMLTemplate,
                              std::to_string(kFontSize),
                              formatErrors(errors));
            console->readTextFromFile(content);
        } else {
            error("Unknown UI state.");
        }
    }

    void TestGUI::changeOccurredIn(GObservable* source) {
        if (source == sidePanel) {
            updateDisplay();
        }
    }

    void TestGUI::actionPerformed(GObservable* source) {
        if (source == loadButton) {
            userLoadAutomaton();
        }
    }

    void TestGUI::loadAutomaton(const string& filename) {
        /* Stash filename for later. */
        currFilename = getTail(filename);
        Core::setLastFilename(filename);

        ifstream input(filename);
        editor = make_shared<Editor::Automaton>(JSON::parse(input));
        editor->setBounds({ 0, 0, automatonDisplay->getWidth(), automatonDisplay->getHeight() });

        nfa = make_shared<Automata::NFA>(editor->toNFA());

        /* Load tests for this section. */
        sidePanel->setText(pastTestCases[currFilename]);

        errors = editor->checkValidity();
        setState(errors.empty()? UIState::ENABLED : UIState::BAD_AUTOMATON);
        updateDisplay();
        requestRepaint();
    }

    void TestGUI::userLoadAutomaton() {
        /* Ask user to pick a file; don't do anything if they don't pick one. */
        string filename = GFileChooser::showOpenDialog(&window(), "Choose Automaton", "res/", "*.automaton");
        if (filename == "") return;

        /* Save tests; we're about to change automata. */
        guiSaveTests();

        loadAutomaton(filename);
    }

    void saveTests(const unordered_map<string, string>& testCases) {
        vector<JSON> entries;
        for (const auto& entry: testCases) {
            entries.push_back(JSON::array(entry.first, entry.second));
        }

        ofstream out(kTestCasesFilename);
        out << JSON(entries);
        if (!out) {
            error("Unable to save your test cases; please contact the course staff.");
        }
    }

    /* File format: a JSON of [[sectionName, tests], ...] */
    void TestGUI::guiSaveTests() {
        if (state != UIState::ENABLED) return;

        /* Stash the current tests. */
        pastTestCases[currFilename] = sidePanel->getText();

        saveTests(pastTestCases);
    }

    unordered_map<string, string> loadTests() {
        try {
            /* Read the JSON data. */
            ifstream in(kTestCasesFilename);
            JSON data = JSON::parse(in);
            if (data.type() != JSON::Type::ARRAY) error("JSON isn't an array.");

            /* Confirm each section header is legit. */
            unordered_map<string, string> result;
            for (JSON entry: data) {
                if (entry.type() != JSON::Type::ARRAY) error("JSON entry isn't an array.");
                if (entry.size() != 2) error("JSON entry isn't an array of size two.");

                string section = entry[0].asString();
                if (result.count(entry[0].asString())) error("Duplicate section.");

                result[entry[0].asString()] = entry[1].asString();
            }

            return result;
        } catch (const exception& e) {
            /* Gracefully degrade to doing nothing. */
            return {};
        }
    }

    void TestGUI::guiLoadTests() {
        pastTestCases = loadTests();
    }

    void TestGUI::repaint() {
        clearDisplay(automatonDisplay, kBackgroundColor);
        if (editor) {
            editor->draw(automatonDisplay, {}, {});
            automatonDisplay->repaint();
        }
    }

    void TestGUI::windowResized() {
        resizeComponents();
    }

    void TestGUI::resizeComponents() {
        double size = 0.9 * window().getHeight();
        GThread::runOnQtGuiThread([&] {
            console->setHeight(size * 0.45);
            automatonDisplay->setSize(console->getWidth(), size * 0.45);
            if (editor) editor->setBounds({ 0, 0, automatonDisplay->getWidth(), automatonDisplay->getHeight() });
        });
        requestRepaint();
    }

    bool TestGUI::shuttingDown() {
        guiSaveTests();
        return ProblemHandler::shuttingDown();
    }
}

GRAPHICS_HANDLER("Automaton Tester", GWindow& window) {
    return make_shared<TestGUI>(window);
}

/* Console testing interface. */

namespace {
    /* Does NOT include the res/ prefix. */
    vector<string> allAutomataFiles() {
        vector<string> result;
        for (string file: listDirectory("res/")) {
            if (endsWith(file, ".automaton")) {
                result.push_back(file);
            }
        }
        return result;
    }

    shared_ptr<Editor::Automaton> loadAutomaton(const string& filename) {
        ifstream input(filename);
        if (!input) error("Error opening file: " + filename);

        JSON j = JSON::parse(input);
        return make_shared<Editor::Automaton>(j);
    }

    struct AllDoneNow{};

    /* Arguments to a REPL command. */
    struct REPLData {
        /* Keys here do NOT include the res/ prefix. */

        /* All tests. */
        unordered_map<string, string> testCases;

        /* Current automaton. */
        shared_ptr<Editor::Automaton> automaton;

        /* The current automaton. */
        string currAutomaton;
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

    /* Converts subscripts to normal indices; used for better displaying on the console. */
    string desubscript(const string& str) {
        static const unordered_map<char32_t, char32_t> kTable = {
            { fromUTF8("₀"), '0' },
            { fromUTF8("₁"), '1' },
            { fromUTF8("₂"), '2' },
            { fromUTF8("₃"), '3' },
            { fromUTF8("₄"), '4' },
            { fromUTF8("₅"), '5' },
            { fromUTF8("₆"), '6' },
            { fromUTF8("₇"), '7' },
            { fromUTF8("₈"), '8' },
            { fromUTF8("₉"), '9' },
        };

        string result;
        for (char32_t ch: utf8Reader(str)) {
            result += toUTF8(kTable.count(ch)? kTable.at(ch) : ch);
        }

        return result;
    }

    void runFN(REPLData& data, const Vector<string>&) {
        /* Get all local tests. */
        istringstream input(data.testCases[data.currAutomaton]);
        auto tests = toTestCases(input);

        cout << "There " << (tests.size() == 1? "is one custom test case" : "are " + to_string(tests.size()) + " custom test cases") << " for this automaton." << endl;
        for (const auto& test: tests) {
            /* Convert epsilons back to empty strings. */
            string input = test.input;
            if (input == "ε") input = "";

            auto result = Automata::accepts(data.automaton->toNFA(), input);

            cout << "Input:    " << test.input << endl;
            cout << "Accepted? " << boolalpha << result << endl;

            /* See if we have a mismatch. */
            bool isError = (test.expected == Expected::TRUE  && !result) ||
                           (test.expected == Expected::FALSE && result);

            if (isError) {
                cerr << "  Error: The automaton should have " << (!result? "accepted" : "rejected") << " this input." << endl;
            }
        }
    }

    void newTestFN(REPLData& data, const Vector<string>&) {
        string input = getLine("Enter the string you would like to use as the new test case. To test the automaton on the empty string, just hit ENTER. ");

        /* Validate input. */
        for (char32_t ch: utf8Reader(input)) {
            if (!data.automaton->alphabet().count(ch)) {
                cerr << "Error: Character " << toUTF8(ch) << " is not in this automaton's alphabet." << endl;
                return;
            }
        }

        /* Make sure this test isn't a duplicate. */
        istringstream in(data.testCases[data.currAutomaton]);
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

        data.testCases[data.currAutomaton] = fromTestCases(tests);
    }

    void delTestFN(REPLData& data, const Vector<string>&) {
        istringstream input(data.testCases[data.currAutomaton]);
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
        data.testCases[data.currAutomaton] = fromTestCases(cases);

        cout << "Removed test case " << options[choice] << endl;
    }

    void printFN(REPLData& data, const Vector<string>&) {
        istringstream input(data.testCases[data.currAutomaton]);
        for (const auto& test: toTestCases(input)) {
            cout << "Input:    " << (test.input == ""? "ε" : test.input) << endl;
            cout << "Expected: ";
            if (test.expected == Expected::TRUE) {
                cout << "Accept" << endl;
            } else if (test.expected == Expected::FALSE) {
                cout << "Reject" << endl;
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

    void automatonREPL(const string& filename, shared_ptr<Editor::Automaton> automaton, unordered_map<string, string>& testCases) {
        REPLData data;
        data.testCases = testCases;
        data.automaton = automaton;
        data.currAutomaton = filename;

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

    void textTestAutomaton(const string& filename, unordered_map<string, string>& testCases) {
        auto automaton = loadAutomaton("res/" + filename);

        /* Can't bulk-test an automaton with errors. */
        auto errors = automaton->checkValidity();
        if (!errors.empty()) {
            cout << "This automaton is invalid. Please correct these errors in the editor:" << endl;
            for (string error: errors) {
                cout << desubscript(error) << endl;
            }
            return;
        }

        automatonREPL(filename, automaton, testCases);
        saveTests(testCases);
    }
}

CONSOLE_HANDLER("Automaton Tester") {
    auto tests = loadTests();
    do {
        auto automata = allAutomataFiles();
        textTestAutomaton(automata[makeSelectionFrom("Choose an automaton: ", automata)], tests);
    } while (getYesOrNo("Test another automaton? "));
}
