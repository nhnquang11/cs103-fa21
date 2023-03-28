#include "../GUI/MiniGUI.h"
#include "AutomataEditor.h"
#include "AutomataEditorCore.h"
#include "../FormalLanguages/Automaton.h"
#include "../FormalLanguages/Languages.h"
#include "../GraphEditor/GVector.h"
#include "Utilities/Unicode.h"
#include "gtimer.h"
#include "filelib.h"
#include <fstream>
#include <memory>
#include <unordered_map>
using namespace std;
using namespace MiniGUI;

namespace {
    const string kBackgroundColor = "white";
    const string kActiveStateColor = "#ffd320"; // Slide highlight color
    const string kAcceptStateColor = "#afd095"; // Celadon-ish
    const string kRejectStateColor = "#ffa6a6"; // Salmon-ish

    /* Default size for one character, as a fraction of the width of the screen. */
    const double kDefaultCharSize = 48.0 / 1000;

    const Font kCharFont(FontFamily::UNICODE_MONOSPACE, FontStyle::BOLD, 24, "black");

    const string kCharBackgroundColor = "#ffffa6"; // Slide's color
    const string kCharBorderColor = "black";

    /* Vectors making up the "current position" arrow. Coordinates are relative
     * to the bounding box making up the arrow.
     */
    const vector<GVector> kArrow = {
        { 0.5,   0 },
        {   0, 0.5 },
        { 0.3, 0.5 },
        { 0.3,   1 },
        { 0.7,   1 },
        { 0.7, 0.5 },
        { 1.0, 0.5 }
    };
    const string kArrowColor = "black";

    /* Aspect ratio. */
    const double kAspectRatio = 5.0 / 3.0;

    /* Fraction of the window height occupied by the automaton. */
    const double kAutomatonHeight = 0.85;
    const double kCharY           = 0.9 / kAspectRatio; // 90% of the way down.
    const double kCharHeight      = 0.05 / kAspectRatio;

    /* Animation controls. */
    const double kAnimationSpeed = 750; //ms

    const string kWelcome = R"(Welcome to the Automaton Debugger!

    This tool lets you single-step through an automaton to better understand how it works.

