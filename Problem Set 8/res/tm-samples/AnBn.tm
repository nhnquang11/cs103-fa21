# INPUT:  A string in {a, b}*.
# OUTPUT: Whether the string is of the form a^n b^n.

Start:
  If Blank Return True
  If 'b' Return False
  Write Blank

ZipRight:
  Move Right
  If Not Blank Goto ZipRight
  Move Left

CrossOff:
  If Not 'b' Return False
  Write Blank

ZipLeft:
  Move Left
  If Not Blank Goto ZipLeft
  Move Right
  Goto Start
