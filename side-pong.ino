#include <Esplora.h>
#include <SPI.h>
#include <TFT.h>

struct Ball {
    int x, y, prevX, prevY,
        xDirection = 1, yDirection,
        stepX = 1, stepY = 1,
        r = 0, g = 0, b = 0,
        size = 2, radius = size / 2;
};

struct GameProps {
    boolean ballTouchedBorder = false, ballReleased = false, resetClicked = false;
    int maxX, maxY, maxMovingStep = 3;
};

struct Screen {
    int r = 255, g = 255, b = 0;
    int width, height;
};

GameProps *gameProps = new GameProps();
Screen *screen = new Screen();
Ball *ball = new Ball();

void setup() {
    // initialize the screen gameProps and ball
    EsploraTFT.begin();
    Serial.begin(9600);
    EsploraTFT.background(screen->r, screen->g, screen->b);
    
    screen->width = EsploraTFT.width();
    screen->height = EsploraTFT.height();
    
    gameProps->maxX = screen->width - ball->size,
    gameProps->maxY = screen->height - ball->size;
    
    ball->prevX = ball->x = screen->width/2;
    ball->prevY = ball->y = screen->height/2;

    // initial point render
    _drawPoint(ball->prevX, ball->prevY, ball->size, ball->r, ball->g, ball->b);
}

void loop() {

    // Case for game restart
    if ( Esplora.readButton(SWITCH_DOWN) == LOW ) {
        gameProps->ballReleased = false;
    }
    
    if ( !gameProps->ballReleased ) {
        // ball not released -> move by joystick

        ball->x = map(Esplora.readJoystickX(), 512, -512, ball->size, screen->width);
        ball->y = map(Esplora.readJoystickY(), -512, 512, ball->size, screen->height + 1);

        ball->y = maxN(ball->size, ball->y - ball->size);
        ball->y = minN(gameProps->maxY, ball->y);

        ball->x = maxN(ball->size, ball->x - ball->size);
        ball->x = minN(gameProps->maxX, ball->x);

        if ( !_handleCollision() ) {

            if ( gameProps->ballTouchedBorder ) {
                // border was touched before
                
                // set ball step
                ball->stepX = map(absN( ball->x - screen->width / 2 ), ball->size, screen->width / 2, 0, gameProps->maxMovingStep);
                ball->stepY = map(absN( ball->y - screen->height / 2 ), ball->size, screen->height / 2, 0, gameProps->maxMovingStep);

                Serial.println(ball->stepX);

                // set ball direction of moving
                ball->xDirection = 1;
                ball->yDirection = 1;

                if ( ball->x >= screen->width / 2 ) {
                    ball->xDirection = -1;
                }

                if ( ball->y >= screen->height / 2 ) {
                    ball->yDirection = -1;
                }

                gameProps->ballReleased = true;
            }

            gameProps->ballTouchedBorder = false;
            _stopNotification();

        } else {
            gameProps->ballReleased = false;
            gameProps->ballTouchedBorder = true;
            _playNotification();
        }

    } else {
        // ball released -> move by joystick

        if ( _handleCollision() ) {
            gameProps->ballTouchedBorder = true;
            _playNotification();
        } else {
            gameProps->ballTouchedBorder = false;
            _stopNotification();
        }
        
        ball->x = ball->prevX + ( ( ball->stepX ) * ball->xDirection );
        ball->y = ball->prevY + ( ( ball->stepY ) * ball->yDirection );
    }
    
    // Check if redraw of ball is needed
    if ( gameProps->ballReleased || absN(ball->x - ball->prevX) > ball->size || absN(ball->y - ball->prevY) > ball->size ) {   
        // remove the old point
        _drawPoint(ball->prevX, ball->prevY, ball->size, screen->r, screen->g, screen->b);
    
        // draw the new point
        _drawPoint(ball->x, ball->y, ball->size, ball->r, ball->g, ball->b);
    
        ball->prevX = ball->x;
        ball->prevY = ball->y;
    }
    
    if ( gameProps->ballReleased ) {
        delay(5);
    }
}

boolean _handleCollision() {

    boolean xTurn = ( ball->x <= ball->size || ball->x >= gameProps->maxX ),
            yTurn = ( ball->y <= ball->size || ball->y >= gameProps->maxY );
    
    if ( xTurn ){
        ball->xDirection = ball->xDirection * -1;
    }
    
    if ( yTurn ){
        ball->yDirection = ball->yDirection * -1;
    }
    
    if ( xTurn || yTurn ) {
        return true;
    }
    
    return false;
}

void _playNotification () {
    Esplora.writeRGB(0, 20, 0);
    Esplora.tone(Esplora.readSlider(), 10);
}

void _stopNotification () {
    Esplora.writeRGB(0, 0, 0);
    Esplora.noTone();
}

void _drawPoint(int x, int y, int pointSize, int r, int g, int b) {
    EsploraTFT.fill(r, g, b);
    EsploraTFT.circle(x, y, pointSize);
}

/* Math helper functions */

int minN (int a, int b) {
    if ( a < b ) {
        return a;
    }
    
    return b;
}

int maxN (int a, int b) {
    if ( a > b ) {
        return a;
    }
    
    return b;
}

int absN (int a) {
    if (a < 0) {
        a = -1 *a;
    }
    
    return a;
}

