# INPUT:  A string made of { and }.
# OUTPUT: Whether the string is a string of balanced braces.
#
# Given a string of open and close braces, check if those braces are
# balanced.
#
# We use the following strategy: scan the string for the first close
# brace. Whatever open brace comes before it must match it. So cross
# both off and repeat until we've either crossed everything off or
# we're left with an unmatched open or close brace.

Start:
    If Blank Goto FinalCheck
    If '}' Goto FindMatch
    Move Right
    Goto Start

# We've found a closing brace. Find an open brace that matches is.
FindMatch:
    Write 'x' # Cross off rather than use a blank

FindMatchLoop:
    Move Left
    If Blank Return False # Oops, no open to match it
    If 'x' Goto FindMatchLoop

    # Cross off the open brace
    Write 'x'
    Goto Start

# There are no more close braces. Are there any more open braces?
FinalCheck:
    Move Left
    If Blank Return True
    If '{' Return False
    Goto FinalCheck
