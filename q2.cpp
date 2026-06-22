#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <fstream>
#include <cmath>

using namespace sf;
using namespace std;

const int WIDTH = 800;
const int HEIGHT = 600;
const float GRAVITY = 900.f;
const float JUMP_FORCE = -350.f;
const float PIPE_SPEED = 200.f;
const float PIPE_WIDTH = 80.f;
const float GAP_HEIGHT = 180.f;
const float HITBOX_MARGIN = 10.f;

// GAME STATE DIFFICULTY
enum GameState { MENU, PLAYING };
GameState state = MENU;
int difficulty = 1; 
float currentGap = GAP_HEIGHT;
float oscillationAmplitude = 0.f;
float oscillationSpeed = 0.f;

void applyDifficulty(int level) {
    switch (level) {
        case 0:
            currentGap = 220.f;
            oscillationAmplitude = 0.f;
            oscillationSpeed = 0.f;
            break;
        case 1:
            currentGap = 180.f;
            oscillationAmplitude = 30.f;
            oscillationSpeed = 2.f;
            break;
        case 2:
            currentGap = 140.f;
            oscillationAmplitude = 60.f;
            oscillationSpeed = 3.5f;
            break;
    }
}

struct Pipe {
    float x;
    float gapY;
    bool passed;
    float baseY;

    float phase;
    float speed;
    float amplitude;
};

