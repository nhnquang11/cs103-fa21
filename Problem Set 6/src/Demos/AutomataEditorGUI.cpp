/* TODO:
 *
 * Undo / redo
 * Interactive testing
 * Styling on GOptionPane popups
 * Don't allow states to drag on top of other states
 */
#include "AutomataEditor.h"
#include "../GraphEditor/GVector.h"
#include "../GraphEditor/GraphEditor.h"
#include "AutomataEditorCore.h"
#include "Utilities/JSON.h"
#include "Utilities/Unicode.h"
#include "../FormalLanguages/Automaton.h"
#include "../GUI/MiniGUI.h"
#include "gthread.h"
#include "filelib.h"
#include "goptionpane.h"
#include <fstream>
using namespace std;
using namespace MiniGUI;

namespace {
    /* Chrome parameters. */
    const double kControlWidth = 0.75;
    const double kControlHeight = 0.1;

    /* Various messages. */
    const string kAutomatonHasErrors = "Your %s contains some structural errors:\n\n%s\nDo you want to save anyway?";
    const string kAutomatonHasErrorsTitle = "Automaton Errors";
    const string kAutomatonErrorString = "• %s.\n";

    const string kUnsavedChanges = "You have unsaved changes.\n\nDo you want to save?";
    const string kUnsavedChangesTitle = "Unsaved Changes";

    const string kWelcome = R"(Welcome to the Automaton Editor!

