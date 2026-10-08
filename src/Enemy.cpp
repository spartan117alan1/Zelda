#include "Enemy.h"
#include <cmath>
Enemy::Enemy(float x, float y)
    : speed(70.f), attackCooldown(1.5f), attackTimer(0.0f)
{
    shape.setSize(sf::Vector2f(32.f, 32.f));
    shape.setFillColor(sf::Color(245, 140, 10));
    shape.setOrigin(shape.getSize() / 2.f);
    shape.setPosition(x, y);
}
void Enemy::update(float deltaTime, const sf::Vector2f& playerPos, const std::vector<Obstacle>& obstacles, const std::vector<Hud>& hud) {
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
        if (shape.getGlobalBounds().intersects(obs.getBounds())) {
            sf::FloatRect e = shape.getGlobalBounds();
            sf::FloatRect o = obs.getBounds();

            float dx = (e.left + e.width / 2.f) - (o.left + o.width / 2.f);
            float dy = (e.top + e.height / 2.f) - (o.top + o.height / 2.f);
            sf::Vector2f push(dx, dy);
            float len = std::sqrt(push.x * push.x + push.y * push.y);
            if (len > 0.f) push /= len;

            shape.move(push * 2.f); // separa ligeramente
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
}

void Enemy::render(sf::RenderWindow& window) const { window.draw(shape); }
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