    Click "Load Automaton" to choose an automaton.)";

    const Font kWelcomeFont(FontFamily::SERIF, FontStyle::BOLD_ITALIC, 24, "#4C5866"); // Marengo

    const string kSingleErrorMessageTemplate = "%s.\n";
    const string kErrorMessageTemplate = "This automaton is not valid and therefore cannot be debugged. Please correct the following errors in the editor:\n%s";

    const Font kErrorFont(FontFamily::SERIF, FontStyle::NORMAL, 24, "#960018"); // Carmine

    class DebugGUI: public ProblemHandler {
    public:
        DebugGUI(GWindow& window);

        void actionPerformed(GObservable* source) override;
        void timerFired() override;
        void windowResized() override;
        void changeOccurredIn(GObservable* source) override;

    protected:
        void repaint() override;

    private:
        /* Main panel. */
        GContainer* mainPanel;
          GContainer* sidePanel;
            GLabel*  currAutomaton;
            GButton* loadButton;
          GContainer* inputPanel;
            GTextField* inputField = nullptr;
            GButton*    startButton;

        /* Debug panel. */
        GContainer* debugPanel;
          GButton* toBeginning;
          GButton* previous;
          GButton* playPause;
          GButton* next;
          GButton* toEnd;
          GButton* stop;

        /* Empty panel. */
        GContainer* emptyPanel;

        /* Bounding rectangle. */
        GRectangle contentBounds;

        /* Displayed panel. */
        Temporary<GContainer> controlPanel;

        /* Currently-loaded editor and automaton, if any. */
        shared_ptr<Editor::Automaton> editor; // TODO: Viewer, not editor.
        shared_ptr<Automata::NFA>  nfa;       // Always use NFA; handles both DFAs and NFAs.

        /* Current set of active states. */
        unordered_set<Editor::State*> active;

        /* Length of string read in so far. */
        size_t index;

        /* String to be debugged. */
        vector<char32_t> debugStr;

        /* Errors that occurred during loading. */
        vector<string> errors;

        /* Current filename. */
        string currFilename = "IfYouSeeThisContactKeith";

        /* Currently-selected panel. */
        GContainer* currPanel = nullptr;

        /* Active timer, if any
         *
         * TODO: GTimer is still buggy, internally, so we're just going to leak memory.
         * This should be fixed once GTimer race conditions are sorted out.
         */
        GTimer* timer = nullptr;

        /* Which UI state we're in. */
        enum class UIState {
            NO_AUTOMATON,      // Nothing yet loaded
            BAD_AUTOMATON,     // Automaton loaded, but is invalid
            STRING_INPUT_GOOD, // Automaton loaded, is valid, waiting for a string, string is good
            STRING_INPUT_BAD,  // Automaton loaded, is valid, waiting for a string, string is bad.
            DEBUG_PAUSE,       // Debugging, but paused
            DEBUG_PLAY         // Debugging, and playing
        } uiState = UIState::NO_AUTOMATON;

        /* Change the UI state. */
        void setState(UIState state);

        /* Changes the controls. Don't call this directly; use setState instead. */
        void setControls(GContainer* controls);
        void controlsEmptyPanel();
        void controlsInputPanel();

        /* Input validation. */
        void checkInput();
        vector<char32_t> inputString();

        /* Loads an automaton. */
        void userLoadAutomaton();
        void loadAutomaton(const string& filename);

        /* Debugger controls. */
        void resetDebugger();
        void seekDebuggerTo(size_t index);
        void play();
        void pause();

        /* Graphics routines. */
        GRectangle automatonArea();
        GRectangle textArea();
        void drawAutomatonWithActiveStates();
        void drawDebuggedString();
        void drawArrow(const GRectangle& bounds);
        void drawSingleCharacter(char32_t ch, const GRectangle& bounds);
        void recomputeBounds();
        void showWelcomeMessage();
        void showErrors();
        string automatonErrors();

        GRectangle worldToGraphics(const GRectangle& pt);

        /* Bugs in the Qt system. :-) */
        void handleInputBug();
    };

    DebugGUI::DebugGUI(GWindow& window) : ProblemHandler(window) {
        sidePanel = new GContainer(GContainer::LAYOUT_FLOW_VERTICAL);
          currAutomaton = new GLabel("Choose an Automaton");
          loadButton    = new GButton("Load Automaton");
          sidePanel->add(currAutomaton);
          sidePanel->add(loadButton);

        inputPanel = new GContainer();
        handleInputBug();


        emptyPanel = new GContainer();

        mainPanel = new GContainer(GContainer::LAYOUT_GRID);
        mainPanel->addToGrid(sidePanel, 0, 0);
        mainPanel->addToGrid(emptyPanel, 0, 1, 1, 3);

        debugPanel = new GContainer();
          toBeginning = new GButton("⏮");
          previous    = new GButton("⏪");
          playPause   = new GButton("▶");
          next        = new GButton("⏩");
          toEnd       = new GButton("⏭️");
          stop        = new GButton("⏹");
          debugPanel->add(toBeginning);
          debugPanel->add(previous);
          debugPanel->add(playPause);
          debugPanel->add(next);
          debugPanel->add(toEnd);
          debugPanel->add(stop);

        /* TODO: Bug: interactors appear in the main window unless explicitly hidden. */
        sidePanel->setVisible(false);
        inputPanel->setVisible(false);
        emptyPanel->setVisible(false);
        mainPanel->setVisible(false);
        debugPanel->setVisible(false);

        controlPanel = make_temporary<GContainer>(window, "SOUTH", GContainer::LAYOUT_GRID);
        controlPanel->setWidth(window.getWidth() * 0.95);

        if (Core::lastFilename() != "") {
            loadAutomaton(Core::lastFilename());
        } else {
            setState(UIState::NO_AUTOMATON);
        }

        recomputeBounds();
    }

    void DebugGUI::setState(UIState state) {
        if (state == UIState::NO_AUTOMATON) {
            controlsEmptyPanel();
        } else if (state == UIState::BAD_AUTOMATON) {
            controlsEmptyPanel();
        } else if (state == UIState::STRING_INPUT_GOOD) {
            controlsInputPanel();
            startButton->setEnabled(true);
        } else if (state == UIState::STRING_INPUT_BAD) {
            controlsInputPanel();
            startButton->setEnabled(false);
        } else if (state == UIState::DEBUG_PAUSE || state == UIState::DEBUG_PLAY) {
            /* If entering from a non-debug state, restart the debugger. */
            if (uiState != UIState::DEBUG_PLAY && uiState != UIState::DEBUG_PAUSE) {
                resetDebugger();
            }
            setControls(debugPanel);
        } else {
            error("Unknown UI state.");
        }

        uiState = state;
    }

    void DebugGUI::controlsEmptyPanel() {
        mainPanel->remove(inputPanel);
        mainPanel->addToGrid(emptyPanel, 0, 1, 1, 3);

        emptyPanel->setVisible(true);
        mainPanel->setVisible(true);
        setControls(mainPanel);
    }

    void DebugGUI::controlsInputPanel() {
        mainPanel->remove(emptyPanel);
        mainPanel->addToGrid(inputPanel, 0, 1, 1, 3);

        inputPanel->setVisible(true);
        mainPanel->setVisible(true);
        setControls(mainPanel);
    }

    void DebugGUI::setControls(GContainer* controls){
        /* No need to do anything if we're already using this set of controls. */
        if (currPanel == controls) return;

        if (currPanel) controlPanel->remove(currPanel);

        controlPanel->add(controls);
        currPanel = controls;
    }

    void DebugGUI::loadAutomaton(const string& filename) {
        /* Stash filename for later. */
        currFilename = getTail(filename);
        Core::setLastFilename(filename);

        ifstream input(filename);
        editor = make_shared<Editor::Automaton>(JSON::parse(input));
        editor->setBounds(automatonArea());

        nfa = make_shared<Automata::NFA>(editor->toNFA());

        errors = editor->checkValidity();
        setState(errors.empty()? UIState::STRING_INPUT_GOOD : UIState::BAD_AUTOMATON);

        currAutomaton->setText(getTail(currFilename));
        requestRepaint();
    }

    void DebugGUI::userLoadAutomaton() {
        /* Ask user to pick a file; don't do anything if they don't pick one. */
        string filename = GFileChooser::showOpenDialog(&window(), "Choose Automaton", "res/", "*.automaton");
        if (filename == "") return;

        loadAutomaton(filename);
    }

    void DebugGUI::actionPerformed(GObservable* source) {
        if (uiState == UIState::NO_AUTOMATON) {
            if (source == loadButton) {
                userLoadAutomaton();
            }
        } else if (uiState == UIState::BAD_AUTOMATON) {
            if (source == loadButton) {
                userLoadAutomaton();
            }
        } else if (uiState == UIState::STRING_INPUT_GOOD) {
            if (source == loadButton) {
                userLoadAutomaton();
            } else if (source == startButton) {
                setState(UIState::DEBUG_PAUSE);
            }
        } else if (uiState == UIState::STRING_INPUT_BAD) {
            if (source == loadButton) {
                userLoadAutomaton();
            }
        } else if (uiState == UIState::DEBUG_PAUSE || uiState == UIState::DEBUG_PLAY) {
            if (source == toBeginning) {
                seekDebuggerTo(0);
                pause();
            } else if (source == toEnd) {
                seekDebuggerTo(debugStr.size());
                pause();
            } else if (source == next && index != debugStr.size()) {
                seekDebuggerTo(index + 1);
                pause();
            } else if (source == previous && index != 0) {
                seekDebuggerTo(index - 1);
                pause();
            } else if (source == stop) {
                handleInputBug();
                pause();
                setState(UIState::STRING_INPUT_GOOD);
                requestRepaint();
            } else if (source == playPause) {
                if (uiState == UIState::DEBUG_PLAY) {
                    pause();
                } else {
                    play();
                }
            }
        }
        else {
            error("Unknown state.");
        }
    }

    GRectangle DebugGUI::automatonArea() {
        return { contentBounds.x, contentBounds.y, contentBounds.width, contentBounds.height * kAutomatonHeight };
    }

    void DebugGUI::recomputeBounds() {
        double aspect = window().getCanvasWidth() / window().getCanvasHeight();
        double width, height;
        if (aspect < kAspectRatio) {
            /* Too tall. */
            width = window().getCanvasWidth();
            height = width / kAspectRatio;
        } else {
            height = window().getCanvasHeight();
            width = height * kAspectRatio;
        }

        double baseX = (window().getCanvasWidth()  - width)  / 2.0;
        double baseY = (window().getCanvasHeight() - height) / 2.0;

        contentBounds = { baseX, baseY, width, height };
        if (editor) editor->setBounds(automatonArea());
    }

    void DebugGUI::windowResized() {
        recomputeBounds();
        ProblemHandler::windowResized();
    }

    void DebugGUI::repaint() {
        clearDisplay(window(), kBackgroundColor);

        if (uiState == UIState::NO_AUTOMATON) {
            showWelcomeMessage();
        } else if (uiState == UIState::BAD_AUTOMATON) {
            showErrors();
        } else if (uiState == UIState::STRING_INPUT_GOOD || uiState == UIState::STRING_INPUT_BAD) {
            editor->draw(window().getCanvas(), {}, {});
        } else if (uiState == UIState::DEBUG_PLAY || uiState == UIState::DEBUG_PAUSE) {
            drawAutomatonWithActiveStates();
            drawDebuggedString();
        } else {
            error("Unknown state.");
        }
    }

    void DebugGUI::changeOccurredIn(GObservable* source) {
        if (uiState == UIState::STRING_INPUT_GOOD || uiState == UIState::STRING_INPUT_BAD) {
            if (source == inputField) {
                checkInput();
            }
        }
    }

    void DebugGUI::checkInput() {
        /* TODO: Explicit input of epsilon? */
        for (auto ch: inputString()) {
            if (!editor->alphabet().count(ch)) {
                setState(UIState::STRING_INPUT_BAD);
                return;
            }
        }

        setState(UIState::STRING_INPUT_GOOD);
    }

    vector<char32_t> DebugGUI::inputString() {
        vector<char32_t> result;

        istringstream source(inputField->getText());
        while (source.peek() != EOF) {
            result.push_back(readChar(source));
        }

        return result;
    }

    void DebugGUI::resetDebugger() {
        debugStr = inputString();
        seekDebuggerTo(0);
    }

    /* Runs the debugger on the prefix of the string consisting of the first index
     * characters, storing the index for later.
     *
     * TODO: This system assumes all input characters are single UTF-8 bytes.
     */
    void DebugGUI::seekDebuggerTo(size_t index) {
        /* Form our string to debug. */
        string input;
        for (size_t i = 0; i < index; i++) {
            input += toUTF8(debugStr[i]);
        }

        /* Translate active NFA states into graphics states. */
        active.clear();
        for (auto state: deltaStar(*nfa, input)) {
            active.insert(editor->nodeLabeled(state->name));
        }

        /* Change which buttons are active. */
        toBeginning->setEnabled(index != 0);
        previous->setEnabled(index != 0);
        playPause->setEnabled(index != debugStr.size());
        next->setEnabled(index != debugStr.size());
        toEnd->setEnabled(index != debugStr.size());

        this->index = index;
        requestRepaint();
    }

    void DebugGUI::drawAutomatonWithActiveStates() {
        unordered_map<GraphEditor::Node*, GraphEditor::NodeStyle> styles;

        /* If we're at the end, color-code based on accepting or rejecting. */
        if (index == debugStr.size()) {
            /* Accepting! */
            if (any_of(active.begin(), active.end(), [](Editor::State* state) {
                return state->accepting();
            })) {
                for (auto state: active) {
                    styles[state].fillColor = state->accepting()? kAcceptStateColor : kActiveStateColor;
                }
            } else {
                for (auto state: active) {
                    styles[state].fillColor = kRejectStateColor;
                }
            }
        } else {
            for (auto state: active) {
                styles[state].fillColor = kActiveStateColor;
            }
        }

        editor->draw(window().getCanvas(), styles, {});
    }

    void DebugGUI::drawDebuggedString() {        
        /* Determine the width to use, done in world coordinates. We're chosen so that
         * (1) we don't end up with total width greater than the screen size, and
         * (2) we don't exceed the height of the character area.
         * The calculation in (1) adds one to the total to account for the arrow.
         */
        double width = min(kCharHeight, min(1.0, kDefaultCharSize * (debugStr.size() + 1)) / (debugStr.size() + 1));
        double baseX = (1 - width * debugStr.size()) / 2.0;

        /* Draw each character. */
        for (size_t i = 0; i < debugStr.size(); i++) {
            drawSingleCharacter(debugStr[i], { baseX + width * i, kCharY, width, width });
        }

        /* Draw the arrow at the current position. */
        if (!debugStr.empty()) {
            drawArrow({ baseX - width / 2 + index * width, kCharY + width, width, width });
        }
    }

    GRectangle DebugGUI::worldToGraphics(const GRectangle& in) {
        return { in.x * contentBounds.width + contentBounds.x,
                 in.y * contentBounds.width + contentBounds.y,
                 in.width  * contentBounds.width,
                 in.height * contentBounds.width };
    }

    void DebugGUI::drawSingleCharacter(char32_t ch, const GRectangle& worldBounds) {
        GRectangle bounds = worldToGraphics(worldBounds);

        /* Draw the bounding box. */
        window().setColor(kCharBackgroundColor);
        window().fillRect(bounds);
        window().setColor(kCharBorderColor);
        window().drawRect(bounds);

        /* Draw the text. */
        auto text = TextRender::construct(toUTF8(ch), bounds, kCharFont);
        text->alignCenterVertically();
        text->alignCenterHorizontally();
        text->draw(window());
    }

    void DebugGUI::drawArrow(const GRectangle& graphicsBounds) {
        GPolygon polygon;
        polygon.setFilled(true);
        polygon.setColor(kArrowColor);

        /* Box origin point. */
        GRectangle bounds = worldToGraphics(graphicsBounds);
        GPoint box = { bounds.x, bounds.y };

        /* Box coordinate transform:
         *  | width    0   |
         *  |   0   height |
         */
        GMatrix transform = { bounds.width, 0, 0, bounds.height };
        for (const auto& v: kArrow) {
            polygon.addVertex(box + transform * v);
        }

        window().draw(polygon);
    }

    /* Starts up the animation. */
    void DebugGUI::play() {
        playPause->setText("⏸");
        timer = new GTimer(kAnimationSpeed);
        timer->start();
        setState(UIState::DEBUG_PLAY);
    }

    /* Stops the animation. */
    void DebugGUI::pause() {
        playPause->setText("▶");

        /* Stop the timer, but not if it's already stopped. */
        if (timer) {
            timer->stop();
            timer = nullptr;
        }

        setState(UIState::DEBUG_PAUSE);
    }

    void DebugGUI::timerFired() {
        /* Stray timer fire. */
        if (uiState != UIState::DEBUG_PLAY) return;

        /* Step forward. */
        if (index != debugStr.size()) {
            seekDebuggerTo(index + 1);
        }

        /* Stop if done. */
        if (index == debugStr.size()) {
            pause();
        }
    }

    /* There appears to be an internal bug where removing an input
     * element and adding it triggers a Qt internal warning. Destroy and
     * then restore the input field.
     */
    void DebugGUI::handleInputBug() {
        /* Recycle? */
        string contents;
        if (inputField) {
            contents = inputField->getText();
            inputPanel->clear();
            delete inputField;
        }
        /* Fresh? */
        else {
            startButton = new GButton("Debug");
        }
        inputField = new GTextField(contents);
        inputField->setPlaceholder("ε");

        inputPanel->add(new GLabel("Input: "));
        inputPanel->add(inputField);
        inputPanel->add(startButton);
    }

    void DebugGUI::showWelcomeMessage() {
        auto render = TextRender::construct(kWelcome, contentBounds, kWelcomeFont);
        render->alignCenterVertically();
        render->alignCenterHorizontally();
        render->draw(window());
    }

    string DebugGUI::automatonErrors() {
        string result;
        for (string error: errors) {
            result += format(kSingleErrorMessageTemplate, error);
        }
        return result;
    }

    void DebugGUI::showErrors() {
        /* Upper half is for the automaton; lower half is for the errors. */
        GRectangle automatonBounds = { contentBounds.x, contentBounds.y,
                                       contentBounds.width, contentBounds.height / 2.0 };
        GRectangle errorBounds     = { contentBounds.x, contentBounds.y + contentBounds.height / 2.0,
                                       contentBounds.width, contentBounds.height / 2.0 };

        /* Draw the automaton. */
        auto oldBounds = editor->bounds();
        editor->setBounds(automatonBounds);
        editor->draw(window().getCanvas(), {}, {});
        editor->setBounds(oldBounds);

        /* Draw the error message. */
        string text = format(kErrorMessageTemplate, automatonErrors());
        auto render = TextRender::construct(text, errorBounds, kErrorFont);
        render->alignCenterVertically();
        render->alignCenterHorizontally();
        render->draw(window());
    }
}

