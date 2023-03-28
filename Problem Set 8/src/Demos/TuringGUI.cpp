#include "../GUI/MiniGUI.h"
#include "../Turing/Turing.h"
#include "../GVector.h"
#include "ProgramCore.h"
#include "goptionpane.h"
#include "gtimer.h"
#include "filelib.h"
#include "Utilities/Unicode.h"
#include "gthread.h"
#include <fstream>
#include <memory>
#include <unordered_map>
#include <iostream>
#include <fstream>
using namespace std;
using namespace MiniGUI;

namespace {
    const string kBackgroundColor = "white";
    const string kCodeBackgroundColor = "white";
    const string kActiveLineColor = "#ffd320";

    /* Default size for one character, as a fraction of the width of the screen. */
    const double kDefaultCharSize = 48.0 / 1000;

    const Font kCharFont   (FontFamily::UNICODE_MONOSPACE, FontStyle::BOLD, 24, "black");
    const Font kCodeFont   (FontFamily::UNICODE_MONOSPACE, FontStyle::NORMAL, 24, "black");
    const Font kBadCodeFont(FontFamily::UNICODE_MONOSPACE, FontStyle::BOLD_ITALIC, 24,  "#960018"); // Carmine
    const Font kErrorFont  (FontFamily::SANS_SERIF, FontStyle::ITALIC, 12,  "#960018"); // Carmine

    const double kCodeLineHeight = 32;

    const string kCharBackgroundColor = "#ffffa6"; // Slide's color
    const string kCharBorderColor = "black";

    /* Number of characters to display at any one time, including the ellipses. */
    const int64_t kNumChars = 21;

    /* What happens on a seek to the end. */
    const size_t kStepsOnEndSeek  = 5000000;
    const string kContinueMessage = "We've run your program for a while and it's not done yet. Keep running it?";
    const string kContinueTitle   = "Program Still Running";

    /* Margin on each side of the tape that the tape head can't leave. If the tape
     * head's displayed position were to move outside of this range, instead we scroll
     * the tape over.
     */
    const int64_t kTapeHeadMargin = 3;

    /* Number of lines to keep below us when scrolling the current line number. */
    const size_t kLineMargin = 6;

    /* Scroll margin, when needed. */
    const int kScrollMargin = kCodeLineHeight * 3;

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

    /* Map from slider settings to speeds, expressed as a pair of time delays and frame skips. */
    const vector<pair<double, int>> kAnimationSpeeds = {
        { 750, 1     },
        { 500, 1     },
        { 250, 1     },
        { 125, 1     },
        {  50, 1     },
        {   5, 1     },
        {   5, 4     },
        {   5, 16    },
        {   5, 64    },
        {   5, 256   },
        {   5, 1024  },
        {   5, 4096  },
        {   5, 16384 },
    };

    /* Initial animation speed. */
    const int kInitialSpeedIndex = 0;

    /* Number of steps to take per tick when in fast mode. */
    const size_t kNumFastSteps = 10;

    /* Aspect ratio. */
    const double kAspectRatio = 5.0 / 3.0;

    /* Fraction of the window height occupied by the program. */
    const double kProgramHeight = 0.85;

    /* Where characters go. All sizes are relative to the tape box. */
    const double kCharY           = 0.05 / kAspectRatio;
    const double kCharHeight      = 0.5 / kAspectRatio;

    /* Animation controls. */
    const double kAnimationSpeed = 10; //ms

    const string kWelcome = R"(Welcome to the TM Debugger!

    This tool lets you single-step through a Turing program to better understand how it works.

