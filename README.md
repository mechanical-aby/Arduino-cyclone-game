# Arduino-cyclone-game
an Arduino-based arcade game build using LED strips, push buttons, an LCD screen, a potentiometer, and an Arduino microcontroller.

The game is inspired by the classic "Cyclone" arcade concept. A moving light travels around a loop of LEDs, and the player must press a button when the light reaches the goal. This project expands on the basic idea with multiple game modes, including Normal, Trick, Brutal, and Pong mode.

**Hardware**
- Arduino
- 30 x WS2813 addressable LED strips
- 16 x 2 LCD display
- 1 x Potentiometer
- 2 x Push buttons
- 3 x indicator LEDs
  - 1 x Green LED
  - 2 x Red LEDs
 
**Embedded Control**
The Arduino continuously manages:
- LED position and animation
- Player button inputs
- Game-state transitions
- Game timing
- Target velocity and direction changes
- Player scores
- Lap progression

The game was designed around different states and behaviours rather than simply moving the LED at a constant speed.

**Engineering Skills Demonstrated**
- Arduino / embedded programming
- C/C++ programming
- Digital/ analog input handling
- LCD interfacing
- Human-machine interface design

**Game modes**
**Normal Mode**
The target moves continuously around the circular LED track in one direction. The three difficulty levels available are beginner, medium, and fast, with each subsequent one increasing the speed. In two-player mode, the speed gradually increases further as the game progresses towards the lap limit.

**Trick Mode**
Trick Mode introduces unpredictable behaviour to make the target harder to time. In this mode, the target can:

- Reverse direction when near the goal
- Stop temporarily near the goal
  - After the pause, it can choose to continue forward or reverse.


