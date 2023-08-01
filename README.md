# simon-2019
This repository contains code to develop a simple memory game on an Arduino Uno for an embedded systems coursework.
The game includes a mode that progressively gets more difficult. The difficulty level can be pre-selected in the game options.


## Game Instructions
1) No button to go back to previous options, RESET must be clicked to enter values again.

2) UP, DOWN, LEFT, to choose menu options.

3) SELECT to continue after choosing options.

4) UP, DOWN, LEFT, to choose starting difficulty in story mode.

5) UP and DOWN to select the size of the sequence, and the number of symbols in practice mode.

6) The direction keys represent each of the 4 custom characters in the game, which is to be pressed when choosing symbols.

7) SELECT to confirm the choice of the symbol shown on the LCD.

8) UP and DOWN to select letters for adding an alias to a new high score.


## Testing
The Serial monitor is used to observe every time the SELECT button is pressed. This is to show that the options chosen are being stored properly in their variables and that the sequence shown on the LCD and the buttons pressed are consistent.

Game progress is also shown on the Serial monitor, i.e. the start of the next stage, losing the game, and the value of some variables to help with debugging.
