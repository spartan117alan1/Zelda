#include "Player.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <filesystem>

using namespace std;

void Player::setPosition(const sf::Vector2f& pos) {
    shape.setPosition(pos);
    sprite.setPosition(pos);
}

void Player::move(float dx, float dy) {
    shape.move(dx, dy);
    sprite.setPosition(shape.getPosition());
}

Player::Player(float x, float y) {
    shape.setSize({32.f,32.f});
    shape.setFillColor(sf::Color::Green);
    shape.setOrigin(shape.getSize() / 2.f);
    shape.setPosition(x,y);

    // --- Sprite setup ---
    textureLoaded = false;
    const std::string texPath = "assets/textures/link.png";
    
    sf::Image image;
    if (!image.loadFromFile(texPath)) {
        std::cerr << "Error: no se pudo cargar " << texPath << "\n";
        textureLoaded = false;
    } else {
        image.createMaskFromColor(sf::Color(116, 116, 116)); 
        texture.loadFromImage(image);
        textureLoaded = true;
        auto sz = texture.getSize();
        std::cout << "Player texture loaded. size: " << sz.x << "x" << sz.y << "\n";
    }

    cout << "Directorio actual: " << std::filesystem::current_path() << endl;

    sprite.setTexture(texture);
    sprite.setScale(2.f, 2.f);

    spriteWidth = 16;
    spriteHeight = 16;
    animationTimer = 0.f;
    animationSpeed = 0.15f;
    moving = false;
    currentFrame = 0;

    if (textureLoaded) {
        sprite.setTextureRect(sf::IntRect(1, 10, spriteWidth, spriteHeight));
    }
    sprite.setPosition(shape.getPosition());

    // --- Configuración del Sprite de la Espada ---
    swordSprite.setTexture(texture);
    swordSprite.setScale(2.f, 2.f);
    swordSprite.setTextureRect(sf::IntRect(36, 154, 8, 16)); 
    swordSprite.setOrigin(4.f, 8.f);

    // Knockback
    knockback = {0.f, 0.f};
    knockbackTimer = 0.f;
    knockbackDuration = 0.25f;

    // Sword
    swordCooldown = 0.4f;
    swordTimer = swordCooldown;
    attacking = false;
    swordHitbox.setSize({20.f, 10.f});

    direction = sf::Vector2f(0.f, -1.f);
    lastDirection = direction;
    speed = 180.f;
    health = maxHealth = 3;
    isAlive = true;

    shootCooldown = 0.5f;
    shootTimer = shootCooldown;

    isInvulnerable = false;
    invulnerableTimer = 0.f;
    invulnerableDuration = 1.0f;

    normalColor = shape.getFillColor();

    try {
        std::cout << "CWD: " << std::filesystem::current_path() << "\n";
    } catch (...) {}
    
    // Nace sin armas equipadas
    itemA = EquipItem::NONE;
    itemB = EquipItem::NONE;
}

void Player::handleInput(float deltaTime) {
    sf::Vector2f newDir(0.f, 0.f);
    moving = false;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  {
        newDir = {-1.f, 0.f}; moving = true;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){
        newDir = {1.f, 0.f}; moving = true;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        newDir = {0.f, -1.f}; moving = true;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        newDir = {0.f, 1.f}; moving = true;
    }

    direction = newDir;

    if (newDir != sf::Vector2f(0.f, 0.f)) {
        lastDirection = newDir;
    }
}

