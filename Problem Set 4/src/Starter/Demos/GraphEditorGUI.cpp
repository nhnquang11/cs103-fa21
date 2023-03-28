#include "../GUI/MiniGUI.h"
#include "../GraphEditor/GraphEditor.h"
#include <fstream>
#include <algorithm>
#include "filelib.h"
using namespace std;
using namespace MiniGUI;

namespace {
    const string kUnsavedChanges = "You have unsaved changes.\n\nDo you want to save?";
    const string kUnsavedChangesTitle = "Unsaved Changes";

    const string kWelcome = R"(Welcome to the Graph Editor!

                            Click "Load Graph" to choose a graph.)";

    const Font kWelcomeFont(FontFamily::SERIF, FontStyle::BOLD_ITALIC, 24, "#4C5866"); // Marengo

    const string kInstructions = R"(Double-click to add a node.)";
    const Font kInstructionsFont = kWelcomeFont;

    const string kBackgroundColor = "white";

    class GraphEdit: public ProblemHandler {
    public:
        GraphEdit(GWindow& window);

        /* Forward to the relevant listener. */
        void mouseDoubleClicked(double x, double y) override;
        void mouseMoved(double x, double y) override;
        void mousePressed(double x, double y) override;
        void mouseDragged(double x, double y) override;
        void mouseReleased(double x, double y) override;

        void actionPerformed(GObservable* source) override;
        void windowResized() override;

        bool shuttingDown() override;

    protected:
        void repaint() override;

    private:
        shared_ptr<GraphEditor::Editor<>> editor;

        /* Panel layout:
         *
         * [name of file]
         * [save button ] [              delete button                  ]
         * [load button ]
         */
        Temporary<GContainer> controls;
        GLabel*  fileLabel;
        GButton* saveButton;
        GButton* loadButton;
        GButton* deleteButton;

        void dirty(bool isDirty = true);
        void entitySelected(GraphEditor::Entity* entity);
        void entityCreated(GraphEditor::Entity* entity);

        void initEditor(istream& in);
        void initChrome();

        void deleteSelected();
        void setEditorBounds();

        void drawWelcomeMessage();
        void drawInstructions();

        GRectangle contentArea();

        void userLoad();
        void load(const string& filename);

        void userSave();
        void save();

        bool handleUnsavedChanges();

        /* Is anything selected? */
        bool somethingSelected = false;

        /* Dirty bit. */
        bool isDirty = false;

        /* Current filename. */
        string currFilename;
    };

    GraphEdit::GraphEdit(GWindow& window) : ProblemHandler(window) {
        initChrome();
        dirty(false);
    }

    void GraphEdit::initChrome() {
        GContainer* leftPanel = new GContainer(GContainer::LAYOUT_FLOW_VERTICAL);
        fileLabel = new GLabel("Choose a Graph");
        saveButton = new GButton("Save Graph");
        saveButton->setEnabled(false);
        loadButton = new GButton("Load Graph");

        leftPanel->add(fileLabel);
        leftPanel->add(saveButton);
        leftPanel->add(loadButton);

        deleteButton = new GButton("Delete");

        controls = make_temporary<GContainer>(window(), "SOUTH", GContainer::LAYOUT_GRID);
        controls->addToGrid(leftPanel, 0, 0, 3, 1);
        controls->addToGrid(deleteButton, 1, 1);

        controls->setWidth(window().getWidth() * 0.9);

        /* Clarify that nothing is currently selected. */
        entitySelected(nullptr);
    }

    void GraphEdit::initEditor(istream& in) {
        editor = make_shared<GraphEditor::Editor<>>(make_shared<GraphEditor::Viewer<>>(JSON::parse(in)));

        class Listener: public GraphEditor::Listener {
        public:
            Listener(GraphEdit* owner) : owner(owner) {

            }

            void needsRepaint() {
                owner->requestRepaint();
            }
            void isDirty() {
                owner->dirty();
            }
            void entitySelected(GraphEditor::Entity* entity) {
                owner->entitySelected(entity);
            }
            void entityCreated(GraphEditor::Entity* entity) {
                owner->entityCreated(entity);
            }

        private:
            GraphEdit* owner;
        };

        editor->addListener(make_shared<Listener>(this));
        setEditorBounds();
        entitySelected(nullptr);
        dirty(false);
        requestRepaint();
    }

    void GraphEdit::mouseDoubleClicked(double x, double y) {
        if (editor) editor->mouseDoubleClicked(x, y);
    }
    void GraphEdit::mouseMoved(double x, double y) {
        if (editor) editor->mouseMoved(x, y);
    }
    void GraphEdit::mousePressed(double x, double y) {
        if (editor) editor->mousePressed(x, y);
    }
    void GraphEdit::mouseDragged(double x, double y) {
        if (editor) editor->mouseDragged(x, y);
    }
    void GraphEdit::mouseReleased(double x, double y) {
        if (editor) editor->mouseReleased(x, y);
    }

    void GraphEdit::repaint() {
        clearDisplay(window(), kBackgroundColor);
        if (editor) {
            if (editor->viewer()->numNodes() == 0) {
                drawInstructions();
            } else {
                editor->draw(window().getCanvas());
            }
        } else {
            drawWelcomeMessage();
        }
    }

    void GraphEdit::entitySelected(GraphEditor::Entity* entity) {
        somethingSelected = !!entity;
        deleteButton->setEnabled(somethingSelected);
    }

    void GraphEdit::entityCreated(GraphEditor::Entity* entity) {
        if (auto* node = dynamic_cast<GraphEditor::Node*>(entity)) {
            node->label(string(1, 'a' + node->index()));
        }
    }

    void GraphEdit::actionPerformed(GObservable* source) {
        /* TODO: For development only! */
        if (source == loadButton) {
            userLoad();
        } else if (editor && source == saveButton) {
            userSave();
        } else if (editor && source == deleteButton) {
            if (somethingSelected) deleteSelected();
        }
    }

    void GraphEdit::deleteSelected() {
        if (editor->selectedNode()) {
            editor->deleteNode(editor->selectedNode());
        } else if (editor->selectedEdge()) {
            editor->deleteEdge(editor->selectedEdge());
        } else {
            error("Something is selected, but no node or edge is selected?");
        }
    }

    void GraphEdit::setEditorBounds() {
        if (editor) editor->viewer()->setBounds(contentArea());
    }

    void GraphEdit::windowResized() {
        setEditorBounds();
        ProblemHandler::windowResized();
    }

    void GraphEdit::drawWelcomeMessage() {
        auto render = TextRender::construct(kWelcome, contentArea(), kWelcomeFont);
        render->alignCenterVertically();
        render->alignCenterHorizontally();
        render->draw(window());
    }

    void GraphEdit::drawInstructions() {
        auto render = TextRender::construct(kInstructions, contentArea(), kInstructionsFont);
        render->alignCenterVertically();
        render->alignCenterHorizontally();
        render->draw(window());
    }

    GRectangle GraphEdit::contentArea() {
        return { 0, 0, window().getCanvasWidth(), window().getCanvasHeight() };
    }

    /* Serializes the current state of things. */
    void GraphEdit::save() {
        /* TODO: Don't overwrite the source until the write has finished.
         * Use mkstemp, write there, and then do a move when done.
         */
        ofstream output(currFilename);
        if (!output) error("Cannot open " + currFilename + " for writing.");

        output << editor->viewer()->toJSON();
    }

    void GraphEdit::userSave() {
        save();

        dirty(false);
        GOptionPane::showMessageDialog(&window(), "Graph " + currFilename + " was saved!");
    }

    bool GraphEdit::handleUnsavedChanges() {
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

        /* Otherwise, they intend to save. */
        userSave();
        return true;
    }

    void GraphEdit::load(const string& filename) {
        currFilename = filename;
        fileLabel->setLabel(getTail(currFilename));

        ifstream input(filename);
        if (!input) throw runtime_error("Error opening file: " + filename);
        initEditor(input);
    }

    void GraphEdit::userLoad() {
        /* Warn user about unsaved changes. */
        if (!handleUnsavedChanges()) {
            return;
        }

        /* Ask user to pick a file; don't do anything if they don't pick one. */
        string filename = GFileChooser::showOpenDialog(&window(), "Choose Graph", "res/", "*.graph");
        if (filename == "") return;

        load(filename);
        saveButton->setEnabled(true);
    }

    void GraphEdit::dirty(bool dirtyBit) {
        if (dirtyBit) {
            if (!isDirty) {
                isDirty = true;
                fileLabel->setText(getTail(currFilename) + "*");
            }
        } else {
            if (isDirty) {
                isDirty = false;
                fileLabel->setText(getTail(currFilename));
            }
        }
    }

    bool GraphEdit::shuttingDown() {
        return handleUnsavedChanges();
    }
}

