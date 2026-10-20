#include "Zora.h"
#include <cstdlib> // Para std::rand()
#include <cmath>
#include <iostream>

sf::Texture Zora::texture;
bool Zora::textureLoaded = false;

Zora::Zora(const std::vector<Obstacle>& obstacles) 
    : state(ZoraState::HIDDEN), stateTimer(0.f), health(2) 
{
    if (!textureLoaded) {
        if (texture.loadFromFile("assets/textures/enemy.png")) {
            textureLoaded = true;
        }
    }
    
    if (textureLoaded) {
        sprite.setTexture(texture);
        sprite.setScale(2.f, 2.f);
        sprite.setOrigin(8.f, 8.f);
    }
    
    // Al nacer, busca un lugar en el agua
    teleportToWater(obstacles);
}

void Zora::teleportToWater(const std::vector<Obstacle>& obstacles) {
    std::vector<sf::FloatRect> waterBlocks;
    
    // 1. Encontrar todos los lagos
    for (const auto& obs : obstacles) {
        if (obs.getIsWater()) {
            waterBlocks.push_back(obs.getBounds());
        }
    }

    // 2. Elegir uno al azar y teletransportarse dentro de él
    if (!waterBlocks.empty()) {
        int index = std::rand() % waterBlocks.size();
        sf::FloatRect water = waterBlocks[index];
        
        float margin = 16.f; // Para no aparecer pegado a la orilla
        float rx = water.left + margin + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / (water.width - margin * 2)));
        float ry = water.top + margin + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / (water.height - margin * 2)));
        
        position = sf::Vector2f(rx, ry);
        sprite.setPosition(position);
    }
}
void Zora::applyStun(float duration) {
    if (duration > stunTimer) {
        stunTimer = duration;
    }
}

void Zora::update(float deltaTime, const sf::Vector2f& playerPos, std::vector<EnemyProjectile>& projectiles, const std::vector<Obstacle>& obstacles) {
    
    // --- LÓGICA DE STUN / CONGELACIÓN ---
    if (stunTimer > 0.f) {
        stunTimer -= deltaTime;
        isStunned = true;
        if (textureLoaded) {
            sprite.setColor(sf::Color::Cyan); // Teñir de azul mientras esté aturdido
        }
        return; // Detiene la IA y el movimiento por completo
    } else {
        isStunned = false;
        if (textureLoaded) {
            sprite.setColor(sf::Color::White); // Regresa al color normal
        }
    }
    
    stateTimer += deltaTime;

    switch (state) {
        case ZoraState::HIDDEN:
            // Espera entre 2 y 4 segundos antes de salir
            if (stateTimer > 2.0f + (std::rand() % 3)) { 
                state = ZoraState::EMERGING;
                stateTimer = 0.f;
                teleportToWater(obstacles); // Cambia de lugar mágicamente
                sprite.setTextureRect(sf::IntRect(189, 11, 16, 16)); // Burbuja 1
            }
            break;

        case ZoraState::EMERGING:
            if (stateTimer > 0.4f) {
                sprite.setTextureRect(sf::IntRect(206, 11, 16, 16)); // Burbuja 2
            }
            if (stateTimer > 0.8f) {
                state = ZoraState::ATTACKING;
                stateTimer = 0.f;
                
                // Decide si asoma su cara (223) o su espalda (240) dependiendo de dónde está Link
                if (playerPos.y > position.y) {
                    sprite.setTextureRect(sf::IntRect(223, 11, 16, 16)); // Frente
                } else {
                    sprite.setTextureRect(sf::IntRect(240, 11, 16, 16)); // Espalda
                }

                // --- DISPARO DEL PROYECTIL ---
                sf::Vector2f dir = playerPos - position;
                float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (dist > 0) dir /= dist; 
                
                // Dispara la bola mágica apuntando directo a Link
                projectiles.emplace_back(
                    texture, 
                    sf::IntRect(257, 11, 8, 16), // El sprite de la magia
                    position, 
                    dir * 200.f // Velocidad del proyectil
                );
            }
            break;

        case ZoraState::ATTACKING:
            if (stateTimer > 1.5f) { // Se queda asomado 1.5 segundos
                state = ZoraState::DIVING;
                stateTimer = 0.f;
                sprite.setTextureRect(sf::IntRect(206, 11, 16, 16)); // Regresa a Burbuja 2
            }
            break;

        case ZoraState::DIVING:
            if (stateTimer > 0.4f) {
                sprite.setTextureRect(sf::IntRect(189, 11, 16, 16)); // Burbuja 1
            }
            if (stateTimer > 0.8f) {
                state = ZoraState::HIDDEN;
                stateTimer = 0.f;
            }
            break;
    }
}

void Zora::render(sf::RenderTarget& window) const {
    if (state != ZoraState::HIDDEN && textureLoaded) {
        window.draw(sprite);
    }
}

sf::FloatRect Zora::getBounds() const {
    // Si está escondido bajo el agua, es intocable
    if (state == ZoraState::HIDDEN) return sf::FloatRect(0, 0, 0, 0); 
    return sprite.getGlobalBounds();
}

void Zora::takeDamage(int amount) {
    if (state != ZoraState::HIDDEN) {
        health -= amount;
    }
}

bool Zora::isDead() const {
    return health <= 0;
}