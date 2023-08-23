//define the snake as a struct
typedef struct Snake Snake;
struct Snake{
  int head[2];     //(row, column) of the snake head
  int body[62][2]; //(row, column)'s of the snake body
  int len;         //length of the snake
  int dir[2];      //direction to move the snake along
};

//define the apple as a struct
typedef struct Apple Apple;
struct Apple{
  int rPos; //row index of the apple
  int cPos; //column index of the apple
};

static byte pattern[8][8];  //keeps track of which LEDs should be on

const int X_PIN = A5; //VRX pin on joystick
const int Y_PIN = A4; //VRY pin on joystick
const int SW_PIN = 0; //SW pin on joystick
const int BUTTON = 1; //red pushbutton

//arrays for the anode (+) and cathode (-) wire pins
const byte ANODE_PINS[8] = {13, 12, 11, 10, 9, 8, 7, 6};
const byte CATHODE_PINS[8] = {A3, A2, A1, A0, 5, 4, 3, 2};

//variables to handle the game time
float oldTime = 0;
float timer = 0;
float updateRate = 3;

//initialize a snake object and an apple object
Snake snake = {{1,5},{{0,5}, {1,5}}, 2, {1,0}};
Apple apple = {(int)random(0,8), (int)random(0,8)};

//setup runs once
void setup() {
  //confirgue anodes/cathodes to outputs and turn them off
  for (byte i = 0; i < 8; i++) {
    pinMode(ANODE_PINS[i], OUTPUT);
    pinMode(CATHODE_PINS[i], OUTPUT);
    digitalWrite(ANODE_PINS[i], HIGH);
    digitalWrite(CATHODE_PINS[i], HIGH);
  }

  //set joystick and regular pushbuttons as inputs
  pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(BUTTON, INPUT_PULLUP);
}

//loop repeatedly runs
void loop() {
  //update game time variables
  float deltaTime = calculateDeltaTime();
  timer += deltaTime;

  //get joystick inputs
  int xVal = analogRead(X_PIN);
  int yVal = analogRead(Y_PIN);

  //restart the game if the red pushbutton is pressed
  if (digitalRead(BUTTON) == 0) {
    snake = {{1,5},{{0,5}, {1,5}}, 2, {1,0}};
    generateApple();
    updateRate = 3;
    return;
  }

  //speed up snake if joystick button is pressed
  if (digitalRead(SW_PIN) == 0) {
    delay(50);
    updateRate += 0.5;
    return;
  }
  
  //update snake direction based on joystick inputs
  if (yVal > 920 && snake.dir[1] == 0){
    snake.dir[0] = 0;
    snake.dir[1] = -1;
  } else if (yVal < 100 && snake.dir[1] == 0){
    snake.dir[0] = 0;
    snake.dir[1] = 1;
  } else if (xVal < 100 && snake.dir[0] == 0){
    snake.dir[0] = -1;
    snake.dir[1] = 0;
  } else if (xVal > 920 && snake.dir[0] == 0){
    snake.dir[0] = 1;
    snake.dir[1] = 0;
  }
  
  //update snake if enough time has passed
  if (timer > 1000/updateRate) {
    timer = 0;
    update();
  }
  
  //render display
  render();
}

//find how much time has passed since oldTime
float calculateDeltaTime(){
  float currentTime = millis();
  float dt = currentTime - oldTime;
  oldTime = currentTime;
  return dt;
}

//turn all LEDs off
void reset(){
  for(int i = 0; i < 8; i++){
    for(int j = 0; j < 8; j++){
      pattern[i][j] = 0;
    }
  }
}

//updates the components of the snake and apple structs
void update(){
  //clear the LED matrix
  reset();
  
  //get the position of the new snake head
  int newHead[2] = {snake.head[0] + snake.dir[0], snake.head[1] + snake.dir[1]};

  //handle borders by allowing snake to pass through
  if (newHead[0] == 8){
    newHead[0] = 0;
  } else if (newHead[0] == -1){
    newHead[0] = 7;
  } else if (newHead[1] == 8){
    newHead[1] = 0;
  } else if (newHead[1] == -1){
    newHead[1] = 7;
  }

  //check if the snake hits itself
  for (int j = 0; j < snake.len; j++){
    if (snake.body[j][0] == newHead[0] && snake.body[j][1] == newHead[1]){
      //display score until joystick is pressed and then restart game
      reset();
      tensPlace(snake.len / 10);
      onesPlace(snake.len % 10);
      while (digitalRead(BUTTON) == 1) {
        render();
      }
      snake = {{1,5},{{0,5}, {1,5}}, 2, {1,0}};
      generateApple();
      updateRate = 3;
      return;
    }
  }

  //check if the snake ate the apple
  if (newHead[0] == apple.rPos && newHead[1] == apple.cPos){
    snake.len += 1;
    generateApple();
  } else {
    removeFirst();  //shift the snake body array to the left
  }
  
  //update the snake head
  snake.body[snake.len-1][0] = newHead[0];
  snake.body[snake.len-1][1] = newHead[1];
  snake.head[0] = newHead[0];
  snake.head[1] = newHead[1];
  
  //update the pattern array to display the snake and apple
  for(int j = 0; j < snake.len; j++){
    pattern[snake.body[j][0]][snake.body[j][1]] = 15;
  }
  pattern[apple.rPos][apple.cPos] = 3;
}