GRAPHICS_HANDLER("Automaton Debugger", GWindow& window) {
    return make_shared<DebugGUI>(window);
}

/* Console testing interface. */

namespace {
    vector<string> allAutomataFiles() {
        vector<string> result;
        for (string file: listDirectory("res/")) {
            if (endsWith(file, ".automaton")) {
                result.push_back("res/" + file);
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
        /* Current automaton. */
        shared_ptr<Editor::Automaton> automaton;
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

    string transitionCharToString(char32_t ch) {
        return ch == Automata::EPSILON_TRANSITION? "ε" : toUTF8(ch);
    }

    string prettyTransitions(const set<char32_t>& chars) {
        if (chars.empty()) {
            return "(no characters selected)";
        } else if (chars.size() == 1) {
            return transitionCharToString(*chars.begin());
        } else if (chars.size() == 2) {
            return transitionCharToString(*chars.begin()) + " and " + transitionCharToString(*chars.rbegin());
        } else {
            vector<char32_t> charVec(chars.begin(), chars.end());
            ostringstream builder;

            for (size_t i = 0; i < charVec.size(); i++) {
                builder << transitionCharToString(charVec[i]);
                if (i + 1 != charVec.size()) {
                    builder << ", ";
                }
                if (i + 2 == charVec.size()) {
                    builder << "and ";
                }
            }

            return builder.str();
        }
    }

    void printFN(REPLData& data, const Vector<string>&) {
        cout << "States: " << endl;

        map<string, Editor::State*> states;
        data.automaton->forEachNode([&](Editor::State* state) {
            states[desubscript(state->label())] = state;
        });


        for (const auto& entry: states) {
            cout << entry.first;
            if (entry.second->accepting()) {
                cout << " (accepting state)";
            }
            if (entry.second->start()) {
                cout << " (start state)";
            }
            cout << endl;
        }

        cout << "Transitions: " << endl;
        map<pair<string, string>, Editor::Transition*> transitions;
        data.automaton->forEachEdge([&](Editor::Transition* transition) {
            auto from  = desubscript(transition->from()->label());
            auto to    = desubscript(transition->to()->label());

            transitions[make_pair(from, to)] = transition;
        });

        for (const auto& transition: transitions) {
            cout << "From " << transition.first.first << " to " << transition.first.second << " on " << prettyTransitions(transition.second->chars()) << endl;
        }
    }

    void runFN(REPLData& data, const Vector<string>&) {
        string input = getLine("Enter the input string to the automaton. To enter the empty string, just press ENTER. ");

        /* Validate input. */
        for (char32_t ch: utf8Reader(input)) {
            if (!data.automaton->alphabet().count(ch)) {
                cerr << "Error: Character " << toUTF8(ch) << " is not in the automaton's alphabet." << endl;
                return;
            }
        }

        /* Step through the automaton. We use a silly trick: we can see what's active at each
         * state by running delta* on larger and larger prefixes.
         *
         * TODO: This system assumes all input characters are single UTF-8 bytes.
         */
        for (size_t i = 0; i <= input.size(); i++) {
            if (i == 0) {
                cout << "The automaton, at start-up, ";
            } else {
                cout << "The automaton, after reading character " << input[i - 1] << ", ";
            }

            auto states = Automata::deltaStar(data.automaton->toNFA(), input.substr(0, i));
            cout << "is in " << (states.size() == 1? "this state: " : "these states: ");

            for (auto* state: states) {
                cout << desubscript(state->name) << " ";
            }

            cout << endl;
        }

        /* See if we're accepting. */
        cout << "Overall, the automaton " << (Automata::accepts(data.automaton->toNFA(), input)? "accepts" : "rejects") << " the input." << endl;
    }

    const vector<Command> kCommands = {
        { "help",          "help: Displays the help menu.",  0, helpFN },
        { "quit",          "quit: Exits the tester.",         0, quitFN },
        { "print",         "print: Display the automaton", 0, printFN },
        { "run",           "run: Prompts for an input and runs the automaton on that input.",  0, runFN  },
    };

    void helpFN(REPLData&, const Vector<string>&){
        for (const auto& command: kCommands) {
            cout << command.desc << endl;
        }
    }

    void automatonREPL(shared_ptr<Editor::Automaton> automaton) {
        REPLData data;
        data.automaton = automaton;

        try {
            cout << "Type 'help' for a list of commands." << endl;
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
    }

    void textTestAutomaton(const string& filename) {
        auto automaton = loadAutomaton(filename);

        /* Can't bulk-test an automaton with errors. */
        auto errors = automaton->checkValidity();
        if (!errors.empty()) {
            cout << "This automaton is invalid. Please correct these errors in the editor:" << endl;
            for (string error: errors) {
                cout << desubscript(error) << endl;
            }
            return;
        }

        automatonREPL(automaton);
    }
}

CONSOLE_HANDLER("Automaton Debugger") {
    do {
        auto automata = allAutomataFiles();
        textTestAutomaton(automata[makeSelectionFrom("Choose an automaton: ", automata)]);
    } while (getYesOrNo("Debug another automaton? "));
}
