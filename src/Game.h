#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Player.h"
#include <vector>

//declaraciones implicitas
#include "Obstacle.h"
#include "Enemy.h"
#include "Zora.h"              // Asegura que Game conoce a Zora
#include "EnemyProjectile.h"   // Asegura que Game conoce los proyectiles

struct Level{
        std::vector<Obstacle> obstacles;
        std::vector<Enemy> enemies;
        sf::Vector2f playerStart;
        sf::Vector2f entryFromLeft;
        sf::Vector2f entryFromRight;
        sf::Vector2f entryFromTop;
        sf::Vector2f entryFromBottom;
    };


class Game {
private:
    sf::RenderWindow window;
    Player player;
    std::vector<Enemy> enemies;
    std::vector<Zora> zoras;
    std::vector<EnemyProjectile> enemyProjectiles;
    std::vector<Obstacle> obstacles;
    std::vector<Hud> hud;
    sf::Clock clock;

    std::vector<Level> levels;
    int currentLevel;

    // Audio Centralizado
    sf::Music fieldMusic;
    
    sf::SoundBuffer rupeeBuffer;
    sf::SoundBuffer lowHealthBuffer;
    sf::SoundBuffer healBuffer;
    sf::SoundBuffer swordBuffer;
    sf::SoundBuffer damageBuffer;

    sf::Sound rupeeSound;
    sf::Sound lowHealthSound;
    sf::Sound healSound;
    sf::Sound swordSound;
    sf::Sound damageSound;

public:
    Game();
    void initLevel();
    void run();
    void resetGame();
    void loadLevel(int index);
    void loadLevelsFromJSON();
};

#endif
