#ifndef HUD_H
#define HUD_H

#include <SFML/Graphics.hpp>

class Hud{
    private:
    sf::RectangleShape shape;
    public:
    Hud(float x=0.f, float y=0.f, float w=32.f, float h=32.f);
    void render(sf::RenderWindow& window) const;
    sf::FloatRect getBounds() const;
};

#endif