//displays LEDs with brightness according to pattern array
void render(){
  for (byte a = 0; a < 8; a++) {
    digitalWrite(ANODE_PINS[a], LOW);
    for (byte c = 0; c < 8; c++) {
      byte time = pattern[a][c];
      for (byte i = 0; i < 16; i++) {
        if (i < time) {
          digitalWrite(CATHODE_PINS[c], LOW);
        } else {
          digitalWrite(CATHODE_PINS[c], HIGH);
        }
      }
    }
    digitalWrite(ANODE_PINS[a], HIGH);
  }
}

//moves elements of snake body array up one
void removeFirst(){
  for(int j = 1; j < snake.len; j++){
    snake.body[j-1][0] = snake.body[j][0];
    snake.body[j-1][1] = snake.body[j][1];
  }
}

//generates a new apple outside of the snake
void generateApple() {
  apple = {(int)random(0,8), (int)random(0,8)};

  if (apple.rPos == snake.head[0] && apple.cPos == snake.head[1]) {
    return generateApple();
  }

  for (int i = 0; i < snake.len; i++) {
    if (apple.rPos == snake.body[i][0] && apple.cPos == snake.body[i][1]) {
      return generateApple();
    }
  }
}

//seven segment display for the tens place of the score
void tensPlace(int tens) {
  if (tens == 2 || tens == 3 || tens == 5 || tens == 6) {
    pattern[1][1] = 15;
    pattern[1][2] = 15;
  }
  if (tens == 1 || tens == 4 || tens == 5 || tens == 6) {
    pattern[2][0] = 15;
    pattern[3][0] = 15;
  }
  if (tens == 2 || tens == 3 || tens == 4) {
    pattern[2][3] = 15;
    pattern[3][3] = 15;
  }
  if (tens == 2 || tens == 3 || tens == 4 || tens == 5 || tens == 6) {
    pattern[4][1] = 15;
    pattern[4][2] = 15;
  }
  if (tens == 1 || tens == 2 || tens == 6) {
    pattern[5][0] = 15;
    pattern[6][0] = 15;
  }
  if (tens == 3 || tens == 4 || tens == 5 || tens == 6) {
    pattern[5][3] = 15;
    pattern[6][3] = 15;
  }
  if (tens == 2 || tens == 3 || tens == 5 || tens == 6) {
    pattern[7][1] = 15;
    pattern[7][2] = 15;
  }
}

//seven segment display for the ones place of the score
void onesPlace(int ones) {
  if (ones == 0 || ones == 2 || ones == 3 || ones == 5 ||
      ones == 6 || ones == 7 || ones == 8 || ones == 9) {
    pattern[1][5] = 15;
    pattern[1][6] = 15;
  }
  if (ones == 0 || ones == 1 || ones == 4 || ones == 5 ||
      ones == 6 || ones == 8 || ones == 9) {
    pattern[2][4] = 15;
    pattern[3][4] = 15;
  }
  if (ones == 0 || ones == 2 || ones == 3 || ones == 4 ||
      ones == 7 || ones == 8 || ones == 9) {
    pattern[2][7] = 15;
    pattern[3][7] = 15;
  }
  if (ones == 2 || ones == 3 || ones == 4 || ones == 5 ||
      ones == 6 || ones == 8 || ones == 9) {
    pattern[4][5] = 15;
    pattern[4][6] = 15;
  }
  if (ones == 0 || ones == 1 || ones == 2 ||
      ones == 6 || ones == 8) {
    pattern[5][4] = 15;
    pattern[6][4] = 15;
  }
  if (ones == 0 || ones == 3 || ones == 4 || ones == 5 ||
      ones == 6 || ones == 7 || ones == 8 || ones == 9) {
    pattern[5][7] = 15;
    pattern[6][7] = 15;
  }
  if (ones == 0 || ones == 2 || ones == 3 ||
      ones == 5 || ones == 6 || ones == 8) {
    pattern[7][5] = 15;
    pattern[7][6] = 15;
  }
}