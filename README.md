# Sorry_game
The project I have done for my Robotics class

# The game
The game will ask the user for the number of players.
Each player will type their name on the keypad. 
The game starts after all players are done submitting their names.

# Turn
Each turn is composed of 2 actions. 
** Rolling the dice (1 - 12)
The user simply presses the button to roll.

** Selecting the pawn
The user uses the joystick to select the pawn they would like to move 
and may move the joystick up to display the current state of the board.

** Checks
The game will check if the user has landed on another player's pawn's space. If it does, 
the game will report it on the LCD and move the bumped pawn back home. The bumped pawn needs to start 
it journey again.

The game will also check if a player has won. If all pawns have traveled once around the board and landed back home,
the game will report that the player has won. If one would like to play again, they would need to reset the arduino.


