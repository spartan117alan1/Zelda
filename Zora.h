#ifndef ZORA_H
#define ZORA_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "Obstacle.h"
#include "EnemyProjectile.h"

// Los 4 estados de nuestra máquina
enum class ZoraState {
    HIDDEN,    // Bajo el agua (Invisible e invulnerable)
    EMERGING,  // Mostrando burbujas
    ATTACKING, // Asomado y disparando
    DIVING     // Regresando al agua
};

class Zora {
private:
    static sf::Texture texture;
    static bool textureLoaded;

    sf::Sprite sprite;
    sf::Vector2f position;
    ZoraState state;
    
    float stateTimer;
    int health;

     float stunTimer = 0.f;
    bool isStunned = false;


    // Función interna para buscar un bloque azul y moverse ahí
    void teleportToWater(const std::vector<Obstacle>& obstacles);

public:
    Zora(const std::vector<Obstacle>& obstacles);
    
    void update(float deltaTime, const sf::Vector2f& playerPos, std::vector<EnemyProjectile>& projectiles, const std::vector<Obstacle>& obstacles);
    void render(sf::RenderTarget& window) const;
    
    sf::FloatRect getBounds() const;
    void takeDamage(int amount);
    bool isDead() const;

    void applyStun(float duration); // <--- NUEVO
};

#endif