/* Utilities for making the editor/tester/debugger work seamlessly
 * together.
 */
#pragma once

#include <string>

namespace ProgramCore {
    /* For simplicity, the UI caches the name of the most recently viewed program.
     * These functions let you set/retrieve that name.
     */
    void setLastFilename(const std::string& filename);
    std::string lastFilename(); // Empty string if none selected
}
