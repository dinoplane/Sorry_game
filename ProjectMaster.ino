#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Keypad.h>

// Keypad constants
const byte ROWS = 4; 
const byte COLS = 4; 
char hexaKeys1[ROWS][COLS] = {
  {'1', '2', '3', 'a'},
  {'4', '5', '6', 'b'},
  {'7', '8', '9', 'c'},
  {'Y', '0', 'Z', 'd'}
};

char hexaKeys2[ROWS][COLS] = {
  {'A', 'B', 'C', 'a'},
  {'D', 'E', 'F', 'b'},
  {'G', 'H', 'I', 'c'},
  {'J', 'K', 'L', 'd'}
};

char hexaKeys3[ROWS][COLS] = {
  {'M', 'N', 'O', 'a'},
  {'P', 'Q', 'R', 'b'},
  {'S', 'T', 'U', 'c'},
  {'V', 'W', 'X', 'd'}
};

// Digital Pins
byte rowPins[ROWS] = {11, 10, 9, 8}; 
byte colPins[COLS] = {7, 6, 5, 4}; 
int latchPin = 10;
int clockPin = 12;
int dataPin = 11;
int sw = 2;

// Analog Pins
int xcoord = 0;
int ycoord = 1;

// Initializations
Keypad customKeypad = Keypad(makeKeymap(hexaKeys1), rowPins, colPins, ROWS, COLS); 
LiquidCrystal_I2C lcd(0x27, 16, 2);
// For the following array values, they correspond to red, blue, green, yellow
char buf[10] = "         ";
char * names[4] = {}; 
int pawn_homes[4] = {2, 10, 18, 26};
int pawn_pos[4][3] = {{},{},{},{}}; // row 1: red, row 2: blue, etc...
bool pawn_pass[4][3] = {{false, false, false},
                        {false, false, false},
                        {false, false, false},
                        {false, false, false}};

// Helper variables
int numplayers = 0;
int numPawns = 3;
int charcount = 0;

int xnorm;
int ynorm;
long lint = 0;
word high_b = 0;
word low_b = 0;
byte* leds[4];

boolean waitingForInput = false;
boolean switchOn = false;

int curr_player = 0;
int curr_pawn = 0;
int currRoll = 0;
long boardstate = 0;

// Adds characters to LCD and buffer
void addToLine1(char customKey)
{
    buf[charcount] = customKey;
    lcd.setCursor(0, 1);
    lcd.print(buf);
    if (charcount != 9) {charcount++;};
}

// Strips trailing whitespace
void trimWhitespace()
{
  char *end;

  // Trim trailing space
  end = buf + 8;
  while(end > buf && isspace((unsigned char)*end)) end--;
  
  // Write new null terminator character
  end[1] = '\0';
}

// Backspaces all characters
void backSpace()
{
    if (charcount != 0) {charcount--;};
    buf[charcount] = ' ';
    lcd.setCursor(0, 1);
    lcd.print(buf);
}

// Turns switch on if waiting for reply
void turnSwitchOn()
{
  if (waitingForInput){ switchOn = true;} 
}

// Gets number of players
void getPlayers()
{
  while (true)
  {
    char customKey = customKeypad.getKey();
    if (customKey)
      {
      switch (customKey)
      {
        case '2':
          numplayers = 2;
          break;
        case '3':
          numplayers = 3;
          break;
        case '4':
          numplayers = 4;
          break;
        default:
          lcd.setCursor(0, 0);
          lcd.print("Invalid Number.  ");
          lcd.setCursor(0, 1);
          lcd.print("Try Again");
          delay(3000);
          break;
      }
    }
    if (numplayers != 0)
    {
      break;
    }
  }
}

