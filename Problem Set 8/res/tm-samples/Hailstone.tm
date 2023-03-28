# INPUT:  a^n.
# OUTPUT: Returns true if the hailstone sequence terminates for n.

# Unary hailstone sequence

Start:
  # Check for 0 as an edge case
  If Blank Return False
  
MainLoop:
  # Check for 1 to see if we're done.
  Move Right
  If Blank Return True
  Move Left
  
# Parity checker. If even length, need to cut in half. If odd length, need to triple and add one.
ParityCheck:
  If Blank Goto Even
  Move Right
  If Blank Goto Odd
  Move Right
  Goto ParityCheck

# Cut the length in half. We begin in the blank at the end, so we need to first zip to the front of the
# sequence of numbers.
Even:
  Move Left
  If Not Blank Goto Even
  Move Right

# Cross off two characters, zip to the blank at the end, skip it, zip to the end of that, and write one
# character. Then return home.
EvenCrossOff:
  If Blank Goto EvenDone
  Write Blank
  Move Right
  Write Blank

# Zip to the blank at the end of the block of remaining characters.
EvenZipToEnd:
  Move Right
  If Not Blank Goto EvenZipToEnd


# Zip to the blank at the end of the block of reconstructed numbers
EvenZipToEnd2:
  Move Right
  If Not Blank Goto EvenZipToEnd2
  Write 'a'
  
# Zip to the start of the reconstructed numbers
EvenZipToStart2:
  Move Left
  If Not Blank Goto EvenZipToStart2

# Zip to the start of the original number
EvenZipToStart:
  Move Left
  If Not Blank Goto EvenZipToStart
  
  # Just stepped off. Go back and resume the loop.
  Move Right
  Goto EvenCrossOff
  
EvenDone:
  Move Right
  Goto MainLoop
  
###############

# Triple the length and add 1. Begin by zipping back to the start.
Odd:
  Move Left
  If Not Blank Goto Odd
  Move Right

# Cross off one character, zip to the blank at the end, skip it, zip to the end of that, and write three
# character. Then return home.
OddCrossOff:
  If Blank Goto OddDone
  Write Blank

# Zip to the blank at the end of the block of remaining characters.
OddZipToEnd:
  Move Right
  If Not Blank Goto OddZipToEnd

# Zip to the blank at the end of the block of reconstructed numbers
OddZipToEnd2:
  Move Right
  If Not Blank Goto OddZipToEnd2
  Write 'a'
  Move Right
  Write 'a'
  Move Right
  Write 'a'
  Move Right
  
# Zip to the start of the reconstructed numbers
OddZipToStart2:
  Move Left
  If Not Blank Goto OddZipToStart2

# Zip to the start of the original number
OddZipToStart:
  Move Left
  If Not Blank Goto OddZipToStart
  
  # Just stepped off. Go back and resume the loop.
  Move Right
  Goto OddCrossOff
  
OddDone:
  Write 'a'
  Goto MainLoop
