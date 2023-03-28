# INPUT:  A base-10 number.
# OUTPUT: Whether that number is a Fibonacci number.
#
# Fibonacci numbers in decimal. This is formed by composing together
# the decimal-to-unary converter and the unary Fibonacci number
# programs.

# Move to far end of the number
Start:
  # Quick edge case: If the input was empty, we'll report an error.
  If Blank Return False

  # Now move to the end of the number
  Move Right
  If Not Blank Goto Start
  Move Left
  
# Decrement if possible. If zero, do a borrow
Decrement:
  # Order here matters!
  If '0' Goto Borrow
  If '1' Write '0'
  If '2' Write '1'
  If '3' Write '2'
  If '4' Write '3'
  If '5' Write '4'
  If '6' Write '5'
  If '7' Write '6'
  If '8' Write '7'
  If '9' Write '8'
  Goto Increment
  
# Adds another a to the end of the chain. Step 1: Zip to the
# end of the number, skip a space, and get to the place where
# the a's should live.
Increment:
  Move Right
  If Not Blank Goto Increment
  Goto WriteA
  
# Skip to the end of the as and write an a in the first blank.
WriteA:
  Move Right
  If 'a' Goto WriteA
  Write 'a'
  Goto Rewind

# Get back to the start of the input number
Rewind:
  Move Left
  If 'a' Goto Rewind
  
  # Hit a blank; take a step back to get back on the number.
  Move Left
  Goto Decrement
  
# We need to decrement a 0. Borrow from the previous column to do that.
Borrow:
  Write '9'
  Move Left
  
  # If we see a blank, we've dropped the number to all 0s and need to clean it up
  If Blank Goto Cleanup
  
  # Otherwise there's a number here. Decrement it.
  Goto Decrement
  
# We are at a blank in front of a chain of 9s that we wrapped expecting to find
# a nonzero number. Wipe all the 9s and step one past the blank at the end.
Cleanup:
  Move Right
  If Blank Goto Done
  Write Blank
  Goto Cleanup
  
# We are in the blank after the digits. Take one step to the right to get onto
# the As and call it a day.
Done:
  Move Right
  Goto StartFib
  
StartFib:
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



