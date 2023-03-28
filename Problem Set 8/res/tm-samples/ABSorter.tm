# INPUT:  A string in {a, b}*.
# OUTPUT: Returns true. The tape ends with the characters sorted
#         such that all a's precede all b's.

Start:
  If Blank Return True
  If 'b' Goto FoundB
  Move Right
  Goto Start

FoundB:
  If Blank Return True
  If 'a' Goto FoundAB
  Move Right
  Goto FoundB

FoundAB:
  Write 'b'
  Move Left
  Write 'a'

GoHome:
  Move Left
  If Not Blank Goto GoHome
  Move Right
  Goto Start
