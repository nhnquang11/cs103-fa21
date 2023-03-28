# Write a TM that's a decider for { a^{2^n} | n in N }

Start:
    If Blank Return False
    Move Right
    If Blank Return True
    Write 'x'
    Move Left
    Write 'x'
    Goto Double

Double:
    Goto MarkLeftA

MarkLeftA:
    Move Left
    If Blank Goto Clear
    If Not 'a' Goto MarkLeftA
    Write 'x'
    Goto MarkRightA

MarkRightA:
    Move Right
    If Blank Return False
    If Not 'a' Goto MarkRightA
    Write 'x'
    Goto Double

Clear:
    If 'x' Write 'a'
    Move Right
    If 'x' Goto Clear
    Move Left
    Goto Start
