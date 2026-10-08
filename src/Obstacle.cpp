#include "Obstacle.h"

Obstacle::Obstacle(float x, float y, float w, float h) {
    shape.setSize({w,h});
    shape.setFillColor(sf::Color(103, 163, 60));
    shape.setPosition(x,y);
}

void Obstacle::render(sf::RenderWindow& window) const { window.draw(shape); }

sf::FloatRect Obstacle::getBounds() const { return shape.getGlobalBounds(); }
