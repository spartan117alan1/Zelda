#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <SFML/Graphics.hpp>

class Obstacle {
private:
    sf::RectangleShape shape;
public:
    Obstacle(float x=0.f, float y=0.f, float w=32.f, float h=32.f);
    void render(sf::RenderWindow& window) const;
    sf::FloatRect getBounds() const;
};

#endif
