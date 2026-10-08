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

Player::Player(float x, float y) {
    shape.setSize({32.f,32.f});
    shape.setFillColor(sf::Color::Green);
    shape.setOrigin(shape.getSize() / 2.f);
    shape.setPosition(x,y);

    // --- Sprite setup ---
    textureLoaded = false;
    const std::string texPath = "assets/textures/link.png";
    
    sf::Image image;
    // 1. Cargamos primero a sf::Image para poder manipular los píxeles
    if (!image.loadFromFile(texPath)) {
        std::cerr << "Error: no se pudo cargar " << texPath << "\n";
        textureLoaded = false;
    } else {
        // 2. Borramos el gris (Alpha por defecto es 255, que equivale al 100% del editor)
        image.createMaskFromColor(sf::Color(116, 116, 116)); 
        
        // 3. Pasamos la imagen limpia a la memoria de la tarjeta gráfica (Textura)
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

    

    // TEXTURA INICIAL (frame abajo idle)
     // Default initial texture rectangle (will validate in update)
    if (textureLoaded) {
        // safe default: if provided coords are out-of-range update() will log
        sprite.setTextureRect(sf::IntRect(1, 11, spriteWidth, spriteHeight));
    }
    sprite.setPosition(shape.getPosition());

    // --- Configuración del Sprite de la Espada ---
    swordSprite.setTexture(texture);
    swordSprite.setScale(2.f, 2.f);
    // Recorte en X 36, Y 154, con ancho 8 y alto 16
    swordSprite.setTextureRect(sf::IntRect(36, 154, 8, 16)); 
    // Centramos el pivote (la mitad de 8x16)
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
    //swordHitbox.setFillColor(sf::Color::Yellow);

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

    // Debug current working directory (useful when texture not found)
    try {
        std::cout << "CWD: " << std::filesystem::current_path() << "\n";
    } catch (...) {}


}

void Player::handleInput(float deltaTime) {
    sf::Vector2f newDir(0.f, 0.f);
    moving = false;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  {
        newDir = {-1.f, 0.f};
        moving = true;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){
        newDir = {1.f, 0.f};
        moving = true;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        newDir = {0.f, -1.f};
        moving = true;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        newDir = {0.f, 1.f};
        moving = true;
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

    //------------------------------------------
    // ANIMACIÓN DEL SPRITE 16x16 (frames exactos)
    //------------------------------------------
    const int DOWN_FRAMES[2]  = {1, 18};
    const int LEFT_FRAMES[2]  = {35, 52};
    const int UP_FRAMES[2]    = {69, 86};
    const int FRAME_Y = 11;

    // ------------------------------------------
    // ANIMACIÓN DEL SPRITE Y DIRECCIÓN
    // ------------------------------------------
    animationTimer += deltaTime;

    // Determinamos hacia dónde debe mirar (si no se mueve, usa la última dirección)
    sf::Vector2f dirToFace = moving ? direction : lastDirection;
    int frameA = DOWN_FRAMES[0], frameB = DOWN_FRAMES[1];
    bool mirrored = false;

    if (dirToFace.y > 0) { // Abajo
        frameA = DOWN_FRAMES[0]; frameB = DOWN_FRAMES[1];
    } else if (dirToFace.y < 0) { // Arriba
        frameA = UP_FRAMES[0]; frameB = UP_FRAMES[1];
    } else if (dirToFace.x < 0) { // Izquierda
        frameA = LEFT_FRAMES[0]; frameB = LEFT_FRAMES[1];
        mirrored = true;  // <-- AHORA SÍ: Espejo para voltear a la izquierda
    } else if (dirToFace.x > 0) { // Derecha
        frameA = LEFT_FRAMES[0]; frameB = LEFT_FRAMES[1];
        mirrored = false; // <-- Normal para mirar a la derecha
    }

    // Aplicamos el espejo. Al tener el origen centrado en 8,8 ya no brincará.
    sprite.setScale(mirrored ? -2.f : 2.f, 2.f);

    if (moving) {
        if (animationTimer >= animationSpeed) {
            currentFrame = (currentFrame == 0 ? 1 : 0);
            animationTimer = 0.f;
        }
    } else {
        currentFrame = 0; // Forzamos el frame base (quieto) en la dirección actual
    }

    int frameX = (currentFrame == 0 ? frameA : frameB);

    // Centramos el pivote del sprite (8, 8). 
    // Al escalar en negativo (-2.f) se volteará sobre su propio centro sin brincar.
    sprite.setOrigin(spriteWidth / 2.f, spriteHeight / 2.f);

        // Validate texture bounds before applying the rectangle
    if (textureLoaded) {
        auto ts = texture.getSize();
        if (frameX < 0 || FRAME_Y < 0 || frameX + spriteWidth > (int)ts.x || FRAME_Y + spriteHeight > (int)ts.y) {
            std::cerr << "Warning: requested sprite rect out of texture bounds: "
                      << "x=" << frameX << " y=" << FRAME_Y << " w=" << spriteWidth << " h=" << spriteHeight
                      << " texture=" << ts.x << "x" << ts.y << "\n";
            // fallback: draw the hitbox (shape) instead of the sprite to avoid white quad
            textureLoaded = false;
        } else {
            sprite.setTextureRect(sf::IntRect(frameX, FRAME_Y, spriteWidth, spriteHeight));
        }
    }

    //------------------------------------------
    // MOVIMIENTO / FÍSICA
    //------------------------------------------

    // Knockback
    if (knockbackTimer > 0.f) {
        shape.move(knockback * deltaTime);
        knockbackTimer -= deltaTime;
        sprite.setPosition(shape.getPosition());
    } else {

        sf::Vector2f movement = direction * speed * deltaTime;

        // Move X
        if (movement.x != 0.f) {
            shape.move(movement.x, 0);
            for (const auto& obs : obstacles) {
                if (shape.getGlobalBounds().intersects(obs.getBounds())) {
                    shape.move(-movement.x, 0);
                    break;
                }
            }
            for (const auto& h : hud) {
                if (shape.getGlobalBounds().intersects(h.getBounds())) {
                    shape.move(0, -movement.x);
                    break;
                }
            }
        }

        // Move Y
        if (movement.y != 0.f) {
            shape.move(0, movement.y);
            for (const auto& obs : obstacles) {
                if (shape.getGlobalBounds().intersects(obs.getBounds())) {
                    shape.move(0, -movement.y);
                    break;
                }
            }
            for (const auto& h : hud) {
                if (shape.getGlobalBounds().intersects(h.getBounds())) {
                    shape.move(0, -movement.y);
                    break;
                }
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

    // INVULNERABILIDAD (parpadeo)
    if (isInvulnerable) {
        invulnerableTimer += deltaTime;
        if ((int)(invulnerableTimer * 8) % 2 == 0)
            sprite.setColor(sf::Color(255,150,150));
        else
            sprite.setColor(sf::Color::White);

        if (invulnerableTimer >= invulnerableDuration) {
            isInvulnerable = false;
            invulnerableTimer = 0;
            sprite.setColor(sf::Color::White);
        }
    }

    if (health == maxHealth)
        canShootProjectile = true;

    //------------------------------------------
    // PROYECTILES
    //------------------------------------------
    for (size_t i = 0; i < projectiles.size();) {
        projectiles[i].update(deltaTime);
        auto pos = projectiles[i].getPosition();

        bool erase = false;

        if (pos.x < 0 || pos.y < 0 ||
            pos.x > window.getSize().x || pos.y > window.getSize().y) {

            erase = true;
        }

        for (const auto& obs : obstacles)
            if (!erase && projectiles[i].getBounds().intersects(obs.getBounds()))
                erase = true;

        if (erase)
            projectiles.erase(projectiles.begin() + i);
        else
            i++;
    }
}

void Player::render(sf::RenderWindow& window) {
    if (textureLoaded) {
        window.draw(sprite);
    } else {
        // draw the debug hitbox so player is visible
        window.draw(shape);
    }    for (auto &p : projectiles) p.render(window);
    
    if (attacking) {
        window.draw(swordSprite);
    }
}

void Player::shoot() {
    if (shootTimer >= shootCooldown && canShootProjectile && health == maxHealth) {

        sf::Vector2f projDir = (direction != sf::Vector2f(0,0))
                               ? direction : lastDirection;

        if (projDir == sf::Vector2f(0,0))
            projDir = sf::Vector2f(0,-1);

        sf::Vector2f spawn = shape.getPosition() +
                            projDir * (shape.getSize().x / 2.f + 4.f);

        projectiles.emplace_back(spawn.x, spawn.y, projDir, 420.f);
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

bool Player::canTakeDamage() const {
    return !isInvulnerable;
}

void Player::applynockback(const sf::Vector2f& dir) {
    knockback = dir * 300.f;
    knockbackTimer = knockbackDuration;
}

bool Player::isAttacking() const {
    return attacking;
}

void Player::attack() {
    if (swordTimer < swordCooldown) return;

    swordTimer = 0.f;
    attacking = true;

    sf::Vector2f offset = lastDirection * 32.f;
    float angulo = 0.f;

    // Como el sprite mira hacia ARRIBA por defecto, los ángulos cambian:
    if (lastDirection.y < 0.f) {        // ARRIBA
        angulo = 0.f; 
        offset = {0.f, lastDirection.y * (shape.getSize().y / 2.f + 12.f)};
    } else if (lastDirection.x > 0.f) { // DERECHA
        angulo = 90.f; 
        offset = {lastDirection.x * (shape.getSize().x / 2.f + 12.f), 0.f};
    } else if (lastDirection.y > 0.f) { // ABAJO
        angulo = 180.f; 
        offset = {0.f, lastDirection.y * (shape.getSize().y / 2.f + 12.f)};
    } else if (lastDirection.x < 0.f) { // IZQUIERDA
        angulo = -90.f; 
        offset = {lastDirection.x * (shape.getSize().x / 2.f + 12.f), 0.f};
    }

    swordSprite.setRotation(angulo);

    // Ajustar hitbox invisible para que coincida con el sprite vertical u horizontal
    if (lastDirection.x != 0.f) swordHitbox.setSize({20.f, 10.f});
    else swordHitbox.setSize({10.f, 20.f});

    // Mantenemos la hitbox física invisible para el daño
    swordHitbox.setOrigin(swordHitbox.getSize() / 2.f);
    
    // Mantenemos la hitbox física invisible para el daño
    swordHitbox.setOrigin(swordHitbox.getSize() / 2.f);
    swordHitbox.setPosition(shape.getPosition() + offset);
    swordHitbox.move(lastDirection * 6.f);
    
    // Anclamos el dibujo de la espada sobre la hitbox
    swordSprite.setPosition(swordHitbox.getPosition());
}

sf::FloatRect Player::getSwordBounds() const {
    if (!attacking) return sf::FloatRect();
    return swordHitbox.getGlobalBounds();
}