// Gets names of players
void getNames()
{
 for (int i = 0; i < numplayers; i++)
 {
    while (true)
    {
      waitingForInput = true;
      if (switchOn == true)
      {
        trimWhitespace();
        names[i] = strdup(buf);
        memset(buf, ' ', 9*sizeof(char));
        lcd.setCursor(0, 1);
        lcd.print(buf);
        charcount = 0;
        switchOn = false;
        waitingForInput = false;
        break;
      }
      char customKey = customKeypad.getKey();
      if (customKey)
      {
        if (customKey == 'a') { customKeypad.begin(makeKeymap(hexaKeys1)); }
        else if (customKey == 'b') { customKeypad.begin(makeKeymap(hexaKeys2)); }
        else if (customKey == 'c') { customKeypad.begin(makeKeymap(hexaKeys3)); }
        else if (customKey == 'd') { backSpace(); }
        else
        {        
          if (charcount < 9)
          {
            addToLine1(customKey);
          }
        }
      }
    }
  }
}

// Sends values in led array to slave arduino
void sendLEDArray()
{
  Wire.beginTransmission(9);
  for (int i = 0; i < 4; i++)
  {
    Wire.write((byte)leds[i]);
  }
  Wire.endTransmission();
}

// Gets Roll from current player
void getRoll()
{
  lcd.clear();
  lcd.print("Please Roll.");
  waitingForInput = true;
  while (true)
  {
    delay(100);
    if (switchOn)
    {
      currRoll = (int) random(1,13);
      switchOn = false;

      break;
    }
  }
  waitingForInput = false;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("You rolled a ");
  lcd.print(currRoll);
  delay(2000);
}

// Initialize pawns
void initializeBoard()
{
  for (int i = 0; i < numplayers; i++)
  {
    for (int j = 0; j < numPawns; j++)
    {
      pawn_pos[i][j] = pawn_homes[i];
    }
  }
  // set lint
  updateBoard();
  sendLEDArray();
}

// Display Board for debugging purposes
void DisplayBoard()
{
  for (int i = 0; i < numplayers; i++)
  {
    for (int j = 0; j < numPawns; j++)
    {
      Serial.print(pawn_pos[i][j]);
      Serial.print(" ");
    }
    Serial.println();
  }
}

// Updates lint for player
void updateForPlayer(int player)
{
  for (int j = 0; j < numPawns; j++)
  {
    bitSet(lint, pawn_pos[player][j]);
  }
}

// Updates lint for all players
void updateBoard()
{
  lint = 0;
  for (int i = 0; i < numplayers; i++)
  {
    updateForPlayer(i);
  }
  updateLightArray();
}

// Update Led Array
void updateLightArray()
{
  int i = 0;
  high_b = lint >> 16;
  low_b = (word) lint;
  leds[i] = highByte(high_b);
  i++;
  leds[i]= lowByte(high_b);
  i++;
  leds[i] = highByte(low_b);
  i++;
  leds[i]= lowByte(low_b);
}

// Sets the lint for player to 0
void setPlayersToZero(int player)
{
  for (int j = 0; j < numPawns; j++)
  {
    bitClear(lint, pawn_pos[player][j]);
  }
}

// Select Pawn
void getSelectedpawn()
{
  curr_pawn =  0;
  lint = 0;

  updateForPlayer(curr_player);
  updateLightArray();
  sendLEDArray();
  waitingForInput = true;
  lcd.setCursor(0,1);
  
  while (true)
  {
    
    int x = analogRead(xcoord);
    int y = analogRead(ycoord);
    
    if (x > xnorm + 60)
    {
      updateForPlayer(curr_player);
      curr_pawn = (curr_pawn + 1) % numPawns; 
    }
    else if (x < xnorm - 60)
    {
      updateForPlayer(curr_player);
      if (curr_pawn == 0) curr_pawn = numPawns-1;
      else curr_pawn = (curr_pawn - 1) % numPawns;
    }
    lcd.setCursor(0,1);
    lcd.print("Select a pawn: ");
    lcd.setCursor(15,1);
    lcd.print(curr_pawn);
    
    //Allow seeing boardstate
    if (y > ynorm + 60 or y < ynorm - 60)
    {
      updateBoard();
      sendLEDArray();
    }
    else
    {
      for (int i = 0; i < 4; i++)
      {
        if (i == curr_player) continue;
        setPlayersToZero(i);
      }
      //Toggle pawn's light
      bitWrite(lint, pawn_pos[curr_player][curr_pawn], not bitRead(lint, pawn_pos[curr_player][curr_pawn]));
      updateLightArray();
      sendLEDArray();
    }

    if (switchOn)
    {
      switchOn = false;
      if (pawn_pos[curr_player][curr_pawn] == pawn_homes[curr_player] and pawn_pass[curr_player][curr_pawn])
      {
        lcd.setCursor(0,1);
        lcd.print("Invalid Pawn    ");
        delay(2000);
      }
      else
      {
        waitingForInput = false;
        break;
      }
    }
    delay(250);
  }
}

