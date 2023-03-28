# INPUT:  a^n
# OUTPUT: Whether n is a perfect square.
#
# Turing program that takes as input a string of the form a^n, then returns
# whether n is a perfect square.
#
# This program uses the fact that
#
# 1 + 3 + 5 + 7 + ... + (2n+1) = n^2.
#
# For example:
#
# 1 = 1^2
# 1 + 3 = 4 = 2^2
# 1 + 3 + 5 = 9 = 3^2
#
# The intuition behind this program is to keep adding in larger and larger
# odd numbers until we either perfectly match the number (done - it's a perfect
# square) or exceed it (not a perfect square).
#
# The specific mechanism we use is the following. At the front of the string,
# we'll write an odd number of b's. We'll then step up from one odd number to
# the next by crossing off the existing b's, writing an equivalent number of b's
# later in the string, and then add two more b's to simulate increasing from
# one odd number to the next.

Start:
  # Edge case: n = 0 is a perfect square
  If Blank Return True
  
  # Write an initial b down to simulate n = 1
  Write 'b'
  
# If the string is fully made of b's, then we're done. So scan to see if you
# can find an 'a'.
CheckIfDone:
  If Blank Return True
  If 'a' Goto GoHome
  Move Right
  Goto CheckIfDone
  
# Okay, there's at least one 'a'. Go to the start of the string so we can start
# adding in the next odd number.
GoHome:
  Move Left
  If Not Blank Goto GoHome
  Move Right
  
# We're now at the start of the string. We're looking at something of the form b^n a^m.
# We want to cross off all the b's and form a new string of b's of length n+2 at the start
# of the a's. To do this, we'll cross off each b and for each b write a 'c' over one of the
# a's.
CloneBs:
  If 'c' Goto AddTwoCs
  Write Blank
  Move Right
  
# Scan forward to the next 'a' and write a 'c' there. If we run out of a's, the string's
# length wasn't a perfect square.
FindNextA:
  If Blank Return False
  If 'a' Goto WriteOneC
  Move Right
  Goto FindNextA

# Write a single 'c' character and jump back to the main loop.
WriteOneC:
  Write 'c'
  Goto GoHome

# We've just finished converting all b's to c's. Our string now looks like c^n a^m. We need
# to convert two of those a's to c's, then convert the c's back to b's. Our first step is
# to find those a's.
AddTwoCs:
  Move Right
  If Blank Return False # Should've found some a's
  If Not 'a' Goto AddTwoCs
  
  # Found first 'a'
  Write 'c'
  Move Right
  
  If Blank Return False # Should have at least two a's
  Write 'c'
  Move Right
  
# Convert c^n a^m into b^n a^m and put the tape head at the start of the input.
ConvertCs:
  Move Left
  
  # Order here matters. Need to first turn c into b and then loop.
  If 'c' Write 'b'
  If 'b' Goto ConvertCs
  
  # Otherwise we have a blank
  Move Right
  Goto CheckIfDone
  


