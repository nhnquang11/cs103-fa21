/* Routines to run a series of private tests. */
#pragma once

#include <istream>
#include <functional>
#include <string>

/* Runs the callback on the indicated test. */
void runPrivateTest(const std::string& testName,
                    std::function<void(std::istream&)> callback);