// Curr pawn moves one space
bool moveOne()
{
  int test_space = pawn_pos[curr_player][curr_pawn] + 1;
  // Check if pawn is going to go over
  if (test_space >= 32)
  {
    test_space -= 32;
    pawn_pass[curr_player][curr_pawn] = true;
  }
  // Check if pawn is going to be at a home
  for (int i = 0; i < 4; i++)
  {
    if (pawn_homes[i] - 2 <= test_space  and test_space <= pawn_homes[i])
    {
      if (i == curr_player)
      {
        if (test_space == pawn_homes[curr_player])
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(names[curr_player]);
          lcd.print(" ");
          lcd.print(curr_pawn);       
          lcd.setCursor(0,1);
          lcd.print("went home.");
          pawn_pos[curr_player][curr_pawn] = test_space;
          updateBoard();
          sendLEDArray();
          delay(2000);
          return true;
        }
      }
      else
      {
        test_space = pawn_homes[i] + 1;
      }
    }  
  }
  // Update everything
  pawn_pos[curr_player][curr_pawn] = test_space;
  updateBoard();
  sendLEDArray();
  return false;
}

void setup() {
  Wire.begin();
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.noAutoscroll();
  customKeypad.setDebounceTime(100);
  attachInterrupt(digitalPinToInterrupt(sw), turnSwitchOn, FALLING);
  xnorm = analogRead(xcoord);
  ynorm = analogRead(ycoord);

  // Start receiving input
  lcd.print("How many players?");
  getPlayers();
  initializeBoard();
    
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter your name: ");
  getNames();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.println("GAME START      ");
  delay(5000);
}

void loop() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(names[curr_player]);
  lcd.print("'s Turn");
  delay(1750);
  getRoll();
  getSelectedpawn();

  // Move the pawn step by step
  for (int j = 0; j < currRoll; j++)
  {
    // Stop when the roll is done or the pawn reaches home
    bool pawnStop = moveOne();
    delay(500);
    if (pawnStop) break;
  }
  
  // Check if there is a sorry
  for (int i = 0; i < numplayers; i++)
  {
    if (i == curr_player)
    {
      continue; 
    }
    for (int j = 0; j < numPawns; j++)
    {
      Serial.println("Checking");
      Serial.print(names[curr_player]);
      Serial.print(" ");
      Serial.print(curr_pawn);
      Serial.print(" at ");
      Serial.println(pawn_pos[curr_player][curr_pawn]);
      Serial.print(names[i]);
      Serial.print(" ");
      Serial.print(j);
      Serial.print(" at ");
      Serial.println(pawn_pos[i][j]);
      if (pawn_pos[curr_player][curr_pawn] == pawn_pos[i][j])
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("SORRY!!");
        delay(5000);
        lcd.clear();
        lcd.print(names[curr_player]);
        lcd.print(" ");
        lcd.print(curr_pawn);
        lcd.print(" bmps");
        lcd.setCursor(0,1);
        lcd.print(names[i]);
        lcd.print(" ");
        lcd.print(j);
        delay(3000);
        pawn_pos[i][j] = pawn_homes[i]; 
        updateBoard();
        sendLEDArray();
      }
    }
  }

  // Check if the player has won
  int homeCount = 0;
  for (int i = 0; i < numPawns; i++)
  {
    if (pawn_pos[curr_player][i] == pawn_homes[curr_player] and pawn_pass[curr_player][i])
    {
      homeCount++;
    }
  }

  if (homeCount == numPawns)
  {
    lcd.clear();
    while (true)
    {
      lcd.setCursor(0,0);
      lcd.print(names[curr_player]);
      lcd.print(" Wins!");
    }
  }
  curr_player = (curr_player + 1) % numplayers;
}
  