void Player::update(float deltaTime, const sf::RenderWindow& window,
                    const std::vector<Obstacle>& obstacles,
                    const std::vector<Hud>& hud,
                    std::vector<Enemy>& enemies) {

    sprite.setTexture(texture);
    swordSprite.setTexture(texture);

    const int DOWN_FRAMES[2]  = {1, 18};
    const int LEFT_FRAMES[2]  = {35, 52};
    const int UP_FRAMES[2]    = {69, 86};
    const int FRAME_Y = 11;

    animationTimer += deltaTime;
    sf::Vector2f dirToFace = moving ? direction : lastDirection;
    int frameA = DOWN_FRAMES[0], frameB = DOWN_FRAMES[1];
    bool mirrored = false;

    if (dirToFace.y > 0) { frameA = DOWN_FRAMES[0]; frameB = DOWN_FRAMES[1]; } 
    else if (dirToFace.y < 0) { frameA = UP_FRAMES[0]; frameB = UP_FRAMES[1]; } 
    else if (dirToFace.x < 0) { frameA = LEFT_FRAMES[0]; frameB = LEFT_FRAMES[1]; mirrored = true; } 
    else if (dirToFace.x > 0) { frameA = LEFT_FRAMES[0]; frameB = LEFT_FRAMES[1]; mirrored = false; }

    sprite.setScale(mirrored ? -2.f : 2.f, 2.f);

    if (moving) {
        if (animationTimer >= animationSpeed) {
            currentFrame = (currentFrame == 0 ? 1 : 0);
            animationTimer = 0.f;
        }
    } else {
        currentFrame = 0; 
    }

    int frameX = (currentFrame == 0 ? frameA : frameB);
    sprite.setOrigin(spriteWidth / 2.f, spriteHeight / 2.f);

    if (textureLoaded) {
        auto ts = texture.getSize();
        if (frameX < 0 || FRAME_Y < 0 || frameX + spriteWidth > (int)ts.x || FRAME_Y + spriteHeight > (int)ts.y) {
            std::cerr << "Warning: requested sprite rect out of texture bounds\n";
            textureLoaded = false;
        } else {
            sprite.setTextureRect(sf::IntRect(frameX, FRAME_Y, spriteWidth, spriteHeight));
        }
    }

    //------------------------------------------
    // MOVIMIENTO / FÍSICA (NUEVO SISTEMA DE NÚCLEO)
    //------------------------------------------

    if (knockbackTimer > 0.f) {
        sf::Vector2f kbStep = knockback * deltaTime;

        // Knockback en X
        shape.move(kbStep.x, 0.f);
        sf::FloatRect pBounds = shape.getGlobalBounds();
        sf::FloatRect coreBox = pBounds;
        coreBox.left += 8.f; coreBox.width -= 16.f; // Hitbox central de 8x8
        coreBox.top += 8.f; coreBox.height -= 16.f;

        bool onPlatformX = false;
        for (const auto& obs : obstacles) {
            if (obs.getType() == ObstacleType::PLATFORM && coreBox.intersects(obs.getBounds())) {
                onPlatformX = true; break;
            }
        }
        for (const auto& obs : obstacles) {
            if (obs.isSolid() && pBounds.intersects(obs.getBounds())) {
                shape.move(-kbStep.x, 0.f); break; // Muros usan el cuerpo entero
            }
            if (obs.getIsWater() && !onPlatformX && coreBox.intersects(obs.getBounds())) {
                shape.move(-kbStep.x, 0.f); break; // Agua usa el núcleo
            }
        }
        for (const auto& h : hud) {
            if (pBounds.intersects(h.getBounds())) { shape.move(-kbStep.x, 0.f); break; }
        }

        // Knockback en Y
        shape.move(0.f, kbStep.y);
        pBounds = shape.getGlobalBounds();
        coreBox = pBounds;
        coreBox.left += 8.f; coreBox.width -= 16.f;
        coreBox.top += 8.f; coreBox.height -= 16.f;

        bool onPlatformY = false;
        for (const auto& obs : obstacles) {
            if (obs.getType() == ObstacleType::PLATFORM && coreBox.intersects(obs.getBounds())) {
                onPlatformY = true; break;
            }
        }
        for (const auto& obs : obstacles) {
            if (obs.isSolid() && pBounds.intersects(obs.getBounds())) {
                shape.move(0.f, -kbStep.y); break; 
            }
            if (obs.getIsWater() && !onPlatformY && coreBox.intersects(obs.getBounds())) {
                shape.move(0.f, -kbStep.y); break; 
            }
        }
        for (const auto& h : hud) {
            if (pBounds.intersects(h.getBounds())) { shape.move(0.f, -kbStep.y); break; }
        }

        knockbackTimer -= deltaTime;
        sprite.setPosition(shape.getPosition());
    } else {
        sf::Vector2f movement = direction * speed * deltaTime;

        // Move X
        if (movement.x != 0.f) {
            shape.move(movement.x, 0);
            sf::FloatRect pBounds = shape.getGlobalBounds();
            sf::FloatRect coreBox = pBounds;
            coreBox.left += 8.f; coreBox.width -= 16.f;
            coreBox.top += 8.f; coreBox.height -= 16.f;
            
            bool onPlatform = false;
            for (const auto& obs : obstacles) {
                if (obs.getType() == ObstacleType::PLATFORM && coreBox.intersects(obs.getBounds())) {
                    onPlatform = true; break;
                }
            }

            for (const auto& obs : obstacles) {
                if (obs.isSolid() && pBounds.intersects(obs.getBounds())) {
                    shape.move(-movement.x, 0); break;
                }
                if (obs.getIsWater() && !onPlatform && coreBox.intersects(obs.getBounds())) {
                    shape.move(-movement.x, 0); break;
                }
            }
            
            for (const auto& h : hud) {
                if (pBounds.intersects(h.getBounds())) { shape.move(-movement.x, 0); break; }
            }
        }

        // Move Y
        if (movement.y != 0.f) {
            shape.move(0, movement.y);
            sf::FloatRect pBounds = shape.getGlobalBounds();
            sf::FloatRect coreBox = pBounds;
            coreBox.left += 8.f; coreBox.width -= 16.f;
            coreBox.top += 8.f; coreBox.height -= 16.f;
            
            bool onPlatform = false;
            for (const auto& obs : obstacles) {
                if (obs.getType() == ObstacleType::PLATFORM && coreBox.intersects(obs.getBounds())) {
                    onPlatform = true; break;
                }
            }

            for (const auto& obs : obstacles) {
                if (obs.isSolid() && pBounds.intersects(obs.getBounds())) {
                    shape.move(0, -movement.y); break;
                }
                if (obs.getIsWater() && !onPlatform && coreBox.intersects(obs.getBounds())) {
                    shape.move(0, -movement.y); break;
                }
            }
            
            for (const auto& h : hud) {
                if (pBounds.intersects(h.getBounds())) { shape.move(0, -movement.y); break; }
            }
        }
    }

    sprite.setPosition(shape.getPosition());

    //------------------------------------------
    // TIMERS
    //------------------------------------------
    shootTimer += deltaTime;
    swordTimer += deltaTime;

    float swordActiveDuration = std::min(0.12f, swordCooldown);
    if (attacking && swordTimer >= swordActiveDuration)
        attacking = false;

    // INVULNERABILIDAD
    if (isInvulnerable) {
        invulnerableTimer += deltaTime;
        if ((int)(invulnerableTimer * 8) % 2 == 0) sprite.setColor(sf::Color(255,150,150));
        else sprite.setColor(sf::Color::White);

        if (invulnerableTimer >= invulnerableDuration) {
            isInvulnerable = false;
            invulnerableTimer = 0;
            sprite.setColor(sf::Color::White);
        }
    }

    if (health == maxHealth) canShootProjectile = true;

    //------------------------------------------
    // PROYECTILES
    //------------------------------------------
    for (size_t i = 0; i < projectiles.size();) {
        projectiles[i].update(deltaTime);
        auto pos = projectiles[i].getPosition();
        bool erase = false;

        if (pos.x < 0 || pos.y < 0 || pos.x > window.getSize().x || pos.y > window.getSize().y) {
            erase = true;
        }

        // CORRECCIÓN: Los proyectiles ahora ignoran el agua y las plataformas
        for (const auto& obs : obstacles) {
            if (!erase && obs.isSolid() && projectiles[i].getBounds().intersects(obs.getBounds())) {
                erase = true;
            }
        }

        if (erase) projectiles.erase(projectiles.begin() + i);
        else i++;
    }
}

