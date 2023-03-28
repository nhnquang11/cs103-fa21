# INPUT:  A string of the form w:x, where w, x in {a, b}*.
# OUTPUT: Whether w is a substring of x.
#
# Given an input of the form w:x, where w and x are made of a's and b's, returns whether
# w is a substring of x.
#
# The basic idea is to try matching w in all possible positions within x. To do so, we'll
# use a marking scheme that lets us track which characters we have matched thus far.
# Specifically:
#
# 1. We'll keep track of the next position at which to begin our search in the string by
#    marking the spot before it with a '>' character.
# 2. We'll mark characters that definitely don't match with an 'x' character.
# 3. As we match characters from the input string, we'll convert them to upper-case to
#    signify they're matched.

Start:

# Validate that the input has the right shape.
Needle:
    If Blank Return False # Need a : symbol
    If ':' Goto FoundSeparator
    Move Right
    Goto Needle

FoundSeparator:
    Write '>' # Mark the spot where the next search begins
    Move Right

Haystack:
    If Blank Goto ValidInput
    If ':' Return False # Should have just one : symbol
    Move Right
    Goto Haystack

# We have a valid input. Rewind back to the start.
ValidInput:
    Move Left
    If Not Blank Goto ValidInput
    Move Right

Seek:
    # These characters means we've hit part of the haystack.
    If 'x' Return True
    If '>' Return True

    # Found the next character to process
    If 'a' Goto SeekA
    If 'b' Goto SeekB

    # Already processed this character.
    Move Right
    Goto Seek

SeekA:
    Write 'A' # Replace 'a' with 'A' so we know what we're trying to match
SeekANeedle:
    Move Right
    If Not '>' Goto SeekANeedle

# We are at initial position for our string. We are looking for an a. The next
# unmatched character should be an a. If it isn't, this match has failed.
SeekAHaystack:
    Move Right
    If Blank Return False
    If 'A' Goto SeekAHaystack
    If 'B' Goto SeekAHaystack
    If 'a' Goto MatchA

    # Reading 'b' now. The match has failed. We need to "undo" our partial match
    # and move the start position forward one step.
    Goto FailHaystack

SeekB:
    Write 'B' # Replace 'b' with 'B' so we know what we're trying to match
SeekBNeedle:
    Move Right
    If Not '>' Goto SeekBNeedle

# We are at initial position for our string. We are looking for b. The next
# unmatched character should be b. If it isn't, this match has failed.
SeekBHaystack:
    Move Right
    If Blank Return False
    If 'A' Goto SeekBHaystack
    If 'B' Goto SeekBHaystack
    If 'b' Goto MatchB

    # Reading 'a' now. The match has failed. We need to "undo" our partial match
    # and move the start position forward one step.
    Goto FailHaystack

# Our match has failed on this character. Flip back and try again.
FailHaystack:
    Move Left
    If '>' Goto AdvanceStart
    If 'A' Write 'a'
    If 'B' Write 'b'
    Goto FailHaystack

# Advance the start position forward one step
AdvanceStart:
    Write 'x'
    Move Right
    Write '>'
    Goto FailNeedle

# Undo any changes to our needle so we can try again
FailNeedle:
    Move Left
    If 'A' Write 'a'
    If 'B' Write 'b'
    If Not Blank Goto FailNeedle
    Move Right
    Goto Seek

# We have matched the character we expected to see.
MatchA:
     Write 'A' # Replace a with A to indicate a match.
     Goto FinishedOneMatch

MatchB:
     Write 'B' # Replace b with B to indicate a match.
     Goto FinishedOneMatch

FinishedOneMatch:
    Move Left
    If Not Blank Goto FinishedOneMatch
    Move Right
    Goto Seek
