# Write a TM for { w in {a, b}* | w has odd length and its middle character is a }

Start:
    Move Right
    If Not Blank Goto BlankTwoEnds  # There are more than 1 characters in the tape.
    Move Left
    If 'a' Return True
    Return False

BlankTwoEnds:
    # Blank the left end.
    Move Left
    Write Blank

    # Blank the right end
    Goto BlankRight

BlankRight:
    Move Right
    If Not Blank Goto BlankRight
    Move Left
    Write Blank
    Goto ZipLeft

ZipLeft:
    Move Left
    If Not Blank Goto ZipLeft
    Move Right
    Goto Start
