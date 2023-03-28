# INPUT:  a^n.
# OUTPUT: Whether n is a Fibonacci number.
#
# Checks if the input is a Fibonacci number long.
# This works by explicitly checking for length 0
# or length 1. If those work, we then write out
# xy, then form yyx, then xxxyy, then yyyyyyxxx, etc.
# until we have an exact match or run out of characters.

Start:
  # Check for empty input or length-1 input.
  If Blank Return True
  Move Right
  If Blank Return True
  Move Left
  
  # Write xy
  Write 'x'
  Move Right
  Write 'y'
  
  # Position to meet the precondition for CoveredX.
  Move Left
  Move Left
  
# See if the next character is a blank. If so, we can stop. We should be looking
# at the first blank before the input.
CoveredX:
  Move Right
  If Blank Return True
  If Not 'a' Goto CoveredX
  Move Left
  Goto CloneXLoop
  
# Find the rightmost x that hasn't been copied. Replace it with a y
# and write x on top of the next non-y character. This begins assuming
# we are looking at the last y.
CloneXLoop:
  Move Left
  If 'y' Goto CloneXLoop
  If Blank Goto CoveredY
  Write 'y'

# Scoot over until we find a place to copy our x.
CopyOneX:
  Move Right
  If Blank Return False
  If Not 'a' Goto CopyOneX
  Write 'x'
  Goto SkipPastX
  
# Moves left until we're past all the x's
SkipPastX:
  Move Left
  If 'x' Goto SkipPastX
  Goto CloneXLoop

# Check to see if the whole input is covered, when pattern is y*x*a*
CoveredY:
  Move Right
  If Blank Return True
  If Not 'a' Goto CoveredY
  Move Left
  Goto CloneYLoop
  
# Find the rightmost y that hasn't been copied. Replace it with an x
# and write y on top of the next non-x character.
CloneYLoop:
  Move Left
  If 'x' Goto CloneYLoop
  If Blank Goto CoveredX
  Write 'x'

# Scoot over until we find a place to copy our y.
CopyOneY:
  Move Right
  If Blank Return False
  If Not 'a' Goto CopyOneY
  Write 'y'
  Goto SkipPastY
  
# Moves left until we're past all the x's
SkipPastY:
  Move Left
  If 'y' Goto SkipPastY
  Goto CloneYLoop
