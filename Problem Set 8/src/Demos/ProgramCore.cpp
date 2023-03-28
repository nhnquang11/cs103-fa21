#include "ProgramCore.h"
#include <vector>
using namespace std;

namespace ProgramCore {
    namespace {
        string theLastFile;
    }

    string lastFilename() {
        return theLastFile;
    }
    void setLastFilename(const string& filename) {
        theLastFile = filename;
    }
}
