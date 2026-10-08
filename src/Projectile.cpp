#include "Projectile.h"

Projectile::Projectile(float startX, float startY, const sf::Vector2f& dir, float spd)
: direction(dir), speed(spd) {
    shape.setSize({10.f,10.f});
    shape.setFillColor(sf::Color::Yellow);
    shape.setOrigin(shape.getSize() / 2.f);
    shape.setPosition(startX, startY);
}

void Projectile::update(float deltaTime) {
    shape.move(direction * speed * deltaTime);
}

void Projectile::render(sf::RenderWindow& window) const {
    window.draw(shape);
}

sf::FloatRect Projectile::getBounds() const {
    return shape.getGlobalBounds();
}
