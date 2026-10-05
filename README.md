# Arduino-cyclone-game
an Arduino-based arcade game build using LED strips, push buttons, an LCD screen, a potentiometer, and an Arduino microcontroller. It combines embedded control, addressable LED animation, physical user inputs, LCD feedback, and dynamic game logic.

The game is inspired by the classic "Cyclone" arcade concept. The project uses a 30-LED WS2813 strip as a circular track. A moving light travels around the track while the player attempts to press a button when the target reaches a goal. This project expands on the basic idea with multiple game modes, namely **Normal**, **Trick**, **Brutal**, and **Pong** mode. Each new game mode includes more unpredictability and complex game mechanics.

The system also includes an LCD menu interface and potentiometer-based menu selection. The potentiometer was used as an alternative input method after the available push buttons were exhausted during prototyping. 

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
The target moves continuously around the circular LED track in one direction. The three difficulty levels available are beginner, medium, and fast, with each subsequent one increasing the speed. 

In two-player mode, the speed gradually increases further as the game progresses towards the lap limit.

**Trick Mode**
Trick Mode introduces unpredictable behaviour to make the target harder to time. In this mode, the target can:

- Reverse direction when near the goal
- Stop temporarily near the goal
  - After the pause, it can choose to continue forward or reverse.

**Brutal Mode**
Brutal Mode is restricted to one player. It builds on Trick Mode by adding even more unpredictable behaviours. The target can:

- Temporarily increase its movement speed.
- Change directions multiple times.
- Deliberately move towards the goal to punish a player's mistake.

**Pong Mode**
Pong mode is inspired by the table tennis sport. It turns the circular track into a two-player game. A successful button press when the moving light reaches a player's goal reverses its direction, increases its speed, and sends it towards the other player's goal. 

Once a player fails to hit the moving light, the opposing player receives points and the target speed is reset.

