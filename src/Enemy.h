#ifndef ENEMY_H
#define ENEMY_H

#include <SFML/Graphics.hpp>
#include "Obstacle.h"
#include "Hud.h"
#include <vector>
#include <cmath>

class Enemy {
private:
    sf::RectangleShape shape;
    float speed;
    float attackCooldown;
    float attackTimer;
    int health;
public:
    Enemy(float x, float y);
    void update(float deltaTime, const sf::Vector2f& playerPos, const std::vector<Obstacle>& obstacles, const std::vector<Hud>& hud);
    void render(sf::RenderWindow& window) const;
    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const;
    void move(const sf::Vector2f& offset);
    bool canAttack();
    void takeDamage(int amount);
    bool isDead() const;
};

#endif
