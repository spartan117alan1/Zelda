#ifndef ENEMYPROJECTILE_H
#define ENEMYPROJECTILE_H

#include <SFML/Graphics.hpp>

class EnemyProjectile {
protected: // Protegido para permitir herencia (Polimorfismo) a futuro
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Sprite sprite;

public:
    EnemyProjectile(const sf::Texture& tex, sf::IntRect rect, sf::Vector2f startPos, sf::Vector2f vel);
    
    virtual ~EnemyProjectile() = default; // Fundamental para herencia segura
    
    virtual void update(float deltaTime);
    virtual void render(sf::RenderTarget& window) const;
    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const { return position; }
};

#endif