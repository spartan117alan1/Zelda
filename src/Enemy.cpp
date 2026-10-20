#include "Enemy.h"
#include <cmath>
#include <iostream>

using namespace std;
// INICIALIZACIÓN ESTÁTICA (Pon esto al inicio del archivo, antes del constructor)
sf::Texture Enemy::texture;
bool Enemy::textureLoaded = false;

Enemy::Enemy(float x, float y)
    : speed(70.f), attackCooldown(1.5f), attackTimer(0.0f), health(2)
{
    shape.setSize(sf::Vector2f(32.f, 32.f));
    shape.setFillColor(sf::Color(245, 140, 10));
    shape.setOrigin(shape.getSize() / 2.f);
    shape.setPosition(x, y);

   // NUEVO: Solo carga la textura una vez en toda la vida del juego
    if (!textureLoaded) {
        if (texture.loadFromFile("assets/textures/enemy.png")) {
            textureLoaded = true;
        }
        else {
            std::cout << "Error: no se pudo cargar assets/textures/enemy.png\n";
        }
    }

    // Configurar el sprite si la textura estática existe
    if (textureLoaded) {
        sprite.setTexture(texture);
        sprite.setTextureRect(sf::IntRect(1, 11, 16, 16)); 
        sprite.setScale(2.f, 2.f);
        sprite.setOrigin(8.f, 8.f);
        
        
    }

}

void Enemy::applyStun(float duration) {
    if (duration > stunTimer) {
        stunTimer = duration;
    }
}

void Enemy::update(float deltaTime, const sf::Vector2f& playerPos, const std::vector<Obstacle>& obstacles, const std::vector<Hud>& hud) {

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
    
    attackTimer += deltaTime;

    

    // --- Movimiento hacia el jugador ---
    sf::Vector2f direction = playerPos - shape.getPosition();
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    // Evita detenerse por completo al llegar al jugador
    if (distance > 2.f) {
        direction /= distance;
    } else {
        // Si está muy cerca, aplica una dirección mínima para mantener presión
        direction = direction / (distance + 0.001f);
    }

    shape.move(direction * speed * deltaTime);

    // --- Colisión con obstáculos ---
    for (const auto& obs : obstacles) {
        if (shape.getGlobalBounds().intersects(obs.getBounds()) && (obs.isSolid() || obs.getIsWater())) {
            
            // 1. Freno total
            shape.move(-direction * speed * deltaTime);
            
            // 2. LA CURA: Obligar al enemigo a darse la vuelta de inmediato
            direction = -direction; 
            
            break; 
        }
    }
    for (const auto& hudElement : hud) {
        if (shape.getGlobalBounds().intersects(hudElement.getBounds())) {
            sf::FloatRect e = shape.getGlobalBounds();
            sf::FloatRect o = hudElement.getBounds();

            float dx = (e.left + e.width / 2.f) - (o.left + o.width / 2.f);
            float dy = (e.top + e.height / 2.f) - (o.top + o.height / 2.f);
            sf::Vector2f push(dx, dy);
            float len = std::sqrt(push.x * push.x + push.y * push.y);
            if (len > 0.f) push /= len;

            shape.move(push * 2.f); // separa ligeramente
        }
    }
    //animacion de movimiento y dirección del sprite
    if (textureLoaded) {
        int frameX = 1;
        bool mirrorX = false;
        bool mirrorY = false; // Nueva bandera para el giro vertical
        
        // "Reloj" de caminata para alternar los tentáculos
        bool walkToggle = (static_cast<int>(shape.getPosition().x / 15.f) + 
                           static_cast<int>(shape.getPosition().y / 15.f)) % 2 == 0;
        
        if (std::abs(direction.x) > std::abs(direction.y)) {
            // MOVIMIENTO HORIZONTAL
            frameX = walkToggle ? 35 : 52; 
            mirrorX = (direction.x > 0); // Voltea en X si va a la derecha
            mirrorY = false;
        } else {
            // MOVIMIENTO VERTICAL
            frameX = walkToggle ? 1 : 18; 
            mirrorX = false;
            mirrorY = (direction.y < 0); // Voltea en Y si va hacia arriba
        }
        
        sprite.setTextureRect(sf::IntRect(frameX, 11, 16, 16));
        
        // --- EFECTO DE ESPEJO X e Y ---
        float scaleX = mirrorX ? -2.f : 2.f;
        float scaleY = mirrorY ? -2.f : 2.f;
        sprite.setScale(scaleX, scaleY);
        
        sprite.setPosition(shape.getPosition()); 
    }
}

void Enemy::render(sf::RenderTarget& window) const { 
    if (textureLoaded) {
        window.draw(sprite);
    } else {
        window.draw(shape); 
    }
}
sf::FloatRect Enemy::getBounds() const { return shape.getGlobalBounds(); }
sf::Vector2f Enemy::getPosition() const { return shape.getPosition(); }
void Enemy::move(const sf::Vector2f& offset) {
    shape.move(offset);
}
bool Enemy::canAttack() {
    if (attackTimer >= attackCooldown) {
        attackTimer = 0.f;
        return true;
    }
    return false;
}

void Enemy::takeDamage(int amount){
    health -= amount;
    if(health <0) health = 0;
}

bool Enemy::isDead() const {
    return health == 0;
}