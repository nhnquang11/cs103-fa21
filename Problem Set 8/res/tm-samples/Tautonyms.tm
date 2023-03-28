# INPUT:  A string in {a, b}*.
# OUTPUT: Whether the string is a tautonym (see below).
#
# Checks if a string is a tautonym. A tautonym is a string formed by writing
# the same string twice, such as "dikdik," "hotshots," or "caracara." In our
# case, we're limiting our alphabet to {a, b} just to make things easier.
#
# The hard part here is finding the middle part of the string. To do this,
# we'll repeatedly capitalize the first uncapitalized letter of the string
# and the last uncapitalized letter of the string. The point at which all
# characters are capitalized indicates the middle. From there, we decapitalize
# the second half of the string, then repeatedly match a capitalized letter
# from the first half with an uncapitalized letter from the second half.


Start:
    # Edge case
    If Blank Return True

CheckEvenLength:
    If Blank Goto Rewind
    Move Right
    If Blank Return False # Odd length
    Move Right
    Goto CheckEvenLength

Rewind:
    Move Left
    If Not Blank Goto Rewind
    Move Right

# Main loop: capitalize the first uncapitalized letter and the last
# uncapitalized letter.
FindHalfway:
    If 'A' Goto NextChar
    If 'B' Goto NextChar
    Goto Capitalize

NextChar:
    Move Right
    Goto FindHalfway

Capitalize:
    If 'a' Write 'A'
    If 'b' Write 'B'
    Goto CapitalizeLast

CapitalizeLast:
    Move Right
    If 'a' Goto CapitalizeLast
    If 'b' Goto CapitalizeLast

    # If we're here, the character we're scanning is capitalized, or we
    # hit a blank at the end of the string. Either way, back up and
    # capitalize the last character.
    Move Left
    If 'a' Write 'A'
    If 'b' Write 'B'

    # See if this was the last uncapitalized character.
    Move Left
    If 'a' Goto Rewind
    If 'b' Goto Rewind

    # We have found the halfway point. Now, decapitalize the back half of
    # the string and start matching.

Decapitalize:
    Move Right
    If Blank Goto RewindMatch
    If 'A' Write 'a'
    If 'B' Write 'b'
    Goto Decapitalize

RewindMatch:
    Move Left
    If Not Blank Goto RewindMatch
    Move Right

    # Matching step. Match this character with something from the back half
    # of the string.
    If 'A' Goto MatchA
    If 'B' Goto MatchB
    Return True # All chars matched!

MatchA:
    Write Blank
MatchALoop:
    Move Right
    If 'A' Goto MatchALoop
    If 'B' Goto MatchALoop
    If 'x' Goto MatchALoop
    If 'b' Return False # Wrong character
    Goto FoundMatch

MatchB:
    Write Blank
MatchBLoop:
    Move Right
    If 'A' Goto MatchBLoop
    If 'B' Goto MatchBLoop
    If 'x' Goto MatchBLoop
    If 'a' Return False # Wrong character
    Goto FoundMatch

FoundMatch:
    Write 'x'
    Goto RewindMatch
