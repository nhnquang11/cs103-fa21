# INPUT:  A string in {a, b}*.
# OUTPUT: Whether there are two b's separated by exactly
#         five characters.
#
# Check if a string has two b's with exactly five characters between them.
# Unlike finite automata, which have to do this nondeterministically, this
# is pretty easy for a TM - we can manually walk forwards and backwards over
# the string to see what we find!

Start:
    If Blank Return False # Oops, no more characters
    If 'b' Goto CheckSeparation
    Move Right
    Goto Start

CheckSeparation:
    Move Right # Step off the b
    Move Right # Skip five characters
    Move Right
    Move Right
    Move Right
    Move Right

    If 'b' Return True

    # Oh fiddlesticks. That didn't work. Back up to just after
    # the b we stepped off of.

    Move Left
    Move Left
    Move Left
    Move Left
    Move Left
    Goto Start
