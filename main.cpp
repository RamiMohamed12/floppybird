#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>

class Bird {
private:
    sf::Sprite sprite;
    sf::Texture texture;
    float velocity;
    static constexpr float GRAVITY = 0.5f;
    static constexpr float JUMP_FORCE = -10.0f;

public:
    Bird() : velocity(0) {
        if (!texture.loadFromFile("/home/ramimohamed/Programing/flappybird/assets/bird.png")) {
            std::cout << "Error loading bird texture!" << std::endl;
        }
        sprite.setTexture(texture);
        sprite.setScale(0.1f, 0.1f); // Adjust scale as needed
        sprite.setPosition(100, 300);
    }

    void update() {
        velocity += GRAVITY;
        sprite.move(0, velocity);
    }

    void jump() {
        velocity = JUMP_FORCE;
    }

    sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }
};

class Pipe {
private:
    sf::Sprite topPipe;
    sf::Sprite bottomPipe;
    static sf::Texture pipeTexture;
    bool passed;

public:
    static constexpr float PIPE_SPEED = 3.0f;
    static constexpr float GAP_SIZE = 200.0f;

    static bool loadTexture() {
        return pipeTexture.loadFromFile("/home/ramimohamed/Programing/flappybird/assets/pipe.png");
    }

    Pipe(float xPos) : passed(false) {
        float randomY = rand() % 300 + 100;

        topPipe.setTexture(pipeTexture);
        bottomPipe.setTexture(pipeTexture);

        // Scale pipes as needed
        topPipe.setScale(0.2f, 0.5f);
        bottomPipe.setScale(0.2f, 0.5f);

        // Rotate top pipe 180 degrees
        topPipe.setRotation(180.f);

        topPipe.setPosition(xPos, randomY);
        bottomPipe.setPosition(xPos, randomY + GAP_SIZE);
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

    void draw(sf::RenderWindow& window) {
        window.draw(topPipe);
        window.draw(bottomPipe);
    }
};

sf::Texture Pipe::pipeTexture;

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Flappy Bird");
    window.setFramerateLimit(60);

    if (!Pipe::loadTexture()) {
        std::cout << "Error loading pipe texture!" << std::endl;
        return -1;
    }

    Bird bird;
    std::vector<Pipe> pipes;
    int score = 0;
    bool gameOver = false;

    sf::Font font;
    if (!font.loadFromFile("/home/ramimohamed/.local/share/fonts/Ubuntu-C.ttf")) {
        std::cout << "Error loading font!" << std::endl;
        return -1;
    }

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(30);
    scoreText.setPosition(10, 10);

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
        }

        if (!gameOver) {
            float dt = clock.restart().asSeconds();
            timeSinceLastPipe += dt;

            bird.update();

            if (timeSinceLastPipe > 2.f) {
                pipes.emplace_back(800.f);
                timeSinceLastPipe = 0;
            }

            for (auto& pipe : pipes) {
                pipe.update();
                if (pipe.checkCollision(bird)) {
                    gameOver = true;
                }
            }

            pipes.erase(
                std::remove_if(pipes.begin(), pipes.end(),
                    [](const Pipe& pipe) { return pipe.isOffscreen(); }),
                pipes.end()
            );
        }

        window.clear(sf::Color(135, 206, 235));

        for (auto& pipe : pipes) {
            pipe.draw(window);
        }

        bird.draw(window);
        scoreText.setString("Score: " + std::to_string(score));
        window.draw(scoreText);

        window.display();
    }

    return 0;
}

