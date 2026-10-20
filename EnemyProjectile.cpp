#include "EnemyProjectile.h"

EnemyProjectile::EnemyProjectile(const sf::Texture& tex, sf::IntRect rect, sf::Vector2f startPos, sf::Vector2f vel)
    : position(startPos), velocity(vel) {
    sprite.setTexture(tex);
    sprite.setTextureRect(rect);
    sprite.setScale(2.f, 2.f);
    // Centramos el origen dinámicamente según el tamaño del recorte
    sprite.setOrigin(rect.width / 2.f, rect.height / 2.f);
}

void EnemyProjectile::update(float deltaTime) {
    position += velocity * deltaTime;
    sprite.setPosition(position);
}

void EnemyProjectile::render(sf::RenderTarget& window) const {
    window.draw(sprite);
}

sf::FloatRect EnemyProjectile::getBounds() const {
    return sprite.getGlobalBounds();
}