    Click "Load Program" to choose an automaton.)";
    const Font kWelcomeFont(FontFamily::SERIF, FontStyle::BOLD_ITALIC, 24, "#4C5866"); // Marengo

    /* Ellipsis character for the ends of the tape. */
    const char32_t ELLIPSIS = fromUTF8("⋯");

    class DebugGUI: public ProblemHandler {
    public:
        DebugGUI(GWindow& window);
        ~DebugGUI();

        void actionPerformed(GObservable* source) override;
        void changeOccurredIn(GObservable* source) override;
        void timerFired() override;
        void windowResized() override;

    protected:
        void repaint() override;

    private /* state */:
        shared_ptr<Turing::Program> program_ = make_shared<Turing::Program>(ifstream("res/Simple.tm", ios::binary));
        shared_ptr<Turing::Interpreter> interpreter_;

        /* Minimum character position to display. */
        int64_t lowIndex_ = -kTapeHeadMargin;

        /* Last input entered; used for the "rewind" feature. */
        string lastInput_;

        /* Number of steps executed. */
        uint64_t numSteps_ = 0;

        /* Line we want to be centered on. */
        size_t lastLine_ = 0;

        /* Time delay and frame skip. */
        double timeDelay_ = kAnimationSpeeds.at(kInitialSpeedIndex).first;
        size_t frameSkip_ = kAnimationSpeeds.at(kInitialSpeedIndex).second;

        /* Text renders to display. */
        vector<shared_ptr<TextRender>> guiProgramLines_;

        /* Used when seeking to the end. */
        bool fastMode_ = false;

        enum class UIState {
            INITIALIZING,
            CHOOSE_PROGRAM,
            BAD_PROGRAM,
            CHOOSE_INPUT,
            RUNNING_PAUSE,
            RUNNING_PLAY
        };
        UIState state_ = UIState::INITIALIZING;

        /* Central display. */
        Temporary<GContainer> centerDisplay_;
          GScrollPane*  consoleScroller_;
            QScrollArea*  qScroller;
            GCanvas*  console_;
          GCanvas*  tapeArea_;

        /* Currently-selected panel. */
        GContainer* currentPanel_ = nullptr;

        Temporary<GContainer> mainPanel_;

        GContainer* emptyPanel_;

        GContainer* badProgramPanel_;

        GContainer* loadPanel_;
          GButton* loadButton_;
          GLabel*  currProgramLabel_;

        GContainer* debugPanel_;
          GButton* toBeginning_;
          GButton* toEnd_;
          GButton* step_;
          GButton* stop_;
          GButton* playPause_;
          GSlider* speedControl_;
          GLabel*  statusLine_;

        GContainer* inputPanel_;

        GTimer* timer_ = new GTimer(timeDelay_); // 07/08/21 Have to leak this due to bugs in StanfordCPPLib

        GContainer* activePanel_ = nullptr;
          GTextField* inputField_ = nullptr;
          GButton*    startButton_;

    private /* helpers */:
        GRectangle programArea() const;
        GRectangle tapeArea() const;

        void drawProgramArea();
        void drawTapeArea();

        GRectangle worldToGraphics(const GRectangle& in) const;
        void drawSingleCharacter(char32_t ch, const GRectangle& worldBounds);
        void drawArrow(const GRectangle& graphicsBounds);

        void setState(UIState state);
        void setPanel(GContainer* panel);

        void initSimulation(const string& input);
        void step();
        void seekToEnd();

        void resetInputPanel();

        void updateStatusLine();

        void userLoadProgram();
        void loadProgram(const string& filename);
        void buildProgramLines();
    };

    DebugGUI::DebugGUI(GWindow& window) : ProblemHandler(window) {
        mainPanel_ = make_temporary<GContainer>(window, "SOUTH", GContainer::LAYOUT_GRID);
          loadPanel_ = new GContainer(GContainer::LAYOUT_FLOW_VERTICAL);
            currProgramLabel_ = new GLabel("Choose a Program");
            loadButton_ = new GButton("Load Program");
            loadPanel_->add(currProgramLabel_);
            loadPanel_->add(loadButton_);
          mainPanel_->addToGrid(loadPanel_, 0, 0);

          currentPanel_ = new GContainer();
          mainPanel_->addToGrid(currentPanel_, 0, 1, 1, 3);

        emptyPanel_ = new GContainer();

        debugPanel_ = new GContainer(GContainer::LAYOUT_FLOW_VERTICAL);
          GContainer* buttons = new GContainer();
            toBeginning_ = new GButton("⏮");
            playPause_ = new GButton("▶");
            step_ = new GButton("⏩");
            toEnd_ = new GButton("⏭️");
            stop_ = new GButton("⏹");
            buttons->add(toBeginning_);
            buttons->add(playPause_);
            buttons->add(step_);
            buttons->add(toEnd_);
            buttons->add(stop_);
          debugPanel_->add(buttons);
          GContainer* speed = new GContainer();
            speed->add(new GLabel("Speed: "));
            speedControl_ = new GSlider(0, kAnimationSpeeds.size() - 1, kInitialSpeedIndex);
            speed->add(speedControl_);
          debugPanel_->add(speed);
          statusLine_ = new GLabel(" ");
          debugPanel_->add(statusLine_);

        inputPanel_ = new GContainer();
        resetInputPanel();

        badProgramPanel_ = new GContainer();
          badProgramPanel_->add(new GLabel("This program contains a syntax error and cannot be run."));

        centerDisplay_ = make_temporary<GContainer>(window, "CENTER", GContainer::LAYOUT_FLOW_VERTICAL);
          console_  = new GCanvas();
          tapeArea_ = new GCanvas();
          tapeArea_->setAutoRepaint(false);

          consoleScroller_ = new GScrollPane(console_);
          consoleScroller_->setHorizontalScrollBarPolicy(GScrollPane::SCROLLBAR_NEVER);
          qScroller = dynamic_cast<QScrollArea *>(consoleScroller_->getWidget());
          if (!qScroller) error("Error getting scroll area.");

          centerDisplay_->add(consoleScroller_);
          centerDisplay_->add(tapeArea_);

          console_->setAutoRepaint(false);
          tapeArea_->setAutoRepaint(false);


        /* Bug: Non-added panels must be marked invisible. */
        debugPanel_->setVisible(false);
        inputPanel_->setVisible(false);
        badProgramPanel_->setVisible(false);

        setState(UIState::CHOOSE_PROGRAM);
        mainPanel_->setWidth(window.getWidth() * 0.95);

        /* Fake a resize to get the chrome to be the right size. */
        windowResized();
    }

    DebugGUI::~DebugGUI() {
        timer_->stop();
    }

    void DebugGUI::actionPerformed(GObservable* source) {
        /* The "load" button always works. */
        if (source == loadButton_) {
            userLoadProgram();
        } else if (state_ == UIState::CHOOSE_INPUT) {
            if (source == startButton_) {
                initSimulation(inputField_->getText());
            }
        } else if (state_ == UIState::RUNNING_PAUSE || state_ == UIState::RUNNING_PLAY) {
            if (source == step_) {
                setState(UIState::RUNNING_PAUSE);
                step();
            } else if (source == playPause_) {
                setState(state_ == UIState::RUNNING_PAUSE? UIState::RUNNING_PLAY : UIState::RUNNING_PAUSE);
            } else if (source == stop_) {
                setState(UIState::CHOOSE_INPUT);
            } else if (source == toBeginning_) {
                initSimulation(lastInput_);
            } else if (source == toEnd_) {
                setState(UIState::RUNNING_PAUSE);
                seekToEnd();
            }
        }
    }

    void DebugGUI::initSimulation(const string& strInput) {
        /* Build the input string. */
        vector<char32_t> input;
        for (char32_t ch: utf8Reader(strInput)) {
            input.push_back(ch);
        }

        lastInput_ = strInput;
        interpreter_ = make_shared<Turing::Interpreter>(*program_, input);

        /* One buffer cell so going before the start of the tape doesn't shift. */
        lowIndex_ = -kTapeHeadMargin - 1;

        numSteps_ = 0;
        updateStatusLine();

        setState(UIState::RUNNING_PAUSE);
        requestRepaint();
    }

    GRectangle DebugGUI::programArea() const {
        return { 0, 0, console_->getWidth(), console_->getHeight() };
    }
    GRectangle DebugGUI::tapeArea() const {
        return { 0, 0, tapeArea_->getWidth(), tapeArea_->getHeight() };
    }

    void DebugGUI::repaint() {
        GThread::runOnQtGuiThread([&] {
            clearDisplay(window(), kBackgroundColor);

            /* Zoom to the correct line. */
            if (interpreter_ && interpreter_->lineNumber() != lastLine_) {
                auto bounds = programArea();

                /* Center on the middle of the line. */
                double y = bounds.y + kCodeLineHeight * (interpreter_->lineNumber() + 0.5);

                lastLine_ = interpreter_->lineNumber();

                qScroller->ensureVisible(0, y, 0, kScrollMargin);
            }

            switch (state_) {
                case UIState::CHOOSE_INPUT:
                case UIState::RUNNING_PAUSE:
                case UIState::RUNNING_PLAY:
                case UIState::BAD_PROGRAM:
                    drawProgramArea();
                    drawTapeArea();
                    break;

                case UIState::CHOOSE_PROGRAM:
                    // TODO: This!
                    break;

                default:
                    break;
            }

            console_->repaint();
            tapeArea_->repaint();
        });
    }

    void DebugGUI::changeOccurredIn(GObservable* source) {
        if (source == speedControl_) {
            int index = speedControl_->getValue();
            tie(timeDelay_, frameSkip_) = kAnimationSpeeds.at(index);

            timer_->setDelay(timeDelay_);
        }
    }

    void DebugGUI::drawProgramArea() {
        console_->setColor(kCodeBackgroundColor);
        console_->fillRect(programArea());

        /* How many lines can we fit? */
        auto bounds = programArea();

        for (size_t i = 0; i < guiProgramLines_.size(); i++) {
            /* Highlight the current line. */
            if (interpreter_ && i == interpreter_->lineNumber()) {
                GRectangle lineBounds = {
                    bounds.x, bounds.y + i * kCodeLineHeight,
                    bounds.width, kCodeLineHeight
                };

                console_->setColor(kActiveLineColor);
                console_->fillRect(lineBounds);
            }

            guiProgramLines_[i]->draw(console_);
        }
    }

    void DebugGUI::drawTapeArea() {
        GRectangle bounds = tapeArea();

        /* Wipe anything in the tape area that may have overdrawn. */
        tapeArea_->setColor(kBackgroundColor);
        tapeArea_->fillRect(bounds);

        /* No interpreter? Nothing to draw. */
        if (interpreter_) {
            /* Determine the width to use, done in graphics coordinates. We're chosen so that
             *
             * (1) we don't end up with total width greater than the screen size, and
             * (2) we don't exceed the height of the character area.
             *
             * The calculation in (1) adds one to the total to account for the arrow.
             */
            double width = bounds.width / double(kNumChars + 1);
            width = min(width, bounds.height / 2 - 2 * bounds.height * kCharY);

            double baseX = bounds.x + (bounds.width - width * kNumChars) / 2.0;
            double baseY = bounds.y;

            /* Draw each character. */
            for (size_t i = 0; i < kNumChars; i++) {
                char32_t ch;
                if (i == 0 || i == kNumChars - 1) {
                    ch = ELLIPSIS;
                } else {
                    ch = interpreter_->tapeAt(i + lowIndex_);
                    if (ch == Turing::kBlankSymbol) {
                        ch = ' '; // Render as if it's a space
                    }
                }
                drawSingleCharacter(ch, { baseX + width * i, baseY + kCharY * bounds.height, width, width });
            }

            /* Draw the arrow at the current position. */
            int64_t offset = interpreter_->tapeHeadPos() - lowIndex_;
            drawArrow({ baseX - width / 2 + width * (0.5 + offset), baseY + bounds.height * kCharY + width, width, width });
        }
    }

    void DebugGUI::drawSingleCharacter(char32_t ch, const GRectangle& bounds) {
        /* Draw the bounding box. */
        tapeArea_->setColor(kCharBackgroundColor);
        tapeArea_->fillRect(bounds);
        tapeArea_->setColor(kCharBorderColor);
        tapeArea_->drawRect(bounds);

        /* Draw the text. */
        auto text = TextRender::construct(toUTF8(ch), bounds, kCharFont);
        text->alignCenterVertically();
        text->alignCenterHorizontally();
        text->draw(tapeArea_);
    }

    void DebugGUI::drawArrow(const GRectangle& bounds) {
        GPolygon polygon;
        polygon.setFilled(true);
        polygon.setColor(kArrowColor);

        /* Box origin point. */
        GPoint box = { bounds.x, bounds.y };

        /* Box coordinate transform:
         *  | width    0   |
         *  |   0   height |
         */
        GMatrix transform = { bounds.width, 0, 0, bounds.height };
        for (const auto& v: kArrow) {
            polygon.addVertex(box + transform * v);
        }

        tapeArea_->draw(&polygon);
    }

    void DebugGUI::step() {
        if (interpreter_->state() == Turing::Result::RUNNING) {
            interpreter_->step();
            numSteps_++;

            /* Constrain the tape head position. */
            if (interpreter_->tapeHeadPos() - lowIndex_ < kTapeHeadMargin) {
                lowIndex_--;
            } else if (interpreter_->tapeHeadPos() - lowIndex_ >= kNumChars - kTapeHeadMargin) {
                lowIndex_++;
            }
        }

        /* Don't do graphics intensive things during fast mode. */
        if (!fastMode_) {
            updateStatusLine();
            requestRepaint();
        }
    }

    void DebugGUI::setState(UIState state) {
        if (state == state_) return; // Nothing to do

        /* If we're currently in the "play" state, stop the timer. */
        if (state_ == UIState::RUNNING_PLAY) {
            timer_->stop();
        }

        if (state == UIState::CHOOSE_PROGRAM) {
            setPanel(emptyPanel_);
        } else if (state == UIState::BAD_PROGRAM) {
            setPanel(badProgramPanel_);
        } else if (state == UIState::CHOOSE_INPUT) {
            setPanel(inputPanel_);
        } else if (state == UIState::RUNNING_PAUSE) {
            playPause_->setText("▶");

            setPanel(debugPanel_);
        } else if (state == UIState::RUNNING_PLAY) {
            playPause_->setText("⏸");

            /* If switching to the "play" state, engage the timer. */
            setPanel(debugPanel_);
            timer_->start();
        }

        state_ = state;

        /* Repaint; we may have a totally different look now! */
        requestRepaint();
    }

    void DebugGUI::timerFired() {
        if (state_ == UIState::RUNNING_PLAY) {
            /* Temporarily engage fast mode when frameskip is bigger than one. */
            fastMode_ = true;
            for (size_t i = 0; i < frameSkip_ - 1; i++) {
                step();
            }

            /* Drop out of fast mode. */
            fastMode_ = false;
            step();

            if (interpreter_->state() != Turing::Result::RUNNING) {
                setState(UIState::RUNNING_PAUSE);
            }
        }
    }

    void DebugGUI::setPanel(GContainer* panel) {
        GThread::runOnQtGuiThread([&] {
            /* Already installed? Nothing to do. */
            if (activePanel_ == panel) return;

            if (activePanel_) {
                currentPanel_->remove(activePanel_);
                activePanel_->setVisible(false);
            }

            /* Handle bug with the input panel. */
            if (panel == inputPanel_) {
                resetInputPanel();
                panel = inputPanel_;
            }

            /* Set this as the active panel. */
            currentPanel_->add(panel);
            activePanel_ = panel;
            panel->setVisible(true);

            /* Fake a resize; the content area size may have changed. */
            windowResized();
        });
    }

    /* There appears to be an internal bug where removing an input
     * element and adding it triggers a Qt internal warning. Destroy and
     * then restore the input field.
     */
    void DebugGUI::resetInputPanel() {
        /* Recycle? */
        string contents;
        if (inputField_) {
            contents = inputField_->getText();
            inputPanel_->clear();
            delete inputField_;
        }
        /* Fresh? */
        else {
            startButton_ = new GButton("Debug");
        }
        inputField_ = new GTextField(contents);
        inputField_->setPlaceholder("ε");

        inputPanel_->add(new GLabel("Input: "));
        inputPanel_->add(inputField_);
        inputPanel_->add(startButton_);
    }

    void DebugGUI::userLoadProgram() {
        /* Ask user to pick a file; don't do anything if they don't pick one. */
        string filename = GFileChooser::showOpenDialog(&window(), "Choose Program", "res/", "*.tm");
        if (filename == "") return;

        loadProgram(filename);
    }

    void DebugGUI::loadProgram(const string& filename) {
        ifstream input(filename, ios::binary);
        if (!input) error("Could not open the file " + filename);

        program_ = make_shared<Turing::Program>(input);
        interpreter_ = nullptr;
        currProgramLabel_->setText(getTail(filename));

        buildProgramLines();

        setState(program_->isValid()? UIState::CHOOSE_INPUT : UIState::BAD_PROGRAM);
        requestRepaint();
    }

    void DebugGUI::seekToEnd() {
        setDemoOptionsEnabled(false);
        mainPanel_->setEnabled(false);
        fastMode_ = true;

        do {
            for (size_t i = 0; i < kStepsOnEndSeek && interpreter_->state() == Turing::Result::RUNNING; i++) {
                step();
            }
            updateStatusLine();
            repaint();
            window().repaint();
        } while(interpreter_->state() == Turing::Result::RUNNING &&
                GOptionPane::showConfirmDialog(window().getWidget(), kContinueMessage, kContinueTitle) == GOptionPane::CONFIRM_YES);

        fastMode_ = false;
        mainPanel_->setEnabled(true);
        setDemoOptionsEnabled(true);
        requestRepaint();
    }

    void DebugGUI::windowResized() {
        auto size = window().getRegionSize("CENTER");

        /* Resize chrome. */
        tapeArea_->setSize(size.width * 0.95, size.height * 0.95 * (1 - kProgramHeight));
        consoleScroller_->setSize(size.width * 0.95, size.height * 0.95 * kProgramHeight);
        console_->setWidth(consoleScroller_->getWidth());
        ProblemHandler::windowResized();
    }

    void DebugGUI::buildProgramLines() {
        guiProgramLines_.clear();

        auto bounds = programArea();

        size_t line = 0;
        for (size_t i = 0; i < program_->numLines(); i++) {
            GRectangle lineBounds = {
                bounds.x, bounds.y + line * kCodeLineHeight,
                numeric_limits<double>::infinity(), kCodeLineHeight
            };

            /* Good code? Handle it as usual. */
            if (program_->errorAtLine(i) == "") {
                auto text = TextRender::construct(program_->line(i), lineBounds, kCodeFont, LineBreak::NO_BREAK_SPACES);
                text->alignCenterVertically();
                guiProgramLines_.push_back(text);
            }
            /* Code has an error. */
            else {
                auto text = TextRender::construct(program_->line(i), lineBounds, kBadCodeFont, LineBreak::NO_BREAK_SPACES);
                text->alignCenterVertically();
                guiProgramLines_.push_back(text);

                line++;
                lineBounds.y += kCodeLineHeight;

                text = TextRender::construct(program_->errorAtLine(i), lineBounds, kErrorFont, LineBreak::NO_BREAK_SPACES);
                text->alignTop();
                guiProgramLines_.push_back(text);
            }

            line++;
        }

        console_->setSize(tapeArea_->getWidth(), guiProgramLines_.size() * kCodeLineHeight);
    }

    void DebugGUI::updateStatusLine() {
        ostringstream builder;
        if (interpreter_->state() == Turing::Result::ACCEPT) {
            builder << "Accepted input \"" << lastInput_ << "\" after ";
        } else if (interpreter_->state() == Turing::Result::REJECT) {
            builder << "Rejected input \"" << lastInput_ << "\" after ";
        } else {
            builder << "Running: ";
        }

        builder << pluralize(numSteps_, "step") << ".";
        statusLine_->setText(builder.str());
    }
}

