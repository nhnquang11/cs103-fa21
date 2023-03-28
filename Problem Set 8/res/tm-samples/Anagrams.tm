# INPUT:  A string of the form w:x, where w, x in {a, b}*.
# OUTPUT: Whether w is an anagram of x.
#
# Given a string of the form w:x, where w, x in {a, b}*, checks whether
# a and b are anagrams of one another. Two strings are anagrams if they
# have the same characters, ignoring order but factoring in frequencies.
# For example, abba, bbaa, baab, baba, and abab are all anagrams of one
# another.
#
# Our strategy is to check whether the input has the right form, then
# to repeatedly cross off one character from the first string and a
# matching character from the second string.

Start:
    If Blank Return False # No : found
    If ':' Goto SecondHalf
    Move Right
    Goto Start

SecondHalf:
    Move Right
    If Blank Goto BackHome # String is good
    If ':' Return False    # Malformed string
    Goto SecondHalf

BackHome:
    Move Left
    If Not Blank Goto BackHome
    Move Right

# Find the next character from the first string and match it in the
# second.
MainLoop:
    If ':' Goto CheckDone
    If 'a' Goto MatchA
    If 'b' Goto MatchB

# Found an 'a', now go match it in the second string.
MatchA:
    Write Blank
MatchAFirst:
    Move Right
    If ':' Goto MatchASecond
    Goto MatchAFirst
MatchASecond:
    Move Right
    If 'a' Goto FoundMatch
    If Blank Return False # No match
    Goto MatchASecond

# Found an 'b', now go match it in the second string.
MatchB:
    Write Blank
MatchBFirst:
    Move Right
    If ':' Goto MatchBSecond
    Goto MatchBFirst
MatchBSecond:
    Move Right
    If 'b' Goto FoundMatch
    If Blank Return False # No match
    Goto MatchBSecond

# Found a match! Cross it off and repeat.
FoundMatch:
    Write 'x'
    Goto BackHome

# All characters on LHS are matched. Make sure there's no unmatched
# characters on the RHS.
CheckDone:
    Move Right
    If Blank Return True    # All matched
    If Not 'x' Return False # Unmatched b
    Goto CheckDone
