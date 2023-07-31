#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

#define SHIP 0 // Custom characters defined as integers
#define PAC 1
#define GHOST 2
#define ROBOT 3
#define MENU 4 // State to choose menu options
#define PLAY 5 // State to choose symbols
#define CHOOSE_LENGTH 6 // State to choose sequence length
#define CHOOSE_SYMBOL 7 // State to choose symbols to include
#define SHOW 8 // State to show sequence
#define END_GAME 9 // State after losing game
#define HIGHSCORE 10 // State to show Highscore board
#define DIFFICULTY 11 // State to choose difficulty
#define ALIAS 12 // State to add initials to new highscores

byte ship[8]  = {B01110, B01110, B11111, B10101, B11111, B11011, B11111, B10101};
byte pac[8]   = {B00000, B01110, B11111, B11110, B11100, B11110, B11111, B01110};
byte ghost[8] = {B01110, B10001, B11011, B10001, B10101, B10001, B10101, B01010};
byte robot[8] = {B11111, B10101, B11111, B11111, B11111, B01010, B01010, B11011};

unsigned int *outputSeq = NULL; // array to store the random sequence to show
unsigned int sequence = 4; // length of the sequence of symbols in the game
unsigned int symbSize = 2; // number of symbols in the game
unsigned int symbol = SHIP; // store the int representation of the custom characters
unsigned int period = 500; // delay between symbols shown
char* mode = "story"; // store main menu options
char* difficulty = "normal"; // store options for story mode
uint8_t buttons, changes; // to read buttons pressed
uint8_t oldButtons = 0; // button state before pressing again
uint8_t state = NULL; // for the finite state machine
unsigned int inputCounter = 0; // counts number of symbols entered when playing
unsigned int scoreCounter = 0; // counts correct symbols entered
char alphabet[] = {'A','B','C','D','E','F','G','H','I','G','K','L','M','N','O',
'P','Q','R','S','T','U','V','W','X','Y','Z'}; // array to enter initials
int letterIndex = 0; // index for the alphabet array
char initial; // store initials for alias
unsigned int aliasAddress = sizeof(int); // address to store alias
unsigned int highScore = EEPROM.get(0, highScore); // call highscore stored in eeprom
String alias = "";

void create_characters()
{
  lcd.createChar(SHIP, ship);
  lcd.createChar(PAC, pac);
  lcd.createChar(GHOST, ghost);
  lcd.createChar(ROBOT, robot);
}

void setup() 
{
  Serial.begin(9600);
  Serial.println("GAME IS RUNNING");
  Serial.println("state == MENU");
  Serial.println();
  lcd.begin(16, 2);
  create_characters();
  state = MENU;
}



void display_difficulty()
{
  lcd.setCursor(0, 0);
  lcd.print("Difficulty:");
  lcd.setCursor(0, 1);
  if (difficulty == "easy")   {lcd.print("EASY  ");}
  if (difficulty == "normal") {lcd.print("NORMAL");}
  if (difficulty == "hard")   {lcd.print("HARD  ");}
}
void initialise_difficulty()
{
  Serial.print("Difficulty: ");
  Serial.println(difficulty);

  if (difficulty == "easy")   {sequence = 4; symbSize = 2; period = 750;}
  if (difficulty == "normal") {sequence = 4; symbSize = 3; period = 550;}
  if (difficulty == "hard")   {sequence = 4; symbSize = 4; period = 350;}
}



void display_symbol_amount()
{
  char symBuff[14];
  lcd.setCursor(0, 0);
  lcd.print("Choose Amount Of");
  lcd.setCursor(0, 1);
  sprintf(symBuff, "Symbols Used: %1d", symbSize);
  lcd.print(symBuff);
}
void initialise_symbol_amount()
{
  char symBuff[16];
  sprintf(symBuff, "Symbols used: %1d", symbSize);

  Serial.println(symBuff);
  Serial.println();
}



void display_menu()
{
  lcd.setCursor(0, 0);
  lcd.print("Menu Options:");
  lcd.setCursor(0, 1);
  if (mode == "practice") {lcd.print("Practice Mode");}
  if (mode == "story")    {lcd.print("Story Mode   ");}
  if (mode == "score")    {lcd.print("High Score   ");}
}
void initialise_mode()
{
  Serial.print("Menu option chosen: ");
  Serial.println(mode);

  if (mode == "practice") {state = CHOOSE_LENGTH;}
  if (mode == "story") {state = DIFFICULTY;}
  if (mode == "score") {state = HIGHSCORE;}
}