    Click "Load Automaton" to choose an automaton.)";

    const Font kWelcomeFont(FontFamily::SERIF, FontStyle::BOLD_ITALIC, 24, "#4C5866"); // Marengo

    const string kInstructions = R"(Double-click to create a state.)";
    const string kBackgroundColor = "white";

    class EditGUI: public ProblemHandler {
    public:
        EditGUI(GWindow& window);

        void windowResized() override;

        void mouseDoubleClicked(double x, double y) override;
        void mouseMoved(double x, double y) override;
        void mousePressed(double x, double y) override;
        void mouseDragged(double x, double y) override;
        void mouseReleased(double x, double y) override;

        void actionPerformed(GObservable* source) override;
        void changeOccurredIn(GObservable* source) override;

        bool shuttingDown() override;

    protected:
        void repaint() override;

    private:
        /* The actual editor! This can be nullptr if we aren't enabled. */
        shared_ptr<GraphEditor::Editor<Editor::Automaton>> editor;

        /* Active items, stored here for the control panel. */
        Editor::State* activeState = nullptr;
        Editor::Transition* activeTransition = nullptr;

        /* Window chrome. */
        GContainer* emptyControl;

        /********************************************
         *         [state name] Properties          *
         * isState | isAccepting | [ Delete State ] *
         ********************************************/
        GContainer* stateControl;
        GLabel*     stateDisplay;
        GCheckBox*  isStart;
        GCheckBox*  isAccepting;
        GButton*    deleteState;

        /************************************************************
         * [ flow layout transition options] | [ delete transition] *
         ************************************************************/
        GContainer* transitionControl;
        GContainer* transitionBox;
        GButton* deleteTransition;
        unordered_set<GCheckBox*> transitionOptions;

        Temporary<GContainer> controlPanel;
        GInteractor* currPanel = nullptr;

        GLabel*  currAutomatonLabel;
        GButton* saveButton;
        GButton* loadButton;
        string currFilename = "res/IfYouSeeThisFileContactKeith";
        bool isDFA = true;

        /* Dirty bit. Don't set this directly; use dirty() instead. */
        bool isDirty = false;

        /* Is the editor enabled? Don't set this directly; use enable() instead. */
        bool isEnabled = false;

        void drawWelcomeMessage();
        void drawInstructions();

        /* Changes the active/hover state/transition. */
        void setActive(GraphEditor::Entity* active);

        void setActiveNode(Editor::State* state);
        void setActiveEdge(Editor::Transition* transition);

        /* Chrome management. */
        void setControls(GInteractor* controls);

        /* Automaton controls. */
        void setAlphabet(const Languages::Alphabet& alphabet, bool isDFA);

        /* Deleters. */
        void deleteActiveState();
        void deleteActiveTransition();

        JSON graphicsToJSON();
        void saveAutomaton();

        /* True if handled, false otherwise. */
        bool handleUnsavedChanges();

        /* True if saved, false otherwise. */
        bool userSaveAutomaton();

        void userLoadAutomaton();
        void loadAutomaton(const string& filename);

        GRectangle contentArea();

        /* Checks if an automaton is valid. If not, fills in the output string
         * with a list of human-readable error messages.
         */
        bool isAutomatonValid(string& message);

        void dirty(bool dirtyBit = true);
        void enable(bool enabled);

        /* Listener to forward things. */
        class Listener;
    };

    class EditGUI::Listener: public GraphEditor::Listener {
    public:
        Listener(EditGUI* owner);

        void isDirty() override;
        void needsRepaint() override;
        void entitySelected(GraphEditor::Entity* entity) override;

    private:
        EditGUI* mOwner;
    };

    EditGUI::EditGUI(GWindow& window) : ProblemHandler(window) {
        /* Set up chrome. */
        GThread::runOnQtGuiThread([&] {
            GContainer* controls = new GContainer(GContainer::LAYOUT_GRID);
            currAutomatonLabel = new GLabel(currFilename);
            saveButton = new GButton("Save Automaton");
            loadButton = new GButton("Load Automaton");
            controls->addToGrid(currAutomatonLabel, 0, 0);
            controls->addToGrid(saveButton, 1, 0);
            controls->addToGrid(loadButton, 2, 0);

            emptyControl = new GContainer(GContainer::LAYOUT_GRID);

            stateControl   = new GContainer(GContainer::LAYOUT_GRID);
            isStart        = new GCheckBox("Start State");
            isAccepting    = new GCheckBox("Accepting");
            deleteState    = new GButton("Delete State");

            GContainer* stateButtonBox = new GContainer();
            GContainer* stateLabelBox  = new GContainer();


            stateDisplay = new GLabel("");
            stateLabelBox->add(stateDisplay);
            stateButtonBox->add(isStart);
            stateButtonBox->add(isAccepting);
            stateControl->addToGrid(stateLabelBox, 0, 0);
            stateControl->addToGrid(stateButtonBox, 1, 0);
            stateControl->addToGrid(deleteState, 0, 1, 2);

            transitionControl = new GContainer(GContainer::LAYOUT_GRID);
            deleteTransition = new GButton("Delete Transition");
            transitionBox = new GContainer(GContainer::LAYOUT_FLOW_HORIZONTAL);
            transitionControl->addToGrid(transitionBox, 0, 0);
            transitionControl->addToGrid(deleteTransition, 0, 1);

            controls->addToGrid(emptyControl, 0, 1, 3);
            controlPanel = Temporary<GContainer>(controls, window, "SOUTH");
            currPanel = emptyControl;

            /* All controls need to have the same dimensions to prevent flickering/
             * shifting of other controls when toggling which panel is active.
             *
             * We ues the height of the main control box as the height, as this is
             * unlikely to clip anything.
             */
            double width  = window.getWidth() * kControlWidth;
            double height = controls->getHeight();
            emptyControl->setSize(width,      height);
            stateControl->setSize(width,      height);
            transitionControl->setSize(width, height);

            /* Make the control panels invisible. There seems to be a bug in the graphics
             * system where controls with no parent get displayed as though they were on
             * the main window?
             */
            stateControl->setVisible(false);
            transitionControl->setVisible(false);

            enable(false);
        });

        /* If there was a last automaton edited, let's edit that one. */
        if (Core::lastFilename() != "") {
            loadAutomaton(Core::lastFilename());
        }
    }

    void EditGUI::windowResized() {
        // TODO: Ask the editor, not the viewer, to do this?
        if (editor) editor->viewer()->setBounds(contentArea());
        requestRepaint();
    }

    void EditGUI::enable(bool enabled) {
        if (enabled) {
            saveButton->setEnabled(true);
        } else {
            saveButton->setEnabled(false);
            currAutomatonLabel->setText("Choose an Automaton");
        }
        isEnabled = enabled;
    }

    void EditGUI::setActive(GraphEditor::Entity* active) {
        if (auto* node = dynamic_cast<Editor::State*>(active)) {
            setActiveNode(node);
        } else if (auto* edge = dynamic_cast<Editor::Transition*>(active)) {
            setActiveEdge(edge);
        } else {
            setActiveNode(nullptr);
            setActiveEdge(nullptr);
        }
    }

    void EditGUI::setActiveNode(Editor::State* state) {
        activeState = state;
        if (activeState) {
            activeTransition = nullptr;
        }

        if (activeState) {
            stateDisplay->setText("State " + activeState->label());
            isStart->setChecked(activeState->start());
            isAccepting->setChecked(activeState->accepting());
            setControls(stateControl);
        } else {
            setControls(emptyControl);
        }
    }

    char32_t checkboxToChar(GCheckBox* checkbox) {
        return checkbox->getText() == "ε"? Automata::EPSILON_TRANSITION : fromUTF8(checkbox->getText());
    }

    void EditGUI::setActiveEdge(Editor::Transition* transition) {
        activeTransition = transition;
        if (activeTransition != nullptr) {
            activeState = nullptr;
        }

        if (activeTransition) {
            GThread::runOnQtGuiThread([&] {
                /* Toggle checkbox states. */
                for (auto* checkbox: transitionOptions) {
                    char32_t ch = checkboxToChar(checkbox);
                    checkbox->setChecked(transition->chars().count(ch));
                }
            });

            setControls(transitionControl);
        } else {
            setControls(emptyControl);
        }
    }

    void EditGUI::setControls(GInteractor* controls) {
        if (currPanel != controls) {
            GThread::runOnQtGuiThread([&] {
                currPanel->setVisible(false);
                controlPanel->remove(currPanel);
                controlPanel->addToGrid(controls, 0, 1, 3);
                controls->setVisible(true);
                currPanel = controls;
            });
        }
    }

    void EditGUI::mouseDoubleClicked(double x, double y) {
        if (!isEnabled) return;

        editor->mouseDoubleClicked(x, y);
    }

    void EditGUI::drawWelcomeMessage() {
        auto render = TextRender::construct(kWelcome, contentArea(), kWelcomeFont);
        render->alignCenterVertically();
        render->alignCenterHorizontally();
        render->draw(window());
    }

    void EditGUI::drawInstructions() {
        auto render = TextRender::construct(kInstructions, contentArea(), kWelcomeFont);
        render->alignCenterVertically();
        render->alignCenterHorizontally();
        render->draw(window());
    }

    void EditGUI::mouseMoved(double x, double y) {
        if (!isEnabled) return;

        editor->mouseMoved(x, y);
    }

    void EditGUI::mousePressed(double x, double y) {
        if (!isEnabled) return;

        editor->mousePressed(x, y);
    }

    void EditGUI::mouseDragged(double x, double y) {
        if (!isEnabled) return;

        editor->mouseDragged(x, y);
    }

    void EditGUI::mouseReleased(double x, double y) {
        if (!isEnabled) return;

        editor->mouseReleased(x, y);
    }

    void EditGUI::setAlphabet(const Languages::Alphabet& alphabet, bool isDFA) {
        this->isDFA = isDFA;
        GThread::runOnQtGuiThread([&] {
            /* Remove all existing transitions. */
            for (auto entry: transitionOptions) {
                transitionBox->remove(entry);
                delete entry;
            }

            transitionOptions.clear();

            /* Make new transitions. */
            for (char32_t ch: alphabet) {
                GCheckBox* option = new GCheckBox(toUTF8(ch));
                transitionBox->add(option);
                transitionOptions.insert(option);
            }

            /* Epsilon transition, if appropriate. */
            if (!isDFA) {
                GCheckBox* epsilon = new GCheckBox("ε");
                transitionBox->add(epsilon);
                transitionOptions.insert(epsilon);
            }

            /* TODO: This is to work around a bug where checkboxes float in ghostly
             * places if they're added to a component that isn't then added into
             * the window.
             */
            transitionControl->setVisible(false);
        });
    }

    void EditGUI::changeOccurredIn(GObservable* source) {
        if (!isEnabled) return;

        if (currPanel == stateControl) {
            if (source == isAccepting) {
                activeState->accepting(isAccepting->isChecked());
                requestRepaint();
                dirty();
            } else if (source == isStart) {
                activeState->start(isStart->isChecked());

                /* There can only be one start state. */
                if (activeState->start()) {
                    editor->viewer()->forEachNode([&](Editor::State* state) {
                        if (state != activeState) state->start(false);
                    });
                }

                requestRepaint();
                dirty();
            }
        } else if (currPanel == transitionControl) {
            /* Is this a letter checkbox? */
            if (auto* checkbox = dynamic_cast<GCheckBox*>(source)) {
                if (transitionOptions.count(checkbox)) {
                    /* Convert the checkbox to a char32_t. */
                    char32_t ch = checkboxToChar(checkbox);

                    /* Set / remove it. */
                    if (checkbox->isChecked()) {
                        activeTransition->add(ch);
                    } else {
                        activeTransition->remove(ch);
                    }

                    dirty();
                    requestRepaint();
                }
            }
        }
    }

    void EditGUI::deleteActiveState() {
        /* Remove the state. */
        editor->deleteNode(activeState);
        setActive(nullptr);
        dirty();
    }

    void EditGUI::deleteActiveTransition() {
        /* Remove from the list of transitions. */
        editor->deleteEdge(activeTransition);
        setActive(nullptr);
        dirty();
    }

    void EditGUI::actionPerformed(GObservable* source) {
        if (source == deleteState && activeState) {
            deleteActiveState();
        } else if (source == deleteTransition && activeTransition) {
            deleteActiveTransition();
        } else if (source == saveButton) {
            userSaveAutomaton();
        } else if (source == loadButton) {
            userLoadAutomaton();
        }
    }

    /* Serializes the current state of things. */
    void EditGUI::saveAutomaton() {
        /* TODO: Don't overwrite the source until the write has finished.
         * Use mkstemp, write there, and then do a move when done.
         */
        ofstream output(currFilename);
        if (!output) error("Cannot open " + currFilename + " for writing.");

        output << editor->viewer()->toJSON();
    }

    bool EditGUI::isAutomatonValid(string& message) {
        auto errors = editor->viewer()->checkValidity();
        if (errors.empty()) return true;

        message = "";
        for (string error: errors) {
            message += format(kAutomatonErrorString, error);
        }
        return false;
    }

    bool EditGUI::userSaveAutomaton() {
        /* Give the user a warning if the automaton is invalid, but let them save things
         * anyway.
         */
        string errors;
        if (!isAutomatonValid(errors) &&
            GOptionPane::showConfirmDialog(&window(), format(kAutomatonHasErrors, isDFA? "DFA" : "NFA", errors), kAutomatonHasErrorsTitle) != GOptionPane::CONFIRM_YES) {
            return false;
        }

        saveAutomaton();

        dirty(false);
        GOptionPane::showMessageDialog(&window(), "Automaton " + currFilename + " was saved!");
        return true;
    }

    bool EditGUI::handleUnsavedChanges() {
        if (!isDirty) return true;

        auto result = GOptionPane::showConfirmDialog(&window(), kUnsavedChanges, kUnsavedChangesTitle, GOptionPane::CONFIRM_YES_NO_CANCEL);

        /* A firm "no" means "okay, I want to discard things. */
        if (result == GOptionPane::CONFIRM_NO) {
            return true;
        }

        /* "Cancel" means "wait, hold on, I didn't mean to do that. */
        if (result == GOptionPane::CONFIRM_CANCEL) {
            return false;
        }

        /* Otherwise, they intend to save. See whether they do. */
        return userSaveAutomaton();
    }

    bool EditGUI::shuttingDown() {
        return handleUnsavedChanges();
    }

    void EditGUI::loadAutomaton(const string& filename) {
        currFilename = filename;
        Core::setLastFilename(filename);
        currAutomatonLabel->setLabel(getTail(currFilename));

        /* Load the data! */
        ifstream input(filename);
        editor = make_shared<GraphEditor::Editor<Editor::Automaton>>(make_shared<Editor::Automaton>(JSON::parse(input)));
        setAlphabet(editor->viewer()->alphabet(), editor->viewer()->isDFA());

        editor->viewer()->setBounds(contentArea());
        editor->addListener(make_shared<Listener>(this));

        setActive(nullptr);
        enable(true);
        dirty(false);
        requestRepaint();
    }

    void EditGUI::userLoadAutomaton() {
        /* Warn user about unsaved changes. */
        if (!handleUnsavedChanges()) {
            return;
        }

        /* Ask user to pick a file; don't do anything if they don't pick one. */
        string filename = GFileChooser::showOpenDialog(&window(), "Choose Automaton", "res/", "*.automaton");
        if (filename == "") return;

        loadAutomaton(filename);
    }

    void EditGUI::dirty(bool dirtyBit) {
        if (dirtyBit) {
            if (!isDirty) {
                isDirty = true;
                currAutomatonLabel->setText(getTail(currFilename) + "*");
            }
        } else {
            if (isDirty) {
                isDirty = false;
                currAutomatonLabel->setText(getTail(currFilename));
            }
        }
    }

    void EditGUI::repaint() {
        clearDisplay(window(), kBackgroundColor);
        if (!isEnabled) {
            drawWelcomeMessage();
        } else if (editor->viewer()->numNodes() == 0) {
            drawInstructions();
        } else {
            editor->draw(window().getCanvas());
        }
    }

    GRectangle EditGUI::contentArea() {
        return { 0, 0, window().getCanvasWidth(), window().getCanvasHeight() };
    }

    EditGUI::Listener::Listener(EditGUI* owner): mOwner(owner) {
        // Handled above
    }

    void EditGUI::Listener::isDirty() {
        mOwner->dirty();
    }

    void EditGUI::Listener::needsRepaint() {
        mOwner->requestRepaint();
    }

    void EditGUI::Listener::entitySelected(GraphEditor::Entity* entity) {
        mOwner->setActive(entity);
    }
}

