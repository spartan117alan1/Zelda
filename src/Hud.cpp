#include "Hud.h"

Hud::Hud(float x, float y, float w, float h) {
    shape.setSize({w,h});
    shape.setFillColor(sf::Color(0,0,0));
    shape.setPosition(x,y);
}

void Hud::render(sf::RenderWindow& window) const { window.draw(shape); }

sf::FloatRect Hud::getBounds() const { return shape.getGlobalBounds(); }
