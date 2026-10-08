#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "Projectile.h"
#include "Obstacle.h"
#include "Hud.h"
#include "Enemy.h"

class Player {
private:
    // Hitbox
    sf::RectangleShape shape;

    // Movement
    sf::Vector2f direction;
    sf::Vector2f lastDirection;
    sf::Vector2f knockback;
    float knockbackTimer;
    float knockbackDuration;
    float speed;
    bool moving;

    //HUD
    // Health
    int health;
    int maxHealth;
    bool isAlive;

    // Score
    int score = 0;

    // Sword attack
    float swordCooldown;
    float swordTimer;
    bool attacking;
    sf::RectangleShape swordHitbox;

    // Projectiles
    std::vector<Projectile> projectiles;
    float shootCooldown;
    float shootTimer;
    bool canShootProjectile;

    // Damage / Invulnerability
    bool isInvulnerable;
    float invulnerableTimer;
    float invulnerableDuration;
    sf::Color normalColor;

    // Sprite system
    sf::Texture texture;
    sf::Sprite sprite;
    sf::Sprite swordSprite;
    int frameX;
    int frameY;
    float animationTimer;
    float animationSpeed;
    int spriteWidth;
    int spriteHeight;
    int currentFrame;
    bool textureLoaded;

    

public:
    Player(float x = 100.f, float y = 100.f);

    void handleInput(float deltaTime);
    void update(float deltaTime,
                const sf::RenderWindow& window,
                const std::vector<Obstacle>& obstacles,
                const std::vector<Hud>& hud,
                std::vector<Enemy>& enemies);

    void render(sf::RenderWindow& window);

    // Positioning
    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition() const { return shape.getPosition(); }

    // Attacks
    void shoot();
    void attack();
    bool isAttacking() const;
    sf::FloatRect getSwordBounds() const;

    // Damage
    void takeDamage(int amount);
    void heal(int amount);
    bool canTakeDamage() const;

    // Knockback
    void applynockback(const sf::Vector2f& dir);

    // Projectiles access
    std::vector<Projectile>& getProjectiles() { return projectiles; }

    // Status
    bool GetIsAlive() const { return isAlive; }
    int getHearts() const { return health; }
    
    //Score
    int getScore() const { return score; }
    void addScore(int points) { score += points; }

    // Hitbox getter
    sf::FloatRect getBounds() const { return shape.getGlobalBounds(); }
};

#endif
