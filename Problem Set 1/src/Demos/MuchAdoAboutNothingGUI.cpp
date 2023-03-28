#include "../GUI/MiniGUI.h"
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
      <th colspan="2">Much Ado About Nothing</th>
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
    const vector<pair<string, string>> kParts = {
        { "[Part (i)]",    "&empty; &cup; {&empty;}" },
        { "[Part (ii)]",   "&empty; &cap; {&empty;}" },
        { "[Part (iii)]",  "{&empty;} &cup; {{&empty;}}" },
        { "[Part (iv)]",   "{&empty;} &cap; {{&empty;}}" },
        { "[Part (v)]",    "&weierp;(&weierp;(&empty;))" },
        { "[Part (vi)]",   "&weierp;(&weierp;(&weierp;(&empty;)))" },
    };

    class MuchAdoAboutNothingGUI: public ProblemHandler {
    public:
        MuchAdoAboutNothingGUI(GWindow& window);

    private:
        Temporary<GBrowserPane> display;
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

    /* Given a string representing a set, styles that string by changing the font
     * size whenever entering or leaving a set.
     */
    string depthStyled(const string& input) {
        int size = kBaseFontSize;

        ostringstream builder;
        for (char ch: input) {
            if (ch == '{') {
                size = max(size - kFontDelta, kMinFontSize);
                builder << "{" << format(kStartSizeChange, to_string(size));
            } else if (ch == '}') {
                builder << kEndSizeChange << "}";
                size = min(kBaseFontSize, size + kFontDelta);
            } else {
                builder << ch;
            }
        }

        return builder.str();
    }

    /* Style for a row - makes things look all zebralike! */
    string styleFor(int row) {
        return format("background-color:%s;border: 3px solid black; border-collapse:collapse;",
                      row % 2 == 0? "#ffff80" : "white");
    }

    const string kMissingSection =
            R"(<span style="color:#800000;"><b><i>Missing section: %s</i></b></span>)";

    const string kParseError =
            R"(<span style="color:#800000;"><b><i>Parse error: %s</i></b></span>)";

    /* Styles and returns the student's answer to a given problem part. */
    string styledAnswerFor(const string& section) {
        /* Get the data. */
        auto contents = parseFile("res/MuchAdoAboutNothing.sets");
        if (!contents.count(section)) {
            return format(kMissingSection, section);
        }

        string answer;
        try {
            ostringstream builder;
            builder << SetTheory::parse(*contents.at(section));
            answer = builder.str();
        } catch (const exception& e) {
            return format(kParseError, e.what());
        }

        return depthStyled(answer);
    }

    const string kRowTemplate =
            R"(<tr style="%s">
                   <td style="border-right:1px solid black;">%s</td>
                   <td>%s</td>
               </tr>)";

    /* Produces the contents of the table to display. */
    string tableContents() {
        ostringstream result;
        int row = 0;
        for (const auto& entry: kParts) {
            result << format(kRowTemplate, styleFor(row), entry.second, styledAnswerFor(entry.first));
            row++;
        }
        return result.str();
    }

    MuchAdoAboutNothingGUI::MuchAdoAboutNothingGUI(GWindow& window) : ProblemHandler(window) {
        display = make_temporary<GBrowserPane>(window, "CENTER");

        istringstream input(format(kHTMLTemplate, kBaseFontSize, tableContents()));
        display->readTextFromFile(input);
    }
}

GRAPHICS_HANDLER("Much Ado About Nothing", GWindow& window) {
    return make_shared<MuchAdoAboutNothingGUI>(window);
}

CONSOLE_HANDLER("Much Ado About Nothing") {
    auto parts = parseFile("res/MuchAdoAboutNothing.sets");

    Map<string, string> contents;
    for (const auto& part: kParts) {
        try {
            if (!parts.count(part.first)) {
                error("No section named " + part.first + " in MuchAdoAboutNothing.sets");
            }

            ostringstream builder;
            builder << SetTheory::parse(*parts.at(part.first));
            contents[part.first] = builder.str();
        } catch (const exception& e) {
            contents[part.first] = "ERROR: " + string(e.what());
        }
    }

    cout << "Contents of MuchAdoAboutNothing.sets:" << endl;
    for (const auto& part: kParts) {
        cout << part.first << ": " << contents[part.first] << endl;
    }
}
