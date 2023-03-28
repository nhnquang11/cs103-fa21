#include "FileParser.h"
#include "StrUtils/StrUtils.h"
#include <sstream>
#include <stdexcept>
#include <fstream>
using namespace std;

namespace {
    string clean(string line) {
        /* Any comments to remove? */
        auto index = line.find('#');
        if (index != string::npos) line.erase(index);

        /* Strip out whitespace. */
        line = Utilities::trim(line);
        return line;
    }

    /* We're a section header if we have the form [text], where text doesn't contain ]. */
    bool isSectionHeader(const string& header) {
        return header.size()  >= 2 &&
               header.front() == '[' &&
               header.back()  == ']' &&
               header.find(']') == header.size() - 1;
    }
}

map<string, shared_ptr<istream>> parseFile(istream& source) {
    map<string, shared_ptr<istream>> result;

    string section;  // What section we're in; empty means "not in a section."
    string contents; // Contents of that section.

    for (string line; getline(source, line); ) {
        line = clean(line);

        /* Skip empty lines. */
        if (line.empty()) continue;

        /* If this is a section header, we just finished the last section. */
        if (isSectionHeader(line)) {
            if (!section.empty()) {
                result[section] = make_shared<istringstream>(contents);
                contents.clear();
            }

            /* Make sure this isn't a duplicate. */
            section = line;
            if (result.count(section)) throw runtime_error("Duplicate section: " + section);
        }
        /* Otherwise, treat it as payload. */
        else {
            /* Not in a section? This is a syntax error. */
            if (section.empty()) throw runtime_error("Text found in file that isn't in a section.");

            /* Otherwise, extend the contents. */
            contents += line;
            contents += '\n';
        }
    }

    /* Close out the current section, if one exists. */
    if (!section.empty()) {
        result[section] = make_shared<istringstream>(contents);
    }

    return result;
}

map<string, shared_ptr<istream>> parseFile(const string& filename) {
   ifstream input(filename);
   if (!input) throw runtime_error("Couldn't open file " + filename);

   return parseFile(input);
}
