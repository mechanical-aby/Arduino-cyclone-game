# Arduino-cyclone-game
an Arduino-based arcade game build using LED strips, push buttons, an LCD screen, a potentiometer, and an Arduino. It combines embedded control, addressable LED animation, physical user inputs, LCD feedback, and dynamic game logic.

The game is inspired by the classic "Cyclone" arcade concept. The project uses a 30-LED WS2813 strip as a circular track. A moving light travels around the track while the player attempts to press a button when the moving light reaches a goal. This project expands on the basic idea with multiple game modes: **Normal**, **Trick**, **Brutal**, and **Pong**. Each new mode introduces additional unpredictability and complex game mechanics.

The system also includes an LCD menu interface and potentiometer-based menu selection. The potentiometer was used as an alternative input method after the available push buttons were exhausted during prototyping. 

**Engineering Highlights**
Integrated an Arduino with a WS2813 addressable LED strip, LCD, push buttons, indicator LEDs, and potentiometer.

- Developed position and speed control around a circular 30-LED track, including timed updates and adjustable movement speed.
- Implemented control logic for menu navigation, gameplay, player input, goal detection, scoring, and game termination.
- Implemented progressive difficulty by adjusting the speed and unpredictability of the moving light based on game mode, game progression, and player actions.
- Organized gameplay, input handling, scoring, and menu navigation into separate functions to keep the program modular and maintainable.

**Hardware**
- Arduino
- 30-LED WS2813 strip
- 16 x 2 LCD
- 1 x Potentiometer
- 2 x Push buttons
- 3 x indicator LEDs
  - 1 x Green LED
  - 2 x Red LEDs

**Game Modes**
The game modes included in this project are:

**Normal Mode**
The moving light moves continuously around the circular LED track in one direction. Three difficulty levels available are:
- Beginner
- Medium
- Fast

Each difficulty level increases the moving light's speed. 

In two-player mode, the speed gradually increases further as the game progresses towards the lap limit.

**Trick Mode**
Trick Mode introduces unpredictable behaviour to make it difficult to press the button while the moving light is in the goal. In this mode, the moving light can:

- Reverse direction when near the goal
- Stop temporarily near the goal
  - After the pause, it can choose to continue forward or reverse.

**Brutal Mode**
Brutal Mode is restricted to one player. It builds on Trick Mode by adding even more unpredictable behaviours. The moving light can:

- Temporarily increase its movement speed.
- Change directions multiple times.
- If the player presses the button too early, it chooses the direction that brings it to the goal quicker.

**Pong Mode**
Pong Mode is inspired by table tennis. It turns the circular track into a two-player game. If a player accurately presses their button, the moving light changes its direction, increases its speed, and moves towards the other player's goal. 

Once a player fails to hit the moving light, the opposing player receives points and the moving light's speed is reset.

**Additional Lighting Mode**
The LED strip can also be used independently of the game. The potentiometer controls the LED hue, allowing the strip to function as a single-color light source.