GRAPHICS_HANDLER("Graph Editor", GWindow& window) {
    return make_shared<GraphEdit>(window);
}

namespace {
    vector<string> allGraphFiles() {
        vector<string> result;
        for (string file: listDirectory("res/")) {
            if (endsWith(file, ".graph")) {
                result.push_back("res/" + file);
            }
        }
        return result;
    }

    shared_ptr<GraphEditor::Viewer<>> loadGraph(const string& filename) {
        ifstream input(filename);
        if (!input) error("Error opening file: " + filename);

        JSON j = JSON::parse(input);
        return make_shared<GraphEditor::Viewer<>>(j);
    }

    void saveGraph(const string& filename, shared_ptr<GraphEditor::Viewer<>> graph) {
        ofstream output(filename);
        output << graph->toJSON();

        if (!output) error("Error saving your answers; contact the course staff.");
    }

    struct AllDoneNow{};

    /* REPL command. */
    struct Command {
        string name;
        string desc;
        int arity;
        function<void(shared_ptr<GraphEditor::Viewer<>>, const Vector<string>&)> command;
    };

    void helpFN(shared_ptr<GraphEditor::Viewer<>>, const Vector<string>&);

    void quitFN(shared_ptr<GraphEditor::Viewer<>>, const Vector<string>&) {
        throw AllDoneNow();
    }

    void printFN(shared_ptr<GraphEditor::Viewer<>> graph, const Vector<string>&) {
        cout << "Nodes: " << endl;

        set<string> nodes;
        graph->forEachNode([&](GraphEditor::Node* node) {
            nodes.insert(node->label());
        });
        for (string node: nodes) {
            cout << node << endl;
        }

        cout << "Edges: " << endl;
        set<pair<string, string>> edges;
        graph->forEachEdge([&](GraphEditor::Edge* edge) {
            edges.insert(make_pair(edge->from()->label(), edge->to()->label()));
        });

        for (const auto& pair: edges) {
            cout << "Between nodes " << pair.first << " and " << pair.second << endl;
        }
    }

    void newNodeFN(shared_ptr<GraphEditor::Viewer<>> graph, const Vector<string>&) {
        string name = "a";
        while (graph->nodeLabeled(name) != nullptr) {
            name[0]++;
        }

        graph->newNode({})->label(name);
        cout << "Created node " << name << "." << endl;
    }

    void newEdgeFN(shared_ptr<GraphEditor::Viewer<>> graph, const Vector<string>& args) {
        auto src = graph->nodeLabeled(args[0]);
        if (!src) {
            cout << "There is no node named " << args[0] << " in this graph." << endl;
            return;
        }

        auto dst = graph->nodeLabeled(args[1]);
        if (!dst) {
            cout << "There is no node named " << args[1] << " in this graph." << endl;
            return;
        }

        if (src == dst) {
            cout << "Self-loops are not permitted." << endl;
            return;
        }

        if (graph->edgeBetween(src, dst) != nullptr) {
            cout << "An edge between those nodes already exists." << endl;
            return;
        }

        graph->newEdge(src, dst);
        cout << "Added an edge between " << args[0] << " and " << args[1] << endl;
    }

    void delNodeFN(shared_ptr<GraphEditor::Viewer<>> graph, const Vector<string>& arg) {
        auto node = graph->nodeLabeled(arg[0]);
        if (node == nullptr) {
            cout << "No node with that name exists in the graph." << endl;
            return;
        }

        graph->removeNode(node);
        cout << "Removed node " << arg[0] << endl;
    }

    void delEdgeFN(shared_ptr<GraphEditor::Viewer<>> graph, const Vector<string>& args) {
        auto src = graph->nodeLabeled(args[0]);
        if (!src) {
            cout << "There is no node named " << args[0] << " in this graph." << endl;
            return;
        }

        auto dst = graph->nodeLabeled(args[1]);
        if (!dst) {
            cout << "There is no node named " << args[1] << " in this graph." << endl;
            return;
        }

        auto edge = graph->edgeBetween(src, dst);
        if (edge == nullptr) {
            cout << "There is no edge between those nodes." << endl;
            return;
        }

        graph->removeEdge(edge);
        cout << "Removed the edge between " << args[0] << " and " << args[1] << endl;
    }

    const vector<Command> kCommands = {
        { "help",    "help: Displays the help menu.",  0, helpFN },
        { "quit",    "quit: Saves and exits.",  0, quitFN },
        { "print",   "print: Prints the graph.", 0, printFN },
        { "newnode", "newnode: Creates a new node. It's assigned the next available letter as a name.", 0, newNodeFN },
        { "newedge", "newedge from to: Creates a new edge between nodes 'from' and 'to'.",  2, newEdgeFN },
        { "delnode", "delnode nodename: Deletes the node 'nodename' and all edges incident to it.", 1, delNodeFN },
        { "deledge", "deledge from to: Deletes the edge between nodes 'from' and 'to'. ", 2, delEdgeFN },
    };

    void helpFN(shared_ptr<GraphEditor::Viewer<>>, const Vector<string>&){
        for (const auto& command: kCommands) {
            cout << command.desc << endl;
        }
    }

    void graphREPL(shared_ptr<GraphEditor::Viewer<>> graph) {
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
                            itr->command(graph, args);
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

    void textEditGraph(const string& filename) {
        auto graph = loadGraph(filename);
        graphREPL(graph);
        saveGraph(filename, graph);
    }
}

CONSOLE_HANDLER("Graph Editor") {
    do {
        auto graphs = allGraphFiles();
        textEditGraph(graphs[makeSelectionFrom("Choose a graph: ", graphs)]);
    } while (getYesOrNo("Edit another graph? "));
}

