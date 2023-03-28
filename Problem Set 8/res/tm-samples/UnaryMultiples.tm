# INPUT:  A string of the form a^n b^m.
# OUTPUT: Whether m is a multiple of n.
#
# Given a^nb^m, checks if m is a multiple of n. The procedure works like this:
#
# If n = 0, then check if m = 0.
# Otherwise:
#   If m = 0, Return True.
#   If not, set m := m - n and repeat.
#
# The specific procedure for doing this is to pair off the a's with the b's
# to simulate the decrement.
Start:
  # Confirm we're looking at something of the form a*b*, just to rule out weird cases.
  If Blank Goto Rewind
  If 'a' Goto AStar
  If 'b' Goto ABStar
  Return False # Something fishy is on the tape

AStar:
  Move Right
  If 'a' Goto AStar
  If 'b' Goto ABStar
  If Blank Goto Rewind

ABStar:
  Move Right
  If 'a' Return False # Bad input: should be a^n b^m
  If 'b' Goto ABStar
  If Blank Goto Rewind
  
Rewind:
  Move Left
  If Not Blank Goto Rewind
  Move Right

Main:
  If Blank Return True # n = 0, m = 0
  If 'b' Return False   # n = 0, m != 0
  
  Goto MoveToBs
  
# Get from the As to the Bs to see if there are no Bs left.
# As we go, flip any x's to a's.
MoveToBs:
  Move Right
  If 'a' Goto MoveToBs
  If Blank Return True # Crossed off all b's
  If 'b' Goto NextA
  Write 'a'
  Goto MoveToBs
  
# Cross off the next A
NextA:
  Move Left
  If Blank Goto RestoreAs
  If 'x' Goto NextA
  If 'b' Goto NextA
  
  # Should be an a here.  
  Write 'x'
  Goto FindLastB
  
# Find the last B so we an cross it off
FindLastB:
  Move Right
  If Not Blank Goto FindLastB
  Move Left
  If Not 'b' Return False # Oops, ran out of things
  Write Blank
  Goto NextA
  
# At the blanks before the As, which are now all flipped
# to x's. Flip all x's to a's.
RestoreAs:
  Move Right
  If 'x' Write 'a'
  If Blank Return True
  If 'b' Goto NextA
  Goto RestoreAs