void check_highscore()
{
  if (scoreCounter > highScore)
  {
    highScore = scoreCounter;
    EEPROM.put(0, highScore);
    Serial.print("Highscore in EEPROM updated!: ");
    Serial.println(highScore);
    Serial.println();

    lcd.setCursor(0, 0);
    lcd.print(" NEW HIGHSCORE  ");
    lcd.setCursor(0, 1);
    lcd.print("   REACHED!     ");
    delay(1000);
    lcd.clear();

    state = ALIAS;
  }
  else
  {
    Serial.print("Current score not stored to EEPROM: ");
    Serial.println("Too low or equal to Highscore");
    Serial.print("Current Score: ");
    Serial.println(scoreCounter);
    Serial.print("Highscore: ");
    Serial.println(highScore);
    Serial.println();

    state = MENU;
  }
}
void display_highscore()
{

  lcd.setCursor(0, 0);
  if (state == HIGHSCORE)
  {
    lcd.print("HIGHSCORE: ");
    lcd.print(highScore);
    lcd.print(" ");
    if (alias != "") {lcd.print(alias);}
  }
  if (state == ALIAS) // Enter alias for new highscore
  {
    lcd.print("NEW HIGHSCORE!!!");
    initial = alphabet[letterIndex];
    lcd.setCursor(0, 1);
    lcd.print("Initials: ");
    lcd.print(initial);
  }
}
void create_alias()
{
  if (alias.length() < 3)
  {
    alias += initial;
    Serial.print("Alias: ");
    Serial.println(alias);

    if (alias.length() == 3)
    {
      state = MENU;
      Serial.println("Game is RESTARTING");
    }
  }
}



void ready_msg()
{
  lcd.setCursor(0,0);
  lcd.print("  PRESS SELECT  ");
  lcd.setCursor(0,1);
  lcd.print("   WHEN READY   ");
}



void display_sequence() 
{
  int *randSeqArr = (int*) malloc(sequence*sizeof(int));
  outputSeq = randSeqArr;

  if (outputSeq != NULL)
  {
    Serial.println("Memory of outputSeq array from previous game FREED");
    free(outputSeq);
  }

  Serial.println("Random Sequence Array Shown on lcd: ");
  for (int i=0; i<sequence; i++)
  {
    randomSeed(analogRead(1));
    int randCh = random(symbSize);
    outputSeq[i] = randCh;
    lcd.setCursor(7, 0);
    lcd.print((char)randCh);
    
    Serial.print("[");
    Serial.print(i, DEC);
    Serial.print("] = ");
    Serial.println(outputSeq[i]);

    delay(period);
    lcd.clear();
    delay(period);
  }
}



void display_buttons_symbol()
{
  lcd.setCursor(0, 0);
  lcd.print("Enter Sequence");
  lcd.setCursor(0, 1);
  lcd.print("Shown: ");
  if (symbol == SHIP) {lcd.print((char)SHIP);}
  if (symbol == GHOST) {lcd.print((char)GHOST);}
  if (symbol == PAC) {lcd.print((char)PAC);}
  if (symbol == ROBOT) {lcd.print((char)ROBOT);}
}



void display_sequence_length() 
{
  char seqBuff[10];
  lcd.setCursor(0, 0);
  lcd.print("Choose Sequence");
  lcd.setCursor(0, 1);
  sprintf(seqBuff, "Length: %2d", sequence);
  lcd.print(seqBuff);
}
void initialise_sequence_length()
{
  char seqBuff[20];
  sprintf(seqBuff, "Sequence Length: %2d", sequence);
  Serial.println(seqBuff);
}



