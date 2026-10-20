#include "Obstacle.h"
#include <iostream>

sf::Texture Obstacle::texture;
bool Obstacle::textureLoaded = false;

// 1. CONSTRUCTOR VIEJO
Obstacle::Obstacle(float x, float y, float w, float h, bool water) {
    type = water ? ObstacleType::WATER : ObstacleType::WALL;
    health = 1; 
    destroyed = false; 
    onFire = false;    // <--- SEGURO CONTRA INCENDIOS ESPONTÁNEOS
    fireTimer = 0.f;
    
    shape.setSize({w, h}); 
    shape.setPosition(x, y);

    if(type == ObstacleType::WATER) shape.setFillColor(sf::Color(0, 0, 255, 150)); 
    else shape.setFillColor(sf::Color(103, 163, 60)); 
}

// 2. CONSTRUCTOR NUEVO
Obstacle::Obstacle(float x, float y, float w, float h, ObstacleType t, int target) {
    type = t;
    health = (type == ObstacleType::TREE) ? 1 : 1; 
    destroyed = false; 
    onFire = false;    
    fireTimer = 0.f;
    targetId = target; 
    
    shape.setSize({w, h}); 
    shape.setPosition(x, y);

    if (!textureLoaded) {
        if (texture.loadFromFile("assets/textures/overworldTexture.png")) {
            textureLoaded = true;
        }
    }

    if (textureLoaded) {
        sprite.setTexture(texture);
        if (type == ObstacleType::TREE) sprite.setTextureRect(sf::IntRect(137, 222, 16, 16));
        else if (type == ObstacleType::JAR) sprite.setTextureRect(sf::IntRect(120, 222, 16, 16));
        else if (type == ObstacleType::DUNGEON_DOOR) sprite.setTextureRect(sf::IntRect(120, 205, 16, 16));
        else if (type == ObstacleType::CAVE_DOOR) sprite.setTextureRect(sf::IntRect(137, 205, 16, 16));
        else if (type == ObstacleType::TRUNK) sprite.setTextureRect(sf::IntRect(18, 222, 16, 16)); 
        else if (type == ObstacleType::BOOMBABLE_WALL) sprite.setTextureRect(sf::IntRect(120, 239, 16, 16)); // <--- ¡Descomentado! (Ajusta la coordenada si prefieres otra)
        else sprite.setTextureRect(sf::IntRect(0, 0, 16, 16)); 
        
        sprite.setScale(w / 16.f, h / 16.f);
        sprite.setPosition(x, y);
    } else {
        if (type == ObstacleType::TREE) shape.setFillColor(sf::Color(0, 100, 0)); 
        else if (type == ObstacleType::TRUNK) shape.setFillColor(sf::Color(139, 69, 19)); 
        else if (type == ObstacleType::DUNGEON_DOOR) shape.setFillColor(sf::Color(0, 0, 0)); 
        else if (type == ObstacleType::CAVE_DOOR) shape.setFillColor(sf::Color(50, 50, 50));
        else if (type == ObstacleType::JAR) shape.setFillColor(sf::Color(180, 130, 70));
        else if (type == ObstacleType::BOOMBABLE_WALL) shape.setFillColor(sf::Color(120, 220, 100)); // <--- VERDE CLARO para identificarlo
    }
}

void Obstacle::transformToDungeonDoor() {
    type = ObstacleType::DUNGEON_DOOR;
    if (textureLoaded) {
        sprite.setTextureRect(sf::IntRect(120, 205, 16, 16)); // Sprite de puerta de mazmorra
    } else {
        shape.setFillColor(sf::Color(0, 0, 0)); // Color negro de entrada/puerta
    }
}

void Obstacle::transformToCaveDoor(int target){
    type = ObstacleType::CAVE_DOOR;
    if (textureLoaded) {
        sprite.setTextureRect(sf::IntRect(137,205,16,16));
    }else{
        shape.setFillColor(sf::Color(50,50,50));
    }
}

void Obstacle::update(float dt) {
    if (onFire) {
        fireTimer += dt;
        
        // Alternar entre los dos fotogramas de árboles quemados cada 0.2 segundos
        bool frameToggle = static_cast<int>(fireTimer * 5.0f) % 2 == 0;
        
        // Considerando 1 píxel de espacio entre texturas:
        // Fotograma 1: Y = 239 (o 240 según tu ajuste)
        // Fotograma 2: Y = 256 (considerando el salto de 1px de separación)
        int rectY = frameToggle ? 239 : 256;
        
        if (textureLoaded) {
            sprite.setTextureRect(sf::IntRect(137, rectY, 12, 12)); // Altura 12 manteniendo el bloque limpio
            // Efecto de parpadeo en color rojo/naranja
            sprite.setColor(frameToggle ? sf::Color(255, 150, 100) : sf::Color(255, 50, 50));
        }

        // A los 3 segundos el árbol se consume y se destruye
        if (fireTimer >= 3.0f) { 
            destroyed = true; 
        }
    }
}

void Obstacle::render(sf::RenderTarget& window) const { 
    // Si es agua o pared sólida normal, dibuja el rectángulo
    if (type == ObstacleType::WALL || type == ObstacleType::WATER) {
        window.draw(shape); 
    } 
    else {
        // Para árboles, jarras o árboles quemándose (onFire), dibujamos el sprite
        if (textureLoaded) {
            window.draw(sprite);
        } else {
            window.draw(shape); 
        }
    }
}

sf::FloatRect Obstacle::getBounds() const { return shape.getGlobalBounds(); }

void Obstacle::takeDamage() {
    if (type == ObstacleType::TREE && !onFire) {
        health--;
        if (health <= 0) {
            type = ObstacleType::TRUNK;
            if (textureLoaded) sprite.setTextureRect(sf::IntRect(18, 222, 16, 16)); 
            else shape.setFillColor(sf::Color(139, 69, 19)); 
        }
    } 
    else if (type == ObstacleType::JAR) {
        destroyed = true; 
    }
}

void Obstacle::ignite() {
    if (type == ObstacleType::TREE && !onFire) {
        onFire = true;
        fireTimer = 0.f;
    }
}

void Obstacle::move(const sf::Vector2f& offset) {
    if (type == ObstacleType::TRUNK) {
        shape.move(offset);
        if (textureLoaded) sprite.move(offset);
    }
}

void Obstacle::transformToPlatform() {
    if (type == ObstacleType::TRUNK) {
        type = ObstacleType::PLATFORM;
        if (!textureLoaded) shape.setFillColor(sf::Color(205, 133, 63)); 
    }
}