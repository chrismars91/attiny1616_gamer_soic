#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_DotStarMatrix.h>
#include <Adafruit_DotStar.h>
#include <Fonts/TomThumb.h>

const uint8_t DATA_PIN  = 14;
const uint8_t CLOCK_PIN = 16;
const uint8_t SWITCH_BUTTON = 4;

Adafruit_DotStarMatrix matrix = Adafruit_DotStarMatrix(
  8, 8, DATA_PIN, CLOCK_PIN,
  DS_MATRIX_BOTTOM + DS_MATRIX_LEFT +
  DS_MATRIX_ROWS + DS_MATRIX_PROGRESSIVE,
  DOTSTAR_BGR
);

// Colors
///////////////////////////////////////////
const uint16_t GAME_COLORS[] = { 
  36895, 63488, 64480, 53216, 
  2016, 2044, 63515, 65248 
};

const uint16_t GAMES_BY_CHRIS_COLORS[] = {
  matrix.Color(255, 0, 0),    // Red
  matrix.Color(255, 125, 0),  // Orange
  matrix.Color(200, 255, 0),  // Yellowish
  matrix.Color(0, 255, 0),    // Green
  matrix.Color(0, 255, 225),  // Blue
  matrix.Color(150, 0, 255),  // Purple
  matrix.Color(255, 0, 220),  // Pink
  matrix.Color(255, 220, 0)   // Orange/Yellow
};

const int GRID_SIZE = 8;
const unsigned long UPDATE_INTERVAL = 200;
const unsigned long INPUT_CHECK_INTERVAL = 50;

uint16_t gameColor1, gameColor2;
bool isFlappyBird = true;

void resetGameColors() {
    int index1 = random(8);
    int index2;
    do {
        index2 = random(8);
    } while (index1 == index2);
    
    gameColor1 = GAME_COLORS[index1];
    gameColor2 = GAME_COLORS[index2];
}

void centerText(int value) {
    char buffer[3];
    snprintf(buffer, sizeof(buffer), "%d", value);
    
    int16_t x1, y1;
    uint16_t w, h;
    matrix.getTextBounds(buffer, 0, 0, &x1, &y1, &w, &h);
    
    int x = (8 - w) / 2;
    int y = (8 + h) / 2;
    
    matrix.clear();
    matrix.setCursor(x, y);
    
    matrix.print(buffer);
    matrix.show();
}

void playIntro() {
    const char* gamesByChris = "Games by Chris";
    int x = matrix.width();
    
    for(int t = 0; t < 59; t++) {
        matrix.fillScreen(0);
        matrix.setCursor(x, 5);
        
        for (byte i = 0; gamesByChris[i] != '\0'; i++) {
            matrix.setTextColor(GAMES_BY_CHRIS_COLORS[i % (sizeof(GAMES_BY_CHRIS_COLORS)/sizeof(GAMES_BY_CHRIS_COLORS[0]))]);
            matrix.print(gamesByChris[i]);
        }
        
        if (--x < -50) x = matrix.width();
        matrix.show();
        delay(100);
    }
}

// Flappy Bird
///////////////////////////////////////////
class FlappyBird {
public:
    FlappyBird() { reset(); }
    
    void run() {
        update();
        render();
    }
    
    void jump() {
        if (gameOver) {
            reset();
        } else {
            velocity = JUMP_STRENGTH;
        }
    }

private:
    static constexpr int BIRD_X = 1;
    static constexpr int GRAVITY = 1;
    static constexpr int JUMP_STRENGTH = -2;
    static constexpr int GAP_SIZE = 3;
    
    int birdY, velocity, pipeX, gapY, score;
    bool gameOver;

    void reset() {
        resetGameColors();
        birdY = 3;
        velocity = 0;
        pipeX = 7;
        gapY = random(1, 6);
        gameOver = false;
        score = 0;

        matrix.setTextColor(GAMES_BY_CHRIS_COLORS[random(sizeof(GAMES_BY_CHRIS_COLORS)/sizeof(GAMES_BY_CHRIS_COLORS[0]))]);
    }

    void update() {
        if (gameOver) return;
        
        velocity += GRAVITY;
        birdY += velocity;
        
        if (birdY < 0 || birdY > 7) gameOver = true;
        
        if (--pipeX < 0) { 
            pipeX = 7; 
            gapY = random(1, 6); 
            score++; 
        }
        
        if (BIRD_X == pipeX && (birdY < gapY || birdY > gapY + GAP_SIZE - 1)) 
            gameOver = true;
    }