void check_input(int symbol)
{
  Serial.println("Values when SELECT is pressed");
  Serial.print("Current index of the outputSeq array: ");
  Serial.println(inputCounter);
  Serial.print("Integer representation of current button value: ");
  Serial.println(symbol);
  Serial.print("Current element of the outputSeq array to match: ");
  Serial.println(outputSeq[inputCounter]);
  Serial.println();

  if(outputSeq[inputCounter] != symbol) // End game if wrong symbol is chosen
  {
    Serial.println("WRONG INPUT");
    Serial.print("SCORE COUNTER: ");
    Serial.println(scoreCounter);

    lcd.clear();
    lcd.setBacklight(1); // Red LED
    lcd.setCursor(0, 0);
    lcd.print("  WRONG INPUT   ");
    delay(1000);
    inputCounter = 0; // Reset
    state = END_GAME;
  } 
  else // Green flash if symbol chosen is correct
  {
    lcd.clear();
    lcd.setBacklight(2); // Green LED
    delay(250);
    lcd.clear();
    lcd.setBacklight(7); // White LED
    
    inputCounter++;
    scoreCounter++;

    Serial.println("CORRECT INPUT"); // Show correct input on Serial
    Serial.print("SCORE COUNTER: ");
    Serial.println(scoreCounter);
    Serial.println();
  }

  if (inputCounter == sequence) // Set the next stage when all symbols are entered correctly
  {
    Serial.println("Sequence Completed: Showing Next Sequence");

    inputCounter = 0;
    sequence++;

    // make the game progressively harder each stage in story mode
    if (mode == "story" && sequence == 8 && symbSize < 4) 
    {
      sequence = 4; // Reset sequence to 4
      symbSize++; // Show more types of symbols
      period = period - 200; // Symbols appear for less time

      Serial.println("Difficulty Increase: New symbol included");
      Serial.println("Difficulty Increase: Delay reduced");
      Serial.print("Amount of symbols used: ");
      Serial.println(symbSize);
      Serial.println();
    }

    // Showing the transition to the next stage on the lcd
    lcd.clear();
    lcd.setBacklight(2); // Green LED
    lcd.setCursor(0, 0);
    lcd.print("   NEXT STAGE   "); 
    delay(1000);
    lcd.setBacklight(7); // White LED
    state = SHOW;
  }
}



void game_over_message() // Message when you lose
{
  lcd.clear();
  lcd.setBacklight(1); // Red LED
  lcd.setCursor(0, 0);
  lcd.print("   GAME OVER   ");
  Serial.println("GAME OVER");
  Serial.println();
  delay(1000);
  lcd.clear();
  lcd.setBacklight(7); // White LED
}



void loop() 
{
  buttons = lcd.readButtons();
  changes = oldButtons & ~buttons;
  switch (state)
  {
    case MENU:
      display_menu();
      if (changes & BUTTON_UP) {mode = "story";}
      if (changes & BUTTON_DOWN) {mode = "score";}
      if (changes & BUTTON_LEFT) {mode = "practice";}
      if (changes & BUTTON_SELECT) {lcd.clear(); initialise_mode();}
      break;

    case DIFFICULTY:
      display_difficulty();
      if (changes & BUTTON_UP) {difficulty = "hard";}
      if (changes & BUTTON_DOWN) {difficulty = "easy";}
      if (changes & BUTTON_LEFT) {difficulty = "normal";}
      if (changes & BUTTON_SELECT) {lcd.clear(); initialise_difficulty(); state = SHOW;}
      break;

    case CHOOSE_LENGTH:
      display_sequence_length();
      if (changes & BUTTON_UP) {if (sequence == 10) {sequence = 4;} else {sequence++;}}
      if (changes & BUTTON_DOWN) {if (sequence == 4) {sequence = 10;} else {sequence--;}}
      if (changes & BUTTON_SELECT) 
      {
        initialise_sequence_length(); 
        lcd.clear(); 
        state = CHOOSE_SYMBOL;
      }
      break;

    case CHOOSE_SYMBOL:
      display_symbol_amount();
      if (changes & BUTTON_UP) {if (symbSize == 4) {symbSize = 2;} else {symbSize++;}}
      if (changes & BUTTON_DOWN) {if (symbSize == 2) {symbSize = 4;} else {symbSize--;}}
      if (changes & BUTTON_SELECT) 
      {
        initialise_symbol_amount();
        lcd.clear(); 
        state = SHOW;
      }
      break;

    case SHOW:
      ready_msg();
      if (changes & BUTTON_SELECT)
      {
        lcd.clear();
        display_sequence();
        state = PLAY;
      }
      break;

    case PLAY:
      display_buttons_symbol();
      if (changes & BUTTON_UP) {symbol = SHIP;}
      if (changes & BUTTON_DOWN) {symbol = GHOST;}
      if (changes & BUTTON_LEFT) {symbol = PAC;}
      if (changes & BUTTON_RIGHT) {symbol = ROBOT;}
      if (changes & BUTTON_SELECT) {check_input(symbol);}
      break;

    case END_GAME:
      game_over_message();
      check_highscore();
      break;

    case HIGHSCORE:
      display_highscore();
      break;

    case ALIAS:
      display_highscore();
      if (changes & BUTTON_UP) {if (letterIndex == 25) {letterIndex = 0;} else {letterIndex++;}}
      if (changes & BUTTON_DOWN) {if (letterIndex == 0) {letterIndex = 25;} else {letterIndex--;}}
      if (changes & BUTTON_SELECT) {lcd.clear(); create_alias();}
      break;

  }
  oldButtons = buttons;
}