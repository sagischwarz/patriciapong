#include <SPI.h>
#include <Gamebuino.h>

Gamebuino gb;

int paddleHeight = 16;
int paddleWidth = 5;
int centerX = LCDWIDTH / 2;
int centerY = LCDHEIGHT / 2;

struct Paddle {
    int collisions;
    int width;
    int heigt;
    int posX;
    int posY;
    int v;
};

struct Paddle initPaddle(int posX, int posY) {
    Paddle paddle;
    paddle.collisions = 0;
    paddle.heigt = paddleHeight;
    paddle.width = paddleWidth;
    paddle.v = 2;
    paddle.posX = posX;
    paddle.posY = posY;
    return paddle;
}

struct Ball {
    int posX;
    int posY;
    int vX;
    int vY;
    int size;
};

struct Ball initBall() {
    Ball ball;
    ball.posX = LCDWIDTH / 2;
    ball.posY = LCDHEIGHT / 2;
    ball.vX = 1;
    ball.vY = 1;
    ball.size = 6;
    return ball;
}

struct State {
    struct Ball ball;
    struct Paddle leftPaddle;
    struct Paddle rightPaddle;
    int leftScore = 0;
    int rightScore = 0;
};

struct State initState() {
    int paddleYPos = LCDHEIGHT / 2 - paddleHeight / 2;
    State state;
    state.ball = initBall();
    state.leftPaddle = initPaddle(5, paddleYPos);
    state.rightPaddle = initPaddle(LCDWIDTH - 10, paddleYPos);
    return state;
}

struct State gameState;

void printScore() {
    gb.display.fontSize = 1;
    gb.display.cursorX = LCDWIDTH / 2 - 5;
    gb.display.cursorY = 0;

    char buffer[10];
    sprintf(buffer, "%i:%i", gameState.leftScore, gameState.rightScore);
    gb.display.print(buffer);
}

void leftPaddleDown() {
    gameState.leftPaddle.posY += gameState.leftPaddle.v;

    if ((gameState.leftPaddle.posY + gameState.leftPaddle.heigt) > LCDHEIGHT) {
        gameState.leftPaddle.posY = LCDHEIGHT - gameState.leftPaddle.heigt;
    }
}

void leftPaddleUp() {
    gameState.leftPaddle.posY -= gameState.leftPaddle.v;

    if (gameState.leftPaddle.posY < 0) {
        gameState.leftPaddle.posY = 0;
    }
}

void rightPaddleDown() {
    gameState.rightPaddle.posY += gameState.rightPaddle.v;

    if ((gameState.rightPaddle.posY + gameState.rightPaddle.heigt) > LCDHEIGHT) {
        gameState.rightPaddle.posY = LCDHEIGHT - gameState.rightPaddle.heigt;
    }
}

void rightPaddleUp() {
    gameState.rightPaddle.posY -= gameState.rightPaddle.v;

    if (gameState.rightPaddle.posY < 0) {
        gameState.rightPaddle.posY = 0;
    }
}

void resetBall() {
        gameState.ball.posX = centerX;
        gameState.ball.posY = centerY;
}

/*int sign(int x) {
    return (x > 0) - (x < 0);
}

void accelerateBall() {
    gameState.ball.vX += sign(gameState.ball.vX);
    gameState.ball.vY += sign(gameState.ball.vY);
}*/

void leftPlayerScore() {
    if (gameState.ball.posX < 0) {
        resetBall();
        gameState.rightScore += 1;
    }   
}

void rightPlayerScore() {
    if ((gameState.ball.posX + gameState.ball.size) > LCDWIDTH) {
        resetBall();
        gameState.leftScore += 1;
    }
}

void handlePaddleHeight(struct Paddle *p) {
    p->collisions++;
    if (p->heigt > 6 && p->collisions % 3 == 0) {
        p->heigt--;
    }
}

void leftPaddleCollision() {
    if (gameState.ball.posY + gameState.ball.size >= gameState.leftPaddle.posY &&
        gameState.ball.posY <= (gameState.leftPaddle.posY + gameState.leftPaddle.heigt) &&
        gameState.ball.posX <= gameState.leftPaddle.posX + gameState.leftPaddle.width &&
        gameState.ball.vX < 0) {
            gameState.ball.vX *= -1;
            gb.sound.playTick();
            handlePaddleHeight(&gameState.leftPaddle);
    }
}

void rightPaddleCollision() {
    if (gameState.ball.posY + gameState.ball.size >= gameState.rightPaddle.posY &&
        gameState.ball.posY <= (gameState.rightPaddle.posY + gameState.rightPaddle.heigt) &&
        gameState.ball.posX + gameState.ball.size >= gameState.rightPaddle.posX &&
        gameState.ball.vX > 0) {
            gameState.ball.vX *= -1;
            gb.sound.playTick();
            handlePaddleHeight(&gameState.rightPaddle);
    }
}

int currentMaxScore() {
    return max(gameState.leftScore, gameState.rightScore);
}

int ballSpeedFactor() {
    if (currentMaxScore() < 5) {
        return 2;
    } else if (currentMaxScore() < 10) {
        return 3;
    } else {
        return 4;
    }
}

void moveBall() {
    if (gameState.ball.posY < 0) {
        gameState.ball.vY *= -1;
    }

    if ((gameState.ball.posY + gameState.ball.size) > LCDHEIGHT) {
        gameState.ball.vY *= -1;
    }

    leftPaddleCollision();
    rightPaddleCollision();

    gameState.ball.posX += gameState.ball.vX * ballSpeedFactor();
    gameState.ball.posY += gameState.ball.vY * ballSpeedFactor();
}

void drawBall() {
    gb.display.fillRect(gameState.ball.posX, gameState.ball.posY, gameState.ball.size, gameState.ball.size);
}

void drawPaddles() {
    gb.display.fillRect(gameState.leftPaddle.posX, gameState.leftPaddle.posY, gameState.leftPaddle.width, gameState.leftPaddle.heigt);
    gb.display.fillRect(gameState.rightPaddle.posX, gameState.rightPaddle.posY, gameState.rightPaddle.width, gameState.rightPaddle.heigt);
}

void setupGame() {
    gameState = initState();
    gb.begin();
    gb.titleScreen(F("Patricia Pong <3"));
}

void handleInput() {
    if (gb.buttons.pressed(BTN_C)) {
        setupGame();
    }

    if (gb.buttons.repeat(BTN_DOWN,2)) {
        leftPaddleDown();
    }

    if (gb.buttons.repeat(BTN_UP,2)) {
        leftPaddleUp();
    }

    if (gb.buttons.repeat(BTN_A, 2)) {
        rightPaddleDown();
    }

    if (gb.buttons.repeat(BTN_B, 2)) {
        rightPaddleUp();
    }
}

void setup() {
    setupGame();
}

void loop() {
    if (gb.update()){
        printScore();

        handleInput(); 

        moveBall();

        leftPlayerScore();
        rightPlayerScore();

        drawBall();
        drawPaddles();
    }
}