GRAPHICS_HANDLER("Turing Debugger", GWindow& window) {
    return make_shared<DebugGUI>(window);
}

namespace {
    vector<string> allTMs() {
        vector<string> result;

        for (string file: listDirectory("res/")) {
            if (endsWith(file, ".tm")) {
                result.push_back("res/" + file);
            }
        }

        for (string file: listDirectory("res/tm-samples/")) {
            if (endsWith(file, ".tm")) {
                result.push_back("res/tm-samples/" + file);
            }
        }

        return result;
    }

    void displayInterpreter(const Turing::Program& tm, const Turing::Interpreter& interpreter) {
        cout << "Currently on line " << (interpreter.lineNumber() + 1) << ": " << tm.line(interpreter.lineNumber()) << endl;

        cout << "Contents of tape near tape head (boxed symbol denotes tape position):" << endl;
        for (int i = -15; i <= +15; i++) {
            if (i == 0) cout << '[';

            auto ch = interpreter.tapeAt(interpreter.tapeHeadPos() + i);
            cout << (ch == Turing::kBlankSymbol? " " : toUTF8(ch));

            if (i == 0) cout << ']';
        }

        cout << endl;
    }

    int getNumSteps() {
        while (true) {
            int result = getInteger("Enter the number of steps to advance, or 0 to abort:" );
            if (result < 0) {
                cerr << "Please enter a nonnegative integer." << endl;
            }
            else {
                return result;
            }
        }
    }

