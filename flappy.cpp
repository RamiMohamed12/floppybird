#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <iostream>
#include <string>

class Bird {
public:
    sf::CircleShape shape;
    float velocity;
    static constexpr float GRAVITY = 0.5f;
    static constexpr float JUMP_FORCE = -10.0f;

    Bird() : velocity(0) {
        shape.setRadius(20.f);
        shape.setFillColor(sf::Color::Yellow);
        shape.setPosition(100, 300);
    }

    void update() {
        velocity += GRAVITY;
        shape.move(0, velocity);
    }

    void jump() {
        velocity = JUMP_FORCE;
    }

    sf::FloatRect getBounds() const {
        return shape.getGlobalBounds();
    }
};

class Pipe {
public:
    sf::RectangleShape topPipe;
    sf::RectangleShape bottomPipe;
    bool passed;
    static constexpr float PIPE_SPEED = 3.0f;
    static constexpr float GAP_SIZE = 200.0f;

    Pipe(float xPos) : passed(false) {
        float randomY = rand() % 300 + 100;

        topPipe.setSize(sf::Vector2f(50, randomY));
        bottomPipe.setSize(sf::Vector2f(50, 600 - randomY - GAP_SIZE));
        
        topPipe.setPosition(xPos, 0);
        bottomPipe.setPosition(xPos, randomY + GAP_SIZE);
        
        topPipe.setFillColor(sf::Color::Green);
        bottomPipe.setFillColor(sf::Color::Green);
    }

    void update() {
        topPipe.move(-PIPE_SPEED, 0);
        bottomPipe.move(-PIPE_SPEED, 0);
    }

    bool isOffscreen() const {
        return topPipe.getPosition().x < -50.f;
    }

    bool checkCollision(const Bird& bird) const {
        return bird.getBounds().intersects(topPipe.getGlobalBounds()) ||
               bird.getBounds().intersects(bottomPipe.getGlobalBounds());
    }
};


#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>

unsigned int computeHash(int score) {
    std::hash<std::string> hasher;
    return hasher(std::to_string(score));
}

// Load high score and hash from file
int loadHighScore() {
    std::ifstream inFile("highscore.txt");
    if (!inFile.is_open()) {
        std::cerr << "No highscore file found. Creating a new one." << std::endl;
        return 0;
    }

    int savedScore;
    unsigned int savedHash;
    inFile >> savedScore >> savedHash;
    inFile.close();

    // Validate the hash
    if (computeHash(savedScore) != savedHash) {
        std::cerr << "Highscore file tampered with. Resetting highscore to 0." << std::endl;
        return 0;
    }

    return savedScore;
}

// Save high score and its hash to file
void saveHighScore(int score) {
    unsigned int scoreHash = computeHash(score);
    std::ofstream outFile("highscore.txt");
    outFile << score << " " << scoreHash;
    outFile.close();
}


int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Flappy Bird");
    window.setFramerateLimit(60);

    Bird bird;
    std::vector<Pipe> pipes;
    int score = 0;
    int highscore = loadHighScore(); // Load highscore
    bool gameOver = false;

    // Font setup
    sf::Font font;
    if (!font.loadFromFile("/home/ramimohamed/.local/share/fonts/UbuntuMono-BI.ttf")) {
        std::cerr << "Error loading font" << std::endl;
        return -1;
    }

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(30);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(10, 10);

    sf::Text highscoreText;
    highscoreText.setFont(font);
    highscoreText.setCharacterSize(30);
    highscoreText.setFillColor(sf::Color::White);
    highscoreText.setPosition(10, 50);

    sf::Clock clock;
    float timeSinceLastPipe = 0.f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Space && !gameOver) {
                bird.jump();
            }

            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::R && gameOver) {
                // Reset game
                gameOver = false;
                score = 0;
                pipes.clear();
                bird = Bird();
            }
        }

        if (!gameOver) {
            float dt = clock.restart().asSeconds();
            timeSinceLastPipe += dt;

            // Update bird
            bird.update();

            // Create new pipes
            if (timeSinceLastPipe > 2.f) {
                pipes.emplace_back(800.f);
                timeSinceLastPipe = 0;
            }

            // Update pipes
            for (auto& pipe : pipes) {
                pipe.update();

                // Score counting
                if (!pipe.passed && pipe.topPipe.getPosition().x < bird.shape.getPosition().x) {
                    score++;
                    pipe.passed = true;
                }

                // Collision detection
                if (pipe.checkCollision(bird)) {
                    gameOver = true;
                }
            }

            // Remove offscreen pipes
            pipes.erase(
                std::remove_if(pipes.begin(), pipes.end(),
                               [](const Pipe& pipe) { return pipe.isOffscreen(); }),
                pipes.end()
            );

            // Check if bird hits ground or ceiling
            if (bird.shape.getPosition().y < 0 || bird.shape.getPosition().y > 600) {
                gameOver = true;
            }

            // Update highscore if needed
            if (score > highscore) {
                highscore = score;
                saveHighScore(highscore);
            }
        }

        // Update score display
        scoreText.setString("Score: " + std::to_string(score));
        highscoreText.setString("Highscore: " + std::to_string(highscore));

        // Render
        window.clear(sf::Color(135, 206, 235)); // Sky blue background

        // Draw pipes
        for (const auto& pipe : pipes) {
            window.draw(pipe.topPipe);
            window.draw(pipe.bottomPipe);
        }

        window.draw(bird.shape);
        window.draw(scoreText);
        window.draw(highscoreText);

        if (gameOver) {
            sf::Text gameOverText;
            gameOverText.setFont(font);
            gameOverText.setString("Game Over!\nPress R to restart");
            gameOverText.setCharacterSize(40);
            gameOverText.setFillColor(sf::Color::White);
            gameOverText.setPosition(
                400 - gameOverText.getGlobalBounds().width / 2,
                300 - gameOverText.getGlobalBounds().height / 2
            );
            window.draw(gameOverText);
        }

        window.display();
    }

    return 0;
}

