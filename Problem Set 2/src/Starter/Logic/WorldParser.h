/**
 * File: WorldParser.h
 * Author: Keith Schwarz (htiek@cs.stanford.edu)
 *
 * Exports functions needed to parse a file containing a description of a world
 * (a collection of people, cats, and robots) into a World object.
 */
#ifndef WorldParser_Included
#define WorldParser_Included

#include "Entity.h"
#include <istream>
#include <ostream>

/* Loads a world from a stream, reporting an error if it's malformed. */
World parseWorld(std::istream& source);

/* Writes a world to a stream. */
std::ostream& operator<< (std::ostream& out, const World& world);

#endif
