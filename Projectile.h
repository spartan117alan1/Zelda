#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <SFML/Graphics.hpp>

class Projectile {
private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    float speed; // pixeles por segundo

    // --- Gráficos de la Espada ---
    sf::Texture texture;
    sf::Sprite sprite;
    bool textureLoaded;
    int direction; // 0=Arriba, 1=Abajo, 2=Izquierda, 3=Derecha
    
public:
    // Actualizamos el constructor para recibir el entero de la dirección
    Projectile(float startX, float startY, int dir);
    
    void update(float deltaTime);
    void render(sf::RenderTarget& window);
    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const { return position; }
};

#endif