    void tmREPL(const Turing::Program& p) {
        do {
            string input = getLine("Enter input string: ");

            Turing::Interpreter interpreter(p, { input.begin(), input.end() });

            uint64_t stepCount = 0;
            while (interpreter.state() == Turing::Result::RUNNING) {
                displayInterpreter(p, interpreter);

                int numSteps = getNumSteps();
                if (numSteps == 0) break;

                for (int i = 0; i < numSteps && interpreter.state() == Turing::Result::RUNNING; i++) {
                    stepCount++;
                    interpreter.step();
                }
            }

            if (interpreter.state() == Turing::Result::ACCEPT) {
                cout << "Accepted input \"" << input << "\" after " << pluralize(stepCount, "step") << "." << endl;
            } else if (interpreter.state() == Turing::Result::REJECT) {
                cout << "Rejecting input \"" << input << "\" after " << pluralize(stepCount, "step") << "." << endl;
            } else if (interpreter.state() == Turing::Result::RUNNING) {
                cout << "TM was still running on \"" << input << "\" after " << pluralize(stepCount, "step") << "." << endl;
            } else {
                error("Unknown TM result?");
            }
        } while (getYesOrNo("Run this TM on another string? "));
    }

    void consoleRunTM(const string& filename) {
        Turing::Program p(ifstream(filename, ios::binary));
        if (!p.isValid()) {
            cerr << "This TM contains errors and cannot be run." << endl;
            for (size_t i = 0; i < p.numLines(); i++) {
                string error = p.errorAtLine(i);
                if (error != "") {
                    cout << "Line " << (i+1) << ": " << p.line(i) << endl;
                    cerr << "  " << error << endl;
                }
            }
        } else {
            tmREPL(p);
        }
    }
}


CONSOLE_HANDLER("TM Debugger") {
    do {
        auto options = allTMs();
        int choice = makeSelectionFrom("Choose a TM to run:", options);

        consoleRunTM(options[choice]);
    } while (getYesOrNo("Debug another TM? "));
}