void Player::render(sf::RenderTarget& window) {
    if (textureLoaded) window.draw(sprite);
    else window.draw(shape); 
    
    for (auto &p : projectiles) p.render(window);
    if (attacking) window.draw(swordSprite);
}

void Player::shoot() {

    if (itemA != EquipItem::MAGICAL_SWORD) {
        return; // Solo dispara si tiene la espada mágica equipada
    }

    if (shootTimer >= shootCooldown && canShootProjectile && health == maxHealth) {
        sf::Vector2f projDir = (direction != sf::Vector2f(0,0)) ? direction : lastDirection;
        if (projDir == sf::Vector2f(0,0)) projDir = sf::Vector2f(0,-1);

        sf::Vector2f spawn = shape.getPosition() + projDir * (shape.getSize().x / 2.f + 4.f);
        int dirInt = 0; 
        if (projDir.y < 0) dirInt = 0;      
        else if (projDir.y > 0) dirInt = 1; 
        else if (projDir.x < 0) dirInt = 2; 
        else if (projDir.x > 0) dirInt = 3; 

        projectiles.emplace_back(spawn.x, spawn.y, dirInt);
        shootTimer = 0.f;
    }
}

void Player::takeDamage(int amount) {
    if (!canTakeDamage()) return;
    health -= amount;
    if (health < 0) health = 0;
    canShootProjectile = false;
    isInvulnerable = true;
    invulnerableTimer = 0;
    sprite.setColor(sf::Color(255,100,100));
}

