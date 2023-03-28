#include "../GUI/MiniGUI.h"
#include "../SetTheory.h"
#include "../FileParser/FileParser.h"
#include "../SetTheory/ObjectParser.h"
#include "gbrowserpane.h"
#include <sstream>
using namespace std;

namespace {

    /* Basic HTML format. */
    const string kHTMLTemplate =
R"(<html>
    <head>
    </head>
    <body style="color:black;background-color:white;font-size:%spt;">
    <table cellpadding="3" cellspacing="0" align="center">
    <tr>
      <th colspan="2">Executable Set Theory</th>
    </tr>
    <tr>
      <td>%s</td>
      <td>%s</td>
    </tr>
    <tr>
      <td><i>Expression</i></td>
      <td><i>Your Answer</i></td>
    </tr>
    %s
    </table>
    </body>
    </html>)";

    /* Base font size. */
    const int kBaseFontSize = 24;

    /* Section headers. */
    using SetRelation = function<bool(Object, Object)>;
    const vector<pair<SetRelation, string>> kTestFunctions = {
        { isElementOf,              "S ∈ T" },
        { isSubsetOf,               "S ⊆ T" },
        { areDisjointSets,          "S ∩ T = ∅" },
        { isSingletonOf,            "S = {T}" },
        { isElementOfPowerSet,      "S ∈ ℘(T)" },
        { isSubsetOfPowerSet,       "S ⊆ ℘(T)" },
        { isSubsetOfDoublePowerSet, "S ⊆ ℘(℘(T))" },
    };


    class ExecutableSetTheoryGUI: public ProblemHandler {
    public:
        ExecutableSetTheoryGUI(GWindow& window);

        void changeOccurredIn(GObservable *) override;

    private:
        Temporary<GBrowserPane> display;

        /* Control panel:
         *
         * +------------------------------+
         * | S = _________  T = _________ |
         * +------------------------------+
         */
        Temporary<GLabel>     sEqualsLabel;
        Temporary<GTextField> sInput;
        Temporary<GLabel>     tEqualsLabel;
        Temporary<GTextField> tInput;

        void updateHTML();
    };

    /* String formatter routine. */
    string format(const string& pattern) {
        /* If there's a replacement site, something is wrong. */
        if (pattern.find("%s") != string::npos) {
            error("Unmatched pattern string?");
        }
        return pattern;
    }

    template <typename First, typename... Args>
    string format(const string& pattern, First&& first, Args&&... args) {
        size_t toReplace = pattern.find("%s");
        if (toReplace == string::npos) {
            error("No pattern to replace?");
        }

        return    pattern.substr(0, toReplace)
                + to_string(std::forward<First>(first))
                + format(pattern.substr(toReplace + 2), args...);
    }

    /* Step size and limits to font sizes. */
    const int kFontDelta = 3;
    const int kMinFontSize = 8;

    const string kStartSizeChange = R"(<span style="font-size:%spt">)";
    const string kEndSizeChange   = "</span>";

    const string kParseError =
            R"(%s = <span style="color:#808080;"><i>parse error</i></span>)";

    string toString(Object obj) {
        ostringstream builder;
        builder << obj;
        return builder.str();
    }

    /* Pretty-prints the given object. */
    string prettyObject(Object obj, int fontSize) {
        /* Not a set? Just display it as usual. */
        if (!isSet(obj)) {
            return toString(obj);
        }

        /* Is a set? List its contents. */
        ostringstream result;
        fontSize = max(fontSize - kFontDelta, kMinFontSize);
        result << "{" << format(kStartSizeChange, to_string(fontSize));

        /* Convert contents. */
        vector<string> pieces;
        for (auto elem: asSet(obj)) {
            pieces.push_back(prettyObject(elem, fontSize));
        }

        /* Assemble contents. */
        for (size_t i = 0; i < pieces.size(); i++) {
            result << pieces[i] << (i + 1 == pieces.size()? "" : ", ");
        }

        result << kEndSizeChange << "}";
        return result.str();
    }

    /* Given an object, styles the object and returns it. */
    string styledSet(const string& setName, Object obj) {
        /* Null object? That means we have a parse error. */
        if (!isValid(obj)) {
            return format(kParseError, setName);
        }

        return format("%s = %s", setName, prettyObject(obj, kBaseFontSize));
    }

    /* Style for a row - makes things look all zebralike! */
    string styleFor(int row) {
        return format("background-color:%s;border: 3px solid black; border-collapse:collapse;",
                      row % 2 == 0? "#ffff80" : "white");
    }

    const string kTrue = "true";
    const string kFalse = "false";

    const string kPredError =
            R"(<span style="color:#800000;"><b><i>error</i></b></span>)";

    /* Computes the given function on the given input, reporting the result. */
    string styledAnswerFor(SetRelation pred, Object s, Object t) {
        /* If the objects aren't defined, there's nothing to report. */
        if (!isValid(s) || !isValid(t)) return "";

        /* Try computing the result and see what we get. */
        try {
            return pred(s, t)? kTrue : kFalse;
        } catch (const exception &) {
            return kPredError;
        }
    }

    const string kRowTemplate =
            R"(<tr style="%s">
                   <td style="border-right:1px solid black;">%s</td>
                   <td>%s</td>
               </tr>)";

    /* Produces the contents of the table to display. */
    string tableContents(Object s, Object t) {
        ostringstream result;
        int row = 0;
        for (const auto& entry: kTestFunctions) {
            result << format(kRowTemplate, styleFor(row), entry.second, styledAnswerFor(entry.first, s, t));
            row++;
        }
        return result.str();
    }

    ExecutableSetTheoryGUI::ExecutableSetTheoryGUI(GWindow& window) : ProblemHandler(window) {
        display      = make_temporary<GBrowserPane>(window, "CENTER");
        sEqualsLabel = make_temporary<GLabel>(window, "SOUTH", "S = ");
        sInput       = make_temporary<GTextField>(window, "SOUTH");
        tEqualsLabel = make_temporary<GLabel>(window, "SOUTH", "T = ");
        tInput       = make_temporary<GTextField>(window, "SOUTH");

        updateHTML();
    }

    void ExecutableSetTheoryGUI::changeOccurredIn(GObservable *) {
        updateHTML();
    }

    /* Tries to parse a string as an entity. If successful, returns the entity.
     * Otherwise, reports an error.
     */
    Object tryParse(const string& source) {
        try {
            istringstream input(source);
            return SetTheory::parse(input);
        } catch (const exception &) {
            return {};
        }
    }

    void ExecutableSetTheoryGUI::updateHTML() {
        Object s = tryParse(sInput->getText());
        Object t = tryParse(tInput->getText());

        istringstream input(format(kHTMLTemplate, to_string(kBaseFontSize), styledSet("S", s), styledSet("T", t), tableContents(s, t)));
        display->readTextFromFile(input);
    }
}

GRAPHICS_HANDLER("Executable Set Theory", GWindow& window) {
    return make_shared<ExecutableSetTheoryGUI>(window);
}

namespace {
    Object userReadObject(const string& prompt) {
        while (true) {
            try {
                istringstream converter(getLine(prompt));
                return SetTheory::parse(converter);
            } catch (const exception& e) {
                cerr << "Error: " << e.what() << endl;
            }
        }
    }
}

CONSOLE_HANDLER("Executable Set Theory") {
    do {
        cout << "We will prompt you for a choice of objects S and T, then "
             << "call all of your functions on those choices to see what "
             << "those functions return." << endl;
        Object s = userReadObject("Enter object S: ");
        Object t = userReadObject("Enter object T: ");

        for (const auto& fn: kTestFunctions) {
            cout << fn.second << ": ";
            try {
                bool result = fn.first(s, t);
                cout << boolalpha << result << endl;
            } catch (const exception& e) {
                cerr << "Error: " << e.what() << endl;
            }
        }
    } while (getYesOrNo("Do you want to enter another choice of S and T? "));
}