    void render() {
        matrix.clear();
        
        if (!gameOver) {
            matrix.drawPixel(BIRD_X, birdY, gameColor1);
            
            for (int i = 0; i < 8; i++) {
                if (i < gapY || i > gapY + GAP_SIZE - 1) {
                    matrix.drawPixel(pipeX, i, gameColor2);
                }
            }
        } else {
            centerText(score);
        }
        
        matrix.show();
    }
};

// Snake
///////////////////////////////////////////
class SnakeGame {
public:
    enum Direction { UP, DOWN, LEFT, RIGHT };
    
    SnakeGame() { reset(); }
    
    void run() {
        update();
        render();
    }
    
    void changeDirection(Direction newDir) {
        if ((dir == UP && newDir != DOWN) || 
            (dir == DOWN && newDir != UP) ||
            (dir == LEFT && newDir != RIGHT) || 
            (dir == RIGHT && newDir != LEFT)) {
            dir = newDir;
        }
    }
private:
    struct Point { int x, y; };
    
    Point snake[GRID_SIZE * GRID_SIZE];
    int length;
    Direction dir;
    int foodX, foodY;
    void reset() {
        length = 3;
        dir = RIGHT;
        
        for (int i = 0; i < length; i++) {
            snake[i] = { GRID_SIZE / 2 - i, GRID_SIZE / 2 };
        }
        
        spawnFood();
        matrix.setTextColor(GAMES_BY_CHRIS_COLORS[random(sizeof(GAMES_BY_CHRIS_COLORS)/sizeof(GAMES_BY_CHRIS_COLORS[0]))]);
    }

    void update() {
        Point newHead = snake[0];
        
        switch(dir) {
            case UP:    newHead.y--; break;
            case DOWN:  newHead.y++; break;
            case LEFT:  newHead.x--; break;
            case RIGHT: newHead.x++; break;
        }
        
        newHead.x = (newHead.x + GRID_SIZE) % GRID_SIZE;
        newHead.y = (newHead.y + GRID_SIZE) % GRID_SIZE;
        
        for (int i = 0; i < length; i++) {
            if (snake[i].x == newHead.x && snake[i].y == newHead.y) {
                centerText(length - 3);
                delay(2000);
                resetGameColors();
                reset();
                return;
            }
        }
        
        for (int i = length; i > 0; i--) snake[i] = snake[i - 1];
        snake[0] = newHead;
        
        if (newHead.x == foodX && newHead.y == foodY) {
            length++;
            spawnFood();
        }
    }

    void render() {
        matrix.clear();
        
        for (int i = 0; i < length; i++) 
            matrix.drawPixel(snake[i].x, snake[i].y, gameColor1);
        
        matrix.drawPixel(foodX, foodY, gameColor2);
        matrix.show();
    }

    void spawnFood() { 
        foodX = random(0, GRID_SIZE); 
        foodY = random(0, GRID_SIZE); 
    }
};

FlappyBird flappyBird;
SnakeGame snakeGame;

void setup() {
    matrix.begin();
    matrix.setBrightness(5);
    matrix.setFont(&TomThumb);
    matrix.setTextWrap(false);

    playIntro();
    
    pinMode(SWITCH_BUTTON, INPUT_PULLUP);
    pinMode(0, INPUT_PULLUP);  // Left
    pinMode(1, INPUT_PULLUP);  // Down
    pinMode(2, INPUT_PULLUP);  // Up
    pinMode(3, INPUT_PULLUP);  // Right
    
}

void handleGameInput() {
    if (!isFlappyBird) {
        // Snake game directional controls
        if (digitalRead(2) == LOW) snakeGame.changeDirection(SnakeGame::UP);
        if (digitalRead(1) == LOW) snakeGame.changeDirection(SnakeGame::DOWN);
        if (digitalRead(0) == LOW) snakeGame.changeDirection(SnakeGame::LEFT);
        if (digitalRead(3) == LOW) snakeGame.changeDirection(SnakeGame::RIGHT);
    } else {
        // Flappy Bird jump control
        if (digitalRead(2) == LOW) {
            flappyBird.jump();
        }
    }
}

void loop() {
    static unsigned long lastUpdate = 0;
    static unsigned long lastInputCheck = 0;
    unsigned long currentMillis = millis();
    
    if (digitalRead(SWITCH_BUTTON) == LOW) {
        isFlappyBird = !isFlappyBird;
        playIntro();
        delay(500);
    }
    
    if (currentMillis - lastInputCheck >= INPUT_CHECK_INTERVAL) {
        lastInputCheck = currentMillis;
        handleGameInput();
    }
    
    if (currentMillis - lastUpdate >= UPDATE_INTERVAL) {
        lastUpdate = currentMillis;
        isFlappyBird ? flappyBird.run() : snakeGame.run();
    }
}