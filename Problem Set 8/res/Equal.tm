# Write a TM that's a decider for { w=w | w in {a, b}* }

Start:
    If Blank Return False
    If '=' Goto Check
    If 'a' Goto FoundA
    If 'b' Goto FoundB

Check:
    Move Right
    If Blank Return True
    If Not 'x' Return False
    Goto Check

FoundA:
    Write Blank
    Goto ZipRightMidA

ZipRightMidA:
    Move Right
    If Blank Return False
    If Not '=' Goto ZipRightMidA
    Goto MarkA

MarkA:
    Move Right
    If 'x' Goto MarkA
    If Not 'a' Return False
    Write 'x'
    Goto ZipLeft

FoundB:
    Write Blank
    Goto ZipRightMidB

ZipRightMidB:
    Move Right
    If Blank Return False
    If Not '=' Goto ZipRightMidB
    Goto MarkB

MarkB:
    Move Right
    If 'x' Goto MarkB
    If Not 'b' Return False
    Write 'x'
    Goto ZipLeft

ZipLeft:
    Move ZipLeft
    If Not Blank Goto ZipLeft
    Move Right
    Goto Start
