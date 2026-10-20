#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>

#include <vector>
#include "Projectile.h"
#include "Obstacle.h"
#include "Hud.h"
#include "Enemy.h"
#include "Zora.h"

// --- SISTEMA DE INVENTARIO ---
    enum class EquipItem { 
        NONE, 
        WOODEN_SWORD, 
        MAGICAL_SWORD, 
        BOW, 
        BOMB, 
        BOOMERANG, 
        CANDLE, 
        ICE_ROD,
        TRIFORCE
    };
    

class Player {
private:
    // Hitbox
    sf::RectangleShape shape;

    // Movement
    sf::Vector2f direction;
    sf::Vector2f lastDirection;
    sf::Vector2f knockback;
    float knockbackTimer;
    float knockbackDuration;
    float speed;
    bool moving;


    //HUD
    // Health
    int health = 3;
    int maxHealth = 3;
    bool isAlive;

    // Score
    int score = 0;
    int rupees = 0;   // <-- NUEVO
    int keys = 0;     // <-- NUEVO
    int bombs = 0;    // <-- NUEVO

    // Sword attack
    float swordCooldown;
    float swordTimer;
    bool attacking;
    sf::RectangleShape swordHitbox;

    // Projectiles
    std::vector<Projectile> projectiles;
    float shootCooldown;
    float shootTimer;
    bool canShootProjectile;

    // Damage / Invulnerability
    bool isInvulnerable;
    float invulnerableTimer;
    float invulnerableDuration;
    sf::Color normalColor;

    // Sprite system
    sf::Texture texture;
    sf::Sprite sprite;
    sf::Sprite swordSprite;
    int frameX;
    int frameY;
    float animationTimer;
    float animationSpeed;
    int spriteWidth;
    int spriteHeight;
    int currentFrame;
    bool textureLoaded;

    

public:
    Player(float x = 100.f, float y = 100.f);

    // --- SISTEMA DE INVENTARIO ---
    enum class EquipItem { 
        NONE, 
        WOODEN_SWORD, 
        MAGICAL_SWORD, 
        BOW, 
        BOMB, 
        BOOMERANG, 
        CANDLE, 
        ICE_ROD,
        TRIFORCE
    };

    //invetario al nacer
    bool hasWoodenSword = false;
    bool hasMagicalSword = false;
    bool hasBow = false;
    bool hasBomb = false;
    bool hasBoomerang = false;
    bool hasCandle = false;
    bool hasIceRod = false;
    bool hasTriforce = false;

    // Qué tiene equipado en sus manos
    EquipItem itemA = EquipItem::NONE;
    EquipItem itemB = EquipItem::NONE;

    // Funciones para equipar
    void setItemA(EquipItem item) { itemA = item; }
    void setItemB(EquipItem item) { itemB = item; }
    EquipItem getItemA() const { return itemA; }
    EquipItem getItemB() const { return itemB; }

    bool canBlock(const sf::FloatRect& projBounds) const;

    void handleInput(float deltaTime);
    void update(float deltaTime,
                const sf::RenderWindow& window,
                const std::vector<Obstacle>& obstacles,
                const std::vector<Hud>& hud,
                std::vector<Enemy>& enemies);

    void render(sf::RenderTarget& window);

    // Positioning
    void setPosition(const sf::Vector2f& pos);
    sf::Vector2f getPosition() const { return shape.getPosition(); }
    void move(float dx, float dy);

    // Attacks
    void shoot();
    void attack();
    bool isAttacking() const;
    sf::FloatRect getSwordBounds() const;

    // Damage
    void takeDamage(int amount);

    void heal(int amount);

    bool canTakeDamage() const;

    // Knockback
    void applynockback(const sf::Vector2f& dir);

    // Projectiles access
    std::vector<Projectile>& getProjectiles() { return projectiles; }

    // Status
    bool GetIsAlive() const { return isAlive; }
    int getHearts() const { return health; }
    int getMaxHealth() const { return maxHealth; }

    void addHeartContainer() {
        maxHealth += 1;
        health = maxHealth; // Te cura al máximo al agarrarlo
    }
    
    //Score
    int getScore() const { return score; }
    void addScore(int points) { score += points; }

    int getRupees() const { return rupees; }
    void addRupees(int amount) { rupees += amount; }
    
    int getKeys() const { return keys; }
    void addKeys(int amount) { keys += amount; }
    
    int getBombs() const { return bombs; }
    void addBombs(int amount) { bombs += amount; }

    // Hitbox getter
    sf::FloatRect getBounds() const { return shape.getGlobalBounds(); }
};

#endif
