#ifndef ENEMY_H
#define ENEMY_H

#include <SFML/Graphics.hpp>
#include "Obstacle.h"
#include "Hud.h"
#include <vector>
#include <cmath>
#include "EnemyProjectile.h"


class Enemy {
private:
    //texture and sprite for the enemy
    sf::RectangleShape shape;
    static sf::Texture texture;
    sf::Sprite sprite;
    static bool textureLoaded;

    float speed;
    float attackCooldown;
    float attackTimer;
    int health;

    float stunTimer = 0.f;
    bool isStunned = false;


public:
    Enemy(float x, float y);
    void update(float deltaTime, const sf::Vector2f& playerPos, const std::vector<Obstacle>& obstacles, const std::vector<Hud>& hud);
    void render(sf::RenderTarget& window) const;
    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const;
    void move(const sf::Vector2f& offset);
    bool canAttack();
    void takeDamage(int amount);
    bool isDead() const;

    void applyStun(float duration); // <--- NUEVO
};

#endif
