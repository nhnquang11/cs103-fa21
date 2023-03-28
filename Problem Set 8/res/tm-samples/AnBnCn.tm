# INPUT:  A string in {a, b, c}*.
# OUTPUT: Whether the string has the form a^n b^n c^n
#
# TM for the language {a^n b^n c^n | n in N }. This is a canonical non-context-free
# language, though we haven't covered this quarter how to prove this. (Do a Google
# search for "pumping lemma for context-free languages" for more about this.)
#
# The basic strategy here is to confirm that the string has the form a*b*c*, then
# repeatedly cross off one a, one b, and one c until we've munched all the characters.

Start:
    If Blank Goto Rewind
    If 'b' Goto AB
    If 'c' Goto ABC
    Move Right
    Goto Start

AB:
    If Blank Goto Rewind
    If 'c' Goto ABC
    If 'a' Return False
    Move Right
    Goto AB

ABC:
    If Blank Goto Rewind
    If 'a' Return False
    If 'b' Return False
    Move Right
    Goto ABC

Rewind:
    Move Left
    If Not Blank Goto Rewind

FindNextA:
    Move Right
    If 'x' Goto FindNextA
    If Blank Return True
    If Not 'a' Return False
    Write 'x'

FindNextB:
    Move Right
    If 'x' Goto FindNextB
    If 'a' Goto FindNextB
    If Not 'b' Return False
    Write 'x'

FindNextC:
    Move Right
    If 'x' Goto FindNextC
    If 'b' Goto FindNextC
    If Not 'c' Return False
    Write 'x'
    Goto Rewind
