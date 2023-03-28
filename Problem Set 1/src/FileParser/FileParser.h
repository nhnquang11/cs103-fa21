/**********************************************************************
 * File: FileParser.h
 * Author: Keith Schwarz (htiek@cs.stanford.edu)
 *
 * Logic to read one of the formatted data files containing answers to
 * problems.
 */
#ifndef FileParser_Included
#define FileParser_Included

#include <map>
#include <memory>
#include <utility>
#include <istream>
#include <string>

/**
 * Given a stream, parses the stream into individual labeled units, returning
 * the result is a collection of streams, one per unit.
 *
 * This code will remove all comments from these units. Comments consist of
 * hash marks (#) followed by any text.
 *
 * However, this code will not strip out newlines.
 *
 * If something goes wrong reading the file, this code will throw a
 * std::runtime_error exception.
 */
std::map<std::string, std::shared_ptr<std::istream>> parseFile(std::istream& source);

/* Convenience wrapper to read directly from a file. */
std::map<std::string, std::shared_ptr<std::istream>> parseFile(const std::string& filename);

#endif