void Player::heal(int amount) {
    health += amount;
    if (health > maxHealth) health = maxHealth;
}

bool Player::canTakeDamage() const { return !isInvulnerable; }

void Player::applynockback(const sf::Vector2f& dir) {
    knockback = dir * 300.f;
    knockbackTimer = knockbackDuration;
}

bool Player::isAttacking() const { return attacking; }

void Player::attack() {
    // CANDADO 1: Si no tiene ninguna espada en la ranura A, no hace nada.
    if (itemA != EquipItem::WOODEN_SWORD && itemA != EquipItem::MAGICAL_SWORD) {
        return; 
    }
    if (swordTimer < swordCooldown) return;

    swordTimer = 0.f;
    attacking = true;

    sf::Vector2f offset = lastDirection * 32.f;
    float angulo = 0.f;

    if (lastDirection.y < 0.f) { angulo = 0.f; offset = {0.f, lastDirection.y * (shape.getSize().y / 2.f + 12.f)}; } 
    else if (lastDirection.x > 0.f) { angulo = 90.f; offset = {lastDirection.x * (shape.getSize().x / 2.f + 12.f), 0.f}; } 
    else if (lastDirection.y > 0.f) { angulo = 180.f; offset = {0.f, lastDirection.y * (shape.getSize().y / 2.f + 12.f)}; } 
    else if (lastDirection.x < 0.f) { angulo = -90.f; offset = {lastDirection.x * (shape.getSize().x / 2.f + 12.f), 0.f}; }

    swordSprite.setRotation(angulo);

    if (lastDirection.x != 0.f) swordHitbox.setSize({20.f, 10.f});
    else swordHitbox.setSize({10.f, 20.f});

    swordHitbox.setOrigin(swordHitbox.getSize() / 2.f);
    swordHitbox.setPosition(shape.getPosition() + offset);
    swordHitbox.move(lastDirection * 6.f);
    swordSprite.setPosition(swordHitbox.getPosition());

    // CANDADO 2: El proyectil mágico AHORA depende del arma y la vida
    if (itemA == EquipItem::MAGICAL_SWORD && health == maxHealth) {
        shoot(); // Dispara el rayo
    }
}

sf::FloatRect Player::getSwordBounds() const {
    if (!attacking) return sf::FloatRect();
    return swordHitbox.getGlobalBounds();
}

bool Player::canBlock(const sf::FloatRect& projBounds) const {
    // Si está atacando, el escudo se hace a un lado y queda vulnerable
    if (attacking) return false; 

    sf::FloatRect pBounds = shape.getGlobalBounds();

    // Compara de qué lado está chocando el proyectil relativo hacia dónde mira Link
    if (lastDirection.y < 0.f && projBounds.top + projBounds.height / 2.f < pBounds.top + pBounds.height / 2.f) return true; // Mirando Arriba
    if (lastDirection.y > 0.f && projBounds.top + projBounds.height / 2.f > pBounds.top + pBounds.height / 2.f) return true; // Mirando Abajo
    if (lastDirection.x < 0.f && projBounds.left + projBounds.width / 2.f < pBounds.left + pBounds.width / 2.f) return true; // Mirando Izquierda
    if (lastDirection.x > 0.f && projBounds.left + projBounds.width / 2.f > pBounds.left + pBounds.width / 2.f) return true; // Mirando Derecha

    return false;
}