#include "Projectile.h"
#include <iostream>

Projectile::Projectile(float startX, float startY, int dir) 
    : position(startX, startY), speed(350.f), direction(dir), textureLoaded(false) 
{
    // Carga de textura (ajusta la ruta si tu imagen se llama diferente)
    if (texture.loadFromFile("assets/textures/link.png")) {
        sprite.setTexture(texture);
        // Recorte para la Espada de Madera (ajusta coordenadas si es necesario)
        sprite.setTextureRect(sf::IntRect(36, 154, 8, 16)); 
        sprite.setScale(2.f, 2.f);
        
        // El origen en el centro facilita las rotaciones
        sprite.setOrigin(4.f, 8.f); 
        textureLoaded = true;
    } else {
        std::cerr << "Error al cargar la textura de la espada en Projectile\n";
    }

    // Configuración de velocidad y rotación según la dirección
    // Asumiendo tu mapeo: 0=Arriba, 1=Abajo, 2=Izquierda, 3=Derecha
    if (direction == 0) { // Arriba
        velocity = sf::Vector2f(0.f, -speed);
        if (textureLoaded) sprite.setRotation(0.f);
    } 
    else if (direction == 1) { // Abajo
        velocity = sf::Vector2f(0.f, speed);
        if (textureLoaded) sprite.setRotation(180.f);
    } 
    else if (direction == 2) { // Izquierda
        velocity = sf::Vector2f(-speed, 0.f);
        if (textureLoaded) sprite.setRotation(270.f);
    } 
    else if (direction == 3) { // Derecha
        velocity = sf::Vector2f(speed, 0.f);
        if (textureLoaded) sprite.setRotation(90.f);
    }
}

void Projectile::update(float deltaTime) {
    position += velocity * deltaTime;
    if (textureLoaded) {
        sprite.setPosition(position);
    }
}

void Projectile::render(sf::RenderTarget& window) {
    if (textureLoaded) {
        window.draw(sprite);
    } else {
        // Respaldo de seguridad en caso de error con la textura
        sf::RectangleShape fallback(sf::Vector2f(10.f, 10.f));
        fallback.setFillColor(sf::Color::White);
        fallback.setPosition(position);
        window.draw(fallback);
    }
}

sf::FloatRect Projectile::getBounds() const {
    if (textureLoaded) {
        return sprite.getGlobalBounds();
    }
    return sf::FloatRect(position.x - 5.f, position.y - 5.f, 10.f, 10.f);
}