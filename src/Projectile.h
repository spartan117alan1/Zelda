#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <SFML/Graphics.hpp>

class Projectile {
private:
    sf::RectangleShape shape;
    sf::Vector2f direction;
    float speed; // pixels per second
public:
    Projectile(float startX = 0.f, float startY = 0.f, const sf::Vector2f& dir = sf::Vector2f(0.f, -1.f), float spd = 400.f);
    void update(float deltaTime);
    void render(sf::RenderWindow& window) const;
    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const { return shape.getPosition(); }
};

#endif
