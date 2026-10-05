#include <FastLED.h>
#include <LiquidCrystal.h>

#define LED_STRIP_PIN 2
#define BUTTON_PIN_1 9//3
//#define BUTTON_PIN_1 3
#define LED_PIN_GREEN 10
#define LED_PIN_RED 11
#define BUTTON_PIN_2 12//8
#define LED_PIN_RED_2 13
#define POTENT_MENU_SELECT A0

//LCD Pins
int LCD_RS = 3;//4;
int LCD_Enable = 4;//5;
int LCD_D4 = 5;//7;
int LCD_D5 = 6;//9;
int LCD_D6 = 7;//11;
int LCD_D7 = 8;//13;
LiquidCrystal lcd(LCD_RS, LCD_Enable, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

#define NUM_LEDS    30
#define BRIGHTNESS  64
#define LED_TYPE    WS2813
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define UPDATES_PER_SECOND 100

//The total number of LEDs the LED strip has.
//This will be used as the circular track that the moving light travels.
int LedCount = 30;
//Represents which LED represents the position of the moving light
int targetPos = 0;
//How fast the moving light is travelling (Up to a max of 1000)
float targetSpeed = 25;
//The speed of the moving light is increased while this value is greater than 0
//It represents how many LEDs the moving light must travel through before its speed is reset
int increasedSpeedDuration = 0;

//Represents whether the moving light is moving clockwise or counter-clockwise
int direction = 1;
//If this is true, the moving light will not try to punish the player when they make a mistake.
//It is set to false the next time the moving light crosses the goal.
//This variable and behaviour is exclusive to brutal mode.
bool punishPlayer = false;

//Represents which LEDs represent the positions of the goals of player 1 and player 2.
//To make the game easier, the 2 leds behind and ahead of this one are also included as the player's goal.
int goalPos[] = {22,6};
//The score of each player
int scores[] = {0,0};
//How many players are currently playing a game mode
int playerCount = 1;

//If any game mode has been selected
bool gameStarted = false;
//If it is currently trick mode
bool trickMode = false;
//If it is currently brutal mode
bool brutalMode = false;
//If it is currently pong mode
bool pongMode = false;
//How many laps the moving light has currently travelled
float curLaps = 0;

//Whether it is in the main menu, game select menu, or light select menu
int currentMenuLevel = 0;
//Which option was chosen in the main menu
int chosenMainMenu = 0;
//Which option was chosen in the game menu
int chosenGameOption = 0;
//Which option was chosen in the light adjust menu
int chosenLightOption = 0;
//String lightOptions[] = {"1. adjust light"} "Use potentiometer to adjust light. click button to return"
//The starting game speed for each game mode
//{Normal - beg, normal - med, normal - fast, trick, brutal, pong}
float gameModeSpeeds[] = {15, 20, 25, 30, 30, 20};//30, 30};
//Represents the color choice selected in the light adjust menu
int savedHue = 0;

//If this is 0, the moving light will trigger another change in direction.
//This direction change will be towards the goal.
//This is exclusive to the brutal mode.
int switcherooCooldown = 0;
//If this is false, it might choose to change its direction near the goal
bool switched = false;
//If the respective player has pressed their button
bool buttonPressed[] = {false,false};
//If the target has entered the goal of the respective player
bool hasEnteredGoal[] = {false,false};
//How much the score increases on a successful or failed button press
//This value changes depending on the chosen game mode
int scoreIncrem = 10;
//This represents how much the speed of the moving light increases.
//It is exclusive to the pong mode.
int pongSpeedIncrem = 1;
//While this is greater than 0. The moving light moves slower.
//This represents how many LEDs it will move through before its speed is restored.
//It activates after the target starts moving after a fakeout.
int stepsSlowedAfterStopping = 0;

void setup() {
  // put your setup code here, to run once:
	Serial.begin(57600);
	//Serial.println("resetting");
	FastLED.addLeds<WS2813,LED_STRIP_PIN,GRB >(leds,NUM_LEDS);
	FastLED.setBrightness(84);
  turnOnMainMenuLights();

  pinMode(BUTTON_PIN_1, INPUT);
  pinMode(BUTTON_PIN_2, INPUT);
  pinMode(LED_PIN_GREEN, OUTPUT);
  pinMode(LED_PIN_RED, OUTPUT);
  pinMode(LED_PIN_RED_2, OUTPUT);
  digitalWrite(LED_PIN_GREEN, LOW);
  digitalWrite(LED_PIN_RED, LOW);
  digitalWrite(LED_PIN_RED_2, LOW);
  
  lcd.begin(16, 2);
  // Print a message to the LCD.

  //createGoals();
  //playerCount = 2;
  //prepareGame();
  //gameStarted = true;
}

int buttonState[] = {0,0};
int buttonCooldown[] = {0,0};
int ledShineDuration = 0;
void loop() {
  //Serial.println(String(buttonState[0]) + " " + String(buttonState[1]));
  //digitalWrite(LED_PIN_GREEN, HIGH);
  // put your main code here, to run repeatedly:
  if(gameStarted)
  {
    createGoals();
    if (targetPos == goalPos[0])
      curLaps += 1;
    if (trickMode)
      chooseATrick();
    moveTarget();
    int delayIndex = ((1000/targetSpeed) * (1 + (0.5 * int(stepsSlowedAfterStopping > 0))))/ (1 + (1.5 * int(increasedSpeedDuration > 0)));
    if (trickMode) Serial.println("Delay Index: " + String(delayIndex));
    updateComponentCooldowns(delayIndex);
    //Serial.println("Cool: " + String(buttonCooldown));
    //Serial.println(buttonState);
    while (delayIndex > 0)
    {
      buttonState[0] = digitalRead(BUTTON_PIN_1);
      buttonState[1] = digitalRead(BUTTON_PIN_2);
      checkTargetEnteredAnyGoal();
      displayScores();
      digitalWrite(LED_PIN_RED, (buttonCooldown[0] > 0));
      digitalWrite(LED_PIN_RED_2, (buttonCooldown[1] > 0));
      delayIndex -= 10;
      delay(10);//delayIndex);
    }
    gameStarted = getGameOverCondition(); //(scores[0] <= -50 || scores[0] >= 100);
    if (!gameStarted)
    {
      //display winner or tie (with 2 players) or you win/lose (with 1 player)
      displayWinner();
      delay(5000);
      currentMenuLevel = 0;
      turnOnMainMenuLights();
    }
  }
  if (!gameStarted)
  {
    buttonState[0] = digitalRead(BUTTON_PIN_1);
    buttonState[1] = digitalRead(BUTTON_PIN_2);
    displayMenu();
    int delayIndex = 40;
    updateComponentCooldowns(delayIndex);
    delay(delayIndex);
  }    
}

bool getGameOverCondition()
{
  if (!pongMode)
    return !(curLaps >= 30 && !checkTargetIsInAGoal());
  else
    return !((scores[0] >= 40 || scores[1] >= 40) && !checkTargetIsInAGoal());
}

void updateComponentCooldowns(int delayIndex)
{
    if (increasedSpeedDuration > 0) increasedSpeedDuration--;
    if (ledShineDuration > 0)
      ledShineDuration -= delayIndex;
    else if (ledShineDuration <= 0 && ledShineDuration > -9999)
    {
      ledShineDuration = -9999;
      digitalWrite(LED_PIN_GREEN, LOW);
    }
    if (buttonCooldown[0] >= 0)
      buttonCooldown[0] -= delayIndex;
    if (buttonCooldown[0] <= 0)
      digitalWrite(LED_PIN_RED, LOW);
    if (buttonCooldown[1] >= 0)
      buttonCooldown[1] -= delayIndex;
}

void displayWinner()
{
  if (playerCount == 1)
  {
    lcd.setCursor(6, 1);  
    if (scores[0] > 0)
      lcd.print("Win");
    else if (scores[0] <= 0)
      lcd.print("Lose");
  }
  else if (playerCount == 2)
  {
    lcd.setCursor(4, 1);  
    if (scores[0] > scores[1])
      lcd.print("P1 wins");
    else if (scores[1] > scores[0])
      lcd.print("P2 wins");
    else if (scores[0] == scores[1])
    {
      lcd.setCursor(7, 1);
      lcd.print("Tie");
    }
  }
}

String menuChoices[] = {"1P Cyc. game", "2P Cyc. game", "Normal Light", "Decor. Light"};
String gameModes[] = {"Norm - Begin.", "Norm - Med.", "Norm - Fast", "Trick mode", "Brutal mode", "Pong mode", "Back"};
//bool shouldResetLEDS = false;
void displayMenu()
{
  lcd.clear();
  //Serial.println("Chose " + String(chosenMainMenu));
  if (currentMenuLevel == 0)
  {
    //pinCount = ;
    /*if (shouldResetLEDS)
    {
      shouldResetLEDS = false;
      for (int i = 0; i < LedCount; i++)
      {
        leds[i] = CHSV(0, 255, 255);
        FastLED.show();
      }
    }*/
    int menuSize = sizeof (menuChoices) / sizeof (menuChoices[0]);
    //if (!(buttonState[0] == 1 && buttonCooldown[0] <= 0))
      chosenMainMenu = (analogRead(POTENT_MENU_SELECT) * menuSize) / 1023;
    if (chosenMainMenu > menuSize - 1)
      chosenMainMenu = menuSize - 1;
    //Serial.println(String(chosenMainMenu) + " " + String(analogRead(POTENT_MENU_SELECT)) + " " + String((analogRead(POTENT_MENU_SELECT) * sizeof(menuChoices)) / 950) + " " + String(sizeof(menuChoices)));
    lcd.setCursor(0, 0);  
    lcd.print(">" + menuChoices[chosenMainMenu]);
    lcd.setCursor(0, 1);  
    if (chosenMainMenu + 1 > menuSize - 1)
      lcd.print("");//menuChoices[0]);
    else
      lcd.print(" " + menuChoices[chosenMainMenu + 1]);

    if (buttonState[0] == 1 && buttonCooldown[0] <= 0 && chosenMainMenu < 3)
    {
      //Serial.println("YYY");
      buttonCooldown[0] = 500;
      currentMenuLevel = 1;
      //shouldResetLEDS = true;
      if (chosenMainMenu < 2)
        playerCount = chosenMainMenu + 1;
    }
  }

  else if (currentMenuLevel == 1)
  {
    //Serial.println("X " + String(chosenGameOption));
    if (chosenMainMenu == 0 || chosenMainMenu == 1)
    {
      int menuSize = sizeof (gameModes) / sizeof (gameModes[0]);
      chosenGameOption = (analogRead(POTENT_MENU_SELECT) * menuSize) / 1023;
      if (chosenGameOption > menuSize - 1)
        chosenGameOption = menuSize - 1;

      lcd.setCursor(0, 0);  
      lcd.print(">" + gameModes[chosenGameOption]);
      lcd.setCursor(0, 1);  
      //Serial.println("Foo " + String(chosenGameOption) + " " + String(chosenGameOption + 1));
      if (chosenGameOption + 1 > menuSize - 1)
        lcd.print(" ");//gameModes[0]);
      else
        lcd.print(" " + gameModes[chosenGameOption + 1]);

      if (buttonState[0] == 1 && buttonCooldown[0] <= 0)
      {
        buttonCooldown[0] = 200;
        if (chosenGameOption < menuSize - 1)
          prepareGame();
        else
        {          
          turnOnMainMenuLights();
          currentMenuLevel = 0;
        }
      }

      if (buttonState[1] == 1)
      {          
        turnOnMainMenuLights();
        currentMenuLevel = 0;
      }
    }

    else if (chosenMainMenu == 2)
    {
      //Serial.println("2");
      lcd.setCursor(0, 0);  
      lcd.print("Dial. to change");
      lcd.setCursor(0, 1);  
      lcd.print("Button to return");
      for (int i = 0; i < LedCount; i++)
      {
        savedHue = (analogRead(POTENT_MENU_SELECT) * 255.0) / 1023.0;
        turnOnMainMenuLights(i);
        if (digitalRead(BUTTON_PIN_1) == 1 && buttonCooldown[0] <= 0 || buttonState[1] == 1) 
        {
          //Serial.print("x");
          buttonCooldown[0] = 500;
          currentMenuLevel = 0;
          /*for (int i = 0; i < LedCount; i++)
            leds[i] = CHSV(savedHue, 255, 255);
          FastLED.show();*/
          break;
        }
        //Serial.println(digitalRead(BUTTON_PIN_1) + " ");
        FastLED.show();
      }
    }
  }
}

void prepareGame()
{
  int menuSize = sizeof (gameModes) / sizeof (gameModes[0]);
  for (int i = 0; i < LedCount; i++)
    leds[i] = CRGB::Black;
  targetSpeed = gameModeSpeeds[chosenGameOption];
  trickMode = (chosenGameOption == menuSize - 3 || chosenGameOption == menuSize - 4);
  brutalMode = (chosenGameOption == menuSize - 3);
  pongMode = (chosenGameOption == menuSize - 2);
  //Serial.println(String(menuSize) + " " + String(chosenGameOption) + " " + String(chosenGameOption == menuSize - 2) + " " + String(pongMode));
  if (brutalMode) 
    playerCount = 1;//int(brutalMode);
  else if (pongMode)
    playerCount = 2;
  scores[0] = scores[1] = 0;
  curLaps = 0;
  if (playerCount == 1)
    targetPos = random(0, 30);
  else 
    targetPos = 15;//goalPos[0] - 2 - random(0,6);
  if (targetPos < 0)
    targetPos += LedCount;
  else if (targetPos >= LedCount)
    targetPos -= LedCount;
  if (checkTargetIsInGoal(1) || playerCount == 2 && checkTargetIsInGoal(2))
    targetPos += 5;
  createGoals();
  FastLED.show();
  int count = 3 + 1;
  while (--count >= 0)
  {
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Game starts in");
    lcd.setCursor(8,1);
    lcd.print(String(count));
    delay(1000);
  }
  lcd.clear();
  gameStarted = true;
}

void turnOnMainMenuLights()
{
  for (int i = 0; i < LedCount; i++)
    turnOnMainMenuLights(i);
}

void turnOnMainMenuLights(int ledIndex)
{
  /*long test = (analogRead(POTENT_MENU_SELECT) * 255) / 1023;
  Serial.println(String(savedHue) + " " + String(analogRead(POTENT_MENU_SELECT)) + " " + String((analogRead(POTENT_MENU_SELECT) / 1023.0) * 255.0) + " " + test);
  if (savedHue > 255) savedHue = 255;
  */if (savedHue >= 220)
    leds[ledIndex] = CRGB::White;
  else
    leds[ledIndex] = CHSV(savedHue, 255, 255);
  FastLED.show();
}

void displayScores()
{
  lcd.clear();
  lcd.setCursor(0, 0);  
  lcd.print("P1");  
  lcd.setCursor(6, 0); 
  String text = "{";
  if (curLaps < 10)
    text += "0";
  if (curLaps < 100)
    text += "0";
  text += String(int(curLaps)) + "}";
  lcd.print(text);//"|");
  lcd.setCursor(14, 0);  
  lcd.print("P2");  

  lcd.setCursor(0, 1);
  lcd.print(scores[0]);  
  lcd.setCursor(8, 1); 
  lcd.print("|");
  int scorePos = 15;
  if (scores[1] < 0)
    scorePos -= 1;
  if (abs(scores[1]) >= 10)
    scorePos -= 1;
  if (abs(scores[1]) >= 100)
    scorePos -= 1;
  lcd.setCursor(scorePos, 1);
  if (playerCount == 2)  
    lcd.print(scores[1]); 
  else
    lcd.print("N"); 


  /*lcd.print("Hello, Worldies!");
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis() / 1000);*/
}

void createGoals()
{  
  for (int i = 0; i < LedCount; i++)
    //if (i != targetPos)
      leds[i] = CRGB::Purple;

  leds[goalPos[0] - 1] = CHSV(0, 255, 255);
  leds[goalPos[0]] = CHSV(0, 255, 255);
  leds[goalPos[0] + 1] = CHSV(0, 255, 255);

  if (playerCount == 2)
  {
    leds[goalPos[1] - 1] = CHSV(0, 255, 255);
    leds[goalPos[1]] = CHSV(0, 255, 255);
    leds[goalPos[1] + 1] = CHSV(0, 255, 255);
  }
  else
  {
    leds[goalPos[1] - 1] = CRGB::Purple;
    leds[goalPos[1]] = CRGB::Purple;
    leds[goalPos[1] + 1] = CRGB::Purple;
  }
FastLED.show();
}

bool directionConfigured = false;
void moveTarget()
{
  //Serial.println(String(targetPos) + " " + String(checkTargetIsInAGoal()));
  /*if (!checkTargetIsInAGoal())
    leds[targetPos] = CRGB::Black;
  else
    createGoals();*/
  if (stepsSlowedAfterStopping > 0) stepsSlowedAfterStopping -= 1;

  if (brutalMode && punishPlayer && !directionConfigured)
  {
    directionConfigured = true;
    int shiftedIndex = 14 - goalPos[0];
    int shiftedTarget = targetPos + shiftedIndex;
    //int changeIndex = //(15 + goalPos[0]) - targetPos);
    if (shiftedIndex != 0)
      direction = (14 - shiftedTarget) / abs((14 - shiftedTarget));
    //if (direction == 0) direction = 1;
    //Serial.println("Direction " + String(direction) + " Goal pos " + String(goalPos[0]) + " Target Position " + String(targetPos) + " shift_Index " + String(shiftedIndex) + " Target_Shift " + String(shiftedTarget));
  }
  targetPos += direction;
  //Serial.println("targetPosition-> " + String(targetPos));
  if (targetPos >= LedCount)
    targetPos -= LedCount;
  else if (targetPos < 0)
    targetPos += LedCount;
  //Serial.println("Test");

  /*for(int i = 0; i < 3; i++) 
  { 
    int colInd = targetPos - (direction * (i + 1));
    if (colInd >= LedCount)
      colInd -= LedCount;
    else if (colInd < 0)
      colInd += LedCount;

    if (!((colInd == goalPos[0] - 1 || colInd == goalPos[0] || colInd == goalPos[0] + 1) || (playerCount == 2 && (colInd == goalPos[1] - 1 || colInd == goalPos[1] ||colInd == goalPos[1] + 1))))
    {
      leds[colInd] = CHSV(126, 255, 255);
    //}
    //if (!((colInd == (goalPos[0] - 1) || colInd == goalPos[0] || colInd == (goalPos[0] + 1))|| playerCount == 2 && (colInd == (goalPos[1] - 1) || colInd == goalPos[1] || colInd == (goalPos[1] + 1))))
      leds[colInd].nscale8(20);//256 / (1 + 0.5 * (255 * i)));//256 / 2);//(i + 2));
    }
    /*else if (checkTargetIsInGoal(1) || playerCount == 2 && checkTargetIsInGoal(2))
      createGoals();
  }*/
  /*blur1d(leds, NUM_LEDS, 172);
  fadeToBlackBy(leds, NUM_LEDS, 16);*/
  leds[targetPos] = CHSV(126, 255, 255);
	FastLED.show();
}

void resetPong()
{
  int dir = direction;
  direction = 0;
  //buttonCooldown[0] = 9999;
  //buttonCooldown[1] = 9999;
  delay(1000);
  //buttonCooldown[0] = buttonCooldown[1] = 0;
  targetPos = 15 + random(3) - 1;//(random(((goalPos[0] + 3) + (goalPos[1] + 3)) / 2);
  int lis[] = {-1,1};
  direction = lis[random(2)] * dir;
}
void checkTargetEntersGoal(int playerNum)
{
  int cooldown = 300 * (gameModeSpeeds[chosenGameOption] / targetSpeed);;
  int index = playerNum - 1;
  if (buttonCooldown[0] <= 0)
  {
    punishPlayer = false;
    directionConfigured = false;
  }
  //When the target just enters the goal and it has not yet been recorded
  if (checkTargetIsInGoal(playerNum))
  {
    //Serial.println("0 POSIt: " + String(targetPos) + " " + String(playerNum));
    if (!hasEnteredGoal[index])
    {
      hasEnteredGoal[index] = true;
      punishPlayer = false;
      directionConfigured = false;
      switched = false;
      //Serial.println("Speed: " + String(targetSpeed) + " Cond. " + String(playerNum == 1 && playerCount == 2) + " Inc. " + String(1 + ((curLaps * 2) / 30)));
      if (!pongMode && playerNum == 1 && playerCount == 2)
        targetSpeed = gameModeSpeeds[chosenGameOption] * (1 + ((curLaps * 1.5) / 30));
    }
    if (buttonCooldown[index] <= 0 && buttonState[index]/*digitalRead(BUTTON_PIN_1)*/ && !buttonPressed[index])
    {
      //Serial.println("1");
      buttonPressed[index] = true;
      buttonCooldown[index] = cooldown;
      digitalWrite(LED_PIN_GREEN, HIGH);
      ledShineDuration = 400 * (gameModeSpeeds[chosenGameOption] / targetSpeed);
      if (!pongMode)
        scores[index] += scoreIncrem;
      else if (pongMode)
      {
        direction *= -1;
        targetSpeed += pongSpeedIncrem;
      }

      /*String text = "Player 1 score: " + String(scores[0]);
      if (playerCount == 2)
        text = " | Player 2 score: " + String(scores[1]);
      Serial.println(text);*/
    }
    //Serial.println("Entered");
  }

  else// if(!checkTargetIsInGoal(playerNum))
  {
    //Serial.println("Score P1: " + String(scores[0]) + " Scores P2: " + String(scores[1]));
    //If the target just left the goal and the button was not pressed
    //OR
    //If they were never in the goal and the button was pressed
    if (hasEnteredGoal[index] && !buttonPressed[index] || buttonCooldown[index] <= 0 && !hasEnteredGoal[index] && buttonState[index])//digitalRead(BUTTON_PIN_1) == 1)
    {
      //Serial.println("1");
      if (buttonCooldown[index] <= 0 && buttonState[index])
      {
        buttonCooldown[index] = cooldown;
        punishPlayer = true;
      }
      if (!pongMode)
        scores[index] -= scoreIncrem * (int(trickMode) + 4 * int(brutalMode)) / 2;
      else
      {
        int playerInd = playerNum;
        //Serial.println(String(playerInd) + " " + String(playerNum));
        if (playerInd >= 2) playerInd = 0;
        scores[playerInd] += scoreIncrem;
        targetSpeed = gameModeSpeeds[chosenGameOption];
        if (gameStarted)
          resetPong();
        //buttonCooldown[]
      }
      /*String text = "Player 1 score: " + String(scores[0]);
      if (playerCount == 2)
        text = " | Player 2 score: " + String(scores[1]);
      Serial.println(text);*/
    }
    buttonPressed[index] = false;
    hasEnteredGoal[index] = false;
    //Serial.println("Dir " + String(direction) + " " + String(targetPos));
    /*amount += 1;
    Serial.println(String(amount) + " " + String(targetPos));
    /*if (amount == 15)
    {
      amount = 0;
      Serial.println("Speed changed");
      doubleSpeed = !doubleSpeed;
    }*/
    //if (!buttonPressed)
  }
}

void checkTargetEnteredAnyGoal()
{
  //Serial.println(String(checkTargetIsInGoal(1)) + " " + String(checkTargetIsInGoal(2)) + " " + String(targetPos));
  checkTargetEntersGoal(1);
  if (playerCount == 2)
    checkTargetEntersGoal(2);
}

void chooseATrick()
{
  if (switcherooCooldown > 0) switcherooCooldown--;
  if (punishPlayer || switched) return;
  /*if (playerNum > playerCount)
    playerNum = playerCount;*/
  //int distance = (targetPos - goalPos[playerNum - 1]) * -direction;
  int distance_1 = (targetPos - goalPos[0]) * -direction;
  int distance_2 = (targetPos - goalPos[1]) * -direction;

  int random_Other = random(8);
  if (distance_1 > 1 && distance_1 <= 3 || playerCount == 2 && distance_2 > 1 && distance_2 <= 3)
  {
    int ran = random(10);
    //Serial.println("Random " + String(ran) + " " + String(targetPos));
    int dirChange[] = {-1,1};
    if (ran < 2)
      direction = dirChange[random(0,2)];
    else if (ran < 5 && (distance_1 == 2 || playerCount == 2 && distance_2 == 2))
    {
      long waitTime = (random(2) + 1) * 500;
      leds[targetPos] = CHSV(126, 255, 255);
      FastLED.show();
      stepsSlowedAfterStopping = 6;
      //Serial.println("WOW!! " + String(waitTime) + " " + String(waitTime > 0));
      while (waitTime > 0)
      {
        /*if (digitalRead(BUTTON_PIN_1) == 1)
        {
          scores[0] -= scoreIncrem;
          String text = "Player 1 score: " + String(scores[0]);
          if (playerCount == 2)
            text = " | Player 2 score: " + String(scores[1]);
          Serial.println(text);
        }*/
        int delayIndex = 10;
        buttonState[0] = digitalRead(BUTTON_PIN_1);
        buttonState[1] = digitalRead(BUTTON_PIN_2);
        updateComponentCooldowns(delayIndex);
        
        checkTargetEnteredAnyGoal();
        displayScores();
        if (brutalMode && punishPlayer)
        {
          waitTime = 0;
          stepsSlowedAfterStopping = 0;
        }
        delay(10);
        waitTime -= 10;
      }
      direction = dirChange[random(0,2)];
    }
  }  
  else if (random_Other < 2 && brutalMode && -distance_1 > 10 && -distance_1 < 12 && switcherooCooldown <= 0)
  {
      switched = true;
      switcherooCooldown = random(16) + 15;
      direction = direction * -1;
  }
    
  else if (random_Other < 3 && brutalMode && ((distance_1 <= LedCount / 2 && distance_1 > 10) || playerCount == 2 && (distance_2 <= (LedCount / (2 * playerCount)) && distance_2 > 10)))
  {
    increasedSpeedDuration = LedCount;// / 2;
  }
  /*if (increasedSpeedDuration > 0)
    Serial.println("ZOOOm " + String((1000/targetSpeed) / (2 + int(increasedSpeedDuration > 0))) + " " + String(increasedSpeedDuration));
*/}

bool checkTargetIsInAGoal()
{
  //if (checkTargetIsInGoal(1))
  //Serial.println("Test " + String(checkTargetIsInGoal(1)) + " " + String(checkTargetIsInGoal(2)) + " " + String(targetPos));
  return (checkTargetIsInGoal(1) || checkTargetIsInGoal(2));
}

bool checkTargetIsInGoal(int playerNum)
{
  if (playerNum > playerCount)
    playerNum = playerCount;

  //Serial.println(String(targetPos) + " " + String(targetPos == (goalPos[playerNum - 1] - 1)) + " " + String(targetPos == goalPos[playerNum - 1]) + " " + String(targetPos == (goalPos[playerNum - 1] + 1)));
  
  if (targetPos == (goalPos[playerNum - 1] - 1) || targetPos == goalPos[playerNum - 1] || targetPos == (goalPos[playerNum - 1] + 1))
    //if (playerNum == 1 || playerNum == 2 && (targetPos == (goalPos[1] - 1 || goalPos[1] || goalPos[1] + 1)))
    return true;

  return false;
}

