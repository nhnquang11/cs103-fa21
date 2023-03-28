/**
 * File: ObjectParser.h
 * Author: Keith Schwarz (htiek@cs.stanford.edu)
 *
 * Exports functions needed to parse a file containing a description of an
 * object into an Object.
 */
#pragma once

#include "SetInternal.h"
#include <istream>

/* Loads an object from a stream, reporting an error if it's malformed. */
namespace SetTheory {
    Object parse(std::istream& source);
}
