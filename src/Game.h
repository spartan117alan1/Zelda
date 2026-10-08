#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include "Player.h"
#include <vector>

struct Level{
        std::vector<Obstacle> obstacles;
        std::vector<Enemy> enemies;
        sf::Vector2f playerStart;
        sf::Vector2f entryFromLeft;
        sf::Vector2f entryFromRight;
    };


class Game {
private:
    sf::RenderWindow window;
    Player player;
    std::vector<Enemy> enemies;
    std::vector<Obstacle> obstacles;
    std::vector<Hud> hud;
    sf::Clock clock;

    std::vector<Level> levels;
    int currentLevel;

public:
    Game();
    void initLevel();
    void run();
    void resetGame();
    void loadLevel(int index);
    void loadLevelsFromJSON();
};

#endif