int main() {
    srand(time(0));

    RenderWindow window(VideoMode(WIDTH, HEIGHT), "Flappy Bird SFML");
    window.setFramerateLimit(60);

    applyDifficulty(difficulty);

    Texture birdTexture, pipeTexture;
    if (!birdTexture.loadFromFile("graphics/bird.png") ||
        !pipeTexture.loadFromFile("graphics/pipes.png")) {
        return -1;
    }

    SoundBuffer jumpBuffer, hitBuffer, overBuffer;
    jumpBuffer.loadFromFile("sound/faaah.wav");
    hitBuffer.loadFromFile("sound/anime-ahh.wav");
    overBuffer.loadFromFile("sound/Dark souls 3 boss music.wav");

    Sound jumpSound(jumpBuffer);
    Sound hitSound(hitBuffer);
    Sound overSound(overBuffer);

    Sprite bird(birdTexture);
    bird.setPosition(200, HEIGHT / 2);
    bird.setScale(0.5f, 0.5f);

    float velocity = 0;
    vector<Pipe> pipes;
    float spawnTimer = 0;
    Font font;
    font.loadFromFile("graphics/KOMIKAP_.ttf");

    Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(30);
    scoreText.setPosition(20, 20);

    Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(50);
    gameOverText.setString("GAME OVER\nPress Space");
    gameOverText.setPosition(200, 200);

    Text menuText;
    menuText.setFont(font);
    menuText.setCharacterSize(40);
    menuText.setPosition(120, 150);
    menuText.setString(
        "FLAPPY BIRD\n\n"
        "1 - EASY\n"
        "2 - MEDIUM\n"
        "3 - HARD\n\n"
        "Press ENTER to Start"
    );

    int score = 0;
    int highScore = 0;
    bool gameOver = false;

    ifstream highScoreFile("highscore.txt");
    if (highScoreFile.is_open()) {
        highScoreFile >> highScore;
        highScoreFile.close();
    }

    Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::KeyPressed) {

                if (state == MENU) {

                    if (event.key.code == Keyboard::Num1) {
                        difficulty = 0;
                        applyDifficulty(difficulty);
                    }
                    if (event.key.code == Keyboard::Num2) {
                        difficulty = 1;
                        applyDifficulty(difficulty);
                    }
                    if (event.key.code == Keyboard::Num3) {
                        difficulty = 2;
                        applyDifficulty(difficulty);
                    }

                    if (event.key.code == Keyboard::Enter) {
                        state = PLAYING;

                        pipes.clear();
                        bird.setPosition(200, HEIGHT / 2);
                        velocity = 0;
                        score = 0;
                        gameOver = false;
                    }
                }

                else if (state == PLAYING) {

                    if (event.key.code == Keyboard::Space) {
                        if (gameOver) {
                            state = MENU; 
                        } else {
                            velocity = JUMP_FORCE;
                            jumpSound.play();
                        }
                    }
                }
            }
        }

        if (state == PLAYING && !gameOver) {

            velocity += GRAVITY * dt;
            bird.move(0, velocity * dt);

            spawnTimer += dt;
            if (spawnTimer > 2.f) {
                spawnTimer = 0;

                Pipe p;
                p.x = WIDTH;
                p.baseY = 100 + rand() % (HEIGHT - 200);
                p.gapY = p.baseY;
                p.passed = false;

                p.phase = (rand() % 360) * 3.14159f / 180.f;
                p.speed = oscillationSpeed * (0.5f + (rand() % 100) / 100.f);
                p.amplitude = oscillationAmplitude * (0.7f + (rand() % 60) / 100.f);

                pipes.push_back(p);
            }

            for (auto &p : pipes) {
                p.x -= PIPE_SPEED * dt;

                if (p.amplitude > 0) {
                    p.gapY = p.baseY +
                        sin(clock.getElapsedTime().asSeconds() * p.speed + p.phase)
                        * p.amplitude;
                }

                if (!p.passed && p.x < bird.getPosition().x) {
                    p.passed = true;
                    score++;
                }
            }

            FloatRect birdBounds = bird.getGlobalBounds();
            birdBounds.left += HITBOX_MARGIN;
            birdBounds.top += HITBOX_MARGIN;
            birdBounds.width -= HITBOX_MARGIN * 2;
            birdBounds.height -= HITBOX_MARGIN * 2;

            for (auto &p : pipes) {
                float topHeight = p.gapY - currentGap / 2;
                float bottomY = p.gapY + currentGap / 2;
                float bottomHeight = HEIGHT - bottomY;

                FloatRect topRect(p.x + HITBOX_MARGIN, 0,
                    PIPE_WIDTH - HITBOX_MARGIN * 2,
                    topHeight - HITBOX_MARGIN);

                FloatRect bottomRect(p.x + HITBOX_MARGIN,
                    bottomY + HITBOX_MARGIN,
                    PIPE_WIDTH - HITBOX_MARGIN * 2,
                    bottomHeight - HITBOX_MARGIN);

                if (birdBounds.intersects(topRect) || birdBounds.intersects(bottomRect)) {
                    gameOver = true;
                    hitSound.play();
                    overSound.play();
                }
            }

            if (bird.getPosition().y < 0 || bird.getPosition().y > HEIGHT) {
                gameOver = true;
                hitSound.play();
                overSound.play();
            }
        }

        if (score > highScore) {
            highScore = score;
            ofstream highScoreFile("highscore.txt");
            if (highScoreFile.is_open()) {
                highScoreFile << highScore;
                highScoreFile.close();
            }
        }

        stringstream ss;
        string diffName = (difficulty == 0) ? "EASY" : (difficulty == 1) ? "MEDIUM" : "HARD";
        ss << "Score: " << score << "  High Score: " << highScore << " [" << diffName << "]";
        scoreText.setString(ss.str());

        window.clear(Color(100, 200, 255));

        if (state == MENU) {
            window.draw(menuText);
        }
        else if (state == PLAYING) {

            for (auto &p : pipes) {
                Sprite topPipe(pipeTexture);
                Sprite bottomPipe(pipeTexture);

                float texW = pipeTexture.getSize().x;
                float texH = pipeTexture.getSize().y;

                float topHeight = p.gapY - currentGap / 2;
                float bottomY = p.gapY + currentGap / 2;
                float bottomHeight = HEIGHT - bottomY;

                topPipe.setPosition(p.x, topHeight);
                topPipe.setScale(PIPE_WIDTH / texW, -topHeight / texH);

                bottomPipe.setPosition(p.x, bottomY);
                bottomPipe.setScale(PIPE_WIDTH / texW, bottomHeight / texH);

                window.draw(topPipe);
                window.draw(bottomPipe);
            }

            window.draw(bird);
            window.draw(scoreText);

            if (gameOver) {
                window.draw(gameOverText);
            }
        }

        window.display();
    }

    return 0;
}