GRAPHICS_HANDLER("Automaton Editor", GWindow& window) {
    return make_shared<EditGUI>(window);
}


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

    void saveAutomaton(const string& filename, shared_ptr<Editor::Automaton> automaton) {
        ofstream output(filename);
        output << automaton->toJSON();

        if (!output) error("Error saving your answers; contact the course staff.");
    }

    struct AllDoneNow{};

    /* Arguments to a REPL command. */
    struct REPLData {
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

    /* Converts non-subscripted text to subscripts; used for input. */
    string subscript(const string& str) {
        static const unordered_map<char32_t, char32_t> kTable = {
            { '0', fromUTF8("₀") },
            { '1', fromUTF8("₁") },
            { '2', fromUTF8("₂") },
            { '3', fromUTF8("₃") },
            { '4', fromUTF8("₄") },
            { '5', fromUTF8("₅") },
            { '6', fromUTF8("₆") },
            { '7', fromUTF8("₇") },
            { '8', fromUTF8("₈") },
            { '9', fromUTF8("₉") },
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

    void newStateFN(REPLData& data, const Vector<string>&) {
        auto* state = data.automaton->newNode({});
        cout << "Created state " << desubscript(state->label()) << "." << endl;
    }

    /* Decodes the input to a character. Throws an exception if the input can't be
     * interpreted as a transition here.
     */
    char32_t decodeTransition(const REPLData& data, const string& input) {
        /* Handle epsilon transitions separately. */
        if (input == "epsilon") {
            if (data.automaton->isDFA()) throw invalid_argument("Epsilon transitions are not permitted in a DFA.");
            return Automata::EPSILON_TRANSITION;
        }

        try {
            char32_t result = fromUTF8(input);

            if (!data.automaton->alphabet().count(result)) {
                throw invalid_argument("Character " + input + " is not allowed on transitions in this automaton.");
            }

            return result;
        } catch (const UTFException& e) {
            throw invalid_argument("Invalid character.");
        }
    }

    void newTransitionFn(REPLData& data, const Vector<string>& args) {
        /* Check which character this transition is on. */
        char32_t transitionCh;
        try {
            transitionCh = decodeTransition(data, args[2]);
        } catch (const exception& e) {
            cerr << e.what() << endl;
            return;
        }

        auto src = data.automaton->nodeLabeled(subscript(args[0]));
        if (!src) {
            cout << "There is no state named " << args[0] << " in this automaton." << endl;
            return;
        }

        auto dst = data.automaton->nodeLabeled(subscript(args[1]));
        if (!dst) {
            cout << "There is no state named " << args[1] << " in this automaton." << endl;
            return;
        }

        auto* edge = data.automaton->edgeBetween(src, dst);
        if (!edge) {
            edge = data.automaton->newEdge(src, dst);
        }

        if (edge->chars().count(transitionCh)) {
            cout << "There already is a transition from " << args[0] << " to " << args[1] << " on character " << transitionCharToString(transitionCh) << endl;
            return;
        }

        edge->add(transitionCh);

        cout << "Added an transition from " << args[0] << " to " << args[1] << " on character " << transitionCharToString(transitionCh) << endl;
    }

    void delStateFN(REPLData& data, const Vector<string>& arg) {
        auto state = data.automaton->nodeLabeled(subscript(arg[0]));
        if (state == nullptr) {
            cout << "No state with that name exists in the automaton." << endl;
            return;
        }

        data.automaton->removeNode(state);
        cout << "Removed state " << arg[0] << endl;
    }

    void delTransitionFN(REPLData& data, const Vector<string>& args) {
        /* Check which character this transition is on. */
        char32_t transitionCh;
        try {
            transitionCh = decodeTransition(data, args[2]);
        } catch (const exception& e) {
            cerr << e.what() << endl;
            return;
        }

        auto src = data.automaton->nodeLabeled(subscript(args[0]));
        if (!src) {
            cout << "There is no state named " << args[0] << " in this automaton." << endl;
            return;
        }

        auto dst = data.automaton->nodeLabeled(subscript(args[1]));
        if (!dst) {
            cout << "There is no state named " << args[1] << " in this automaton." << endl;
            return;
        }

        auto transition = data.automaton->edgeBetween(src, dst);
        if (transition == nullptr || !transition->chars().count(transitionCh)) {
            cout << "There is no transition from " << args[0] << " to " << args[1] << " on character " << args[2] << endl;
            return;
        }

        transition->remove(transitionCh);
        cout << "Removed the transition from " << args[0] << " to " << args[1] << " on character " << args[2] << endl;

        /* Console mode doesn't allow for empty transitions. */
        if (transition->chars().empty()) {
            data.automaton->removeEdge(transition);
        }
    }

    void checkFN(REPLData& data, const Vector<string>&) {
        auto errors = data.automaton->checkValidity();
        if (errors.empty()) {
            cout << "Automaton is valid!" << endl;
        } else {
            cout << "This automaton is invalid. Please correct the following errors:" << endl;
            for (string error: errors) {
                cout << desubscript(error) << endl;
            }
        }
    }

    void startStateFN(REPLData& data, const Vector<string>& arg) {
        auto* state = data.automaton->nodeLabeled(subscript(arg[0]));
        if (state == nullptr) {
            cerr << "No state named " << arg[0] << " exists in the automaton." << endl;
            return;
        }

        if (state->start()) {
            cerr << "State " << arg[0] << " is already the start state." << endl;
            return;
        }

        state->start(true);
        data.automaton->forEachNode([&](Editor::State* s) {
            if (s != state) {
                s->start(false);
            }
        });

        cout << "State " << arg[0] << " is now the start state." << endl;
    }

    void markAccepting(REPLData& data, const string& stateName, bool isAccepting) {
        auto* state = data.automaton->nodeLabeled(subscript(stateName));
        if (state == nullptr) {
            cerr << "No state named " << stateName << " exists in this automaton." << endl;
            return;
        }

        if (state->accepting() == isAccepting) {
            cerr << "State " << stateName << " is already " << (isAccepting? "an accepting" : "a rejecting") << " state." << endl;
            return;
        }

        state->accepting(isAccepting);
        cout << "State " << stateName << " is now " << (isAccepting? "an accepting" : "a rejecting") << " state." << endl;
    }

    void acceptStateFN(REPLData& data, const Vector<string>& arg) {
        markAccepting(data, arg[0], true);
    }

    void rejectStateFN(REPLData& data, const Vector<string>& arg) {
        markAccepting(data, arg[0], false);
    }

    const vector<Command> kCommands = {
        { "help",          "help: Displays the help menu.",  0, helpFN },
        { "quit",          "quit: Saves and exits.",  0, quitFN },
        { "print",         "print: Prints the graph.", 0, printFN },
        { "newstate",      "newstate: Creates a new state.", 0, newStateFN },
        { "newtransition", "newtransition from to char: Creates a new transition between states 'from' and 'to' on character 'ch'.",  3, newTransitionFn },
        { "delstate",      "delstate state: Deletes the state 'state' and all transitions into and out of it.", 1, delStateFN },
        { "deltransition", "deltransition from to ch: Deletes the transition labeled 'ch' between nodes 'from' and 'to'. ", 3, delTransitionFN },
        { "startstate",    "startstate state: Makes the state 'state' the start state.", 1, startStateFN },
        { "accepting",     "accepting state: Makes the state 'state' an accepting state.", 1, acceptStateFN },
        { "rejecting",     "rejecting state: Makes the state 'state' a rejecting state.", 1, rejectStateFN },
        { "check",         "check: Checks whether the automaton is valid.", 0, checkFN }
    };

    void helpFN(REPLData& data, const Vector<string>&){
        for (const auto& command: kCommands) {
            cout << command.desc << endl;
        }

        if (!data.automaton->isDFA()) {
            cout << "Any command that requires a character can also accept epsilon as an input. To do so, use the word 'epsilon,' without quotes, as the argument." << endl;
        }
    }

    void automatonREPL(shared_ptr<Editor::Automaton> automaton) {
        REPLData data;
        data.automaton = automaton;
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
    }

    void textEditAutomaton(const string& filename) {
        auto automaton = loadAutomaton(filename);
        automatonREPL(automaton);
        saveAutomaton(filename, automaton);
    }
}

CONSOLE_HANDLER("Automaton Editor") {
    do {
        auto automata = allAutomataFiles();
        textEditAutomaton(automata[makeSelectionFrom("Choose an automaton: ", automata)]);
    } while (getYesOrNo("Edit another automaton? "));
}
