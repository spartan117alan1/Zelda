#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <SFML/Graphics.hpp>

enum class ObstacleType {
    WALL,
    WATER,
    TREE,
    TRUNK,
    PLATFORM,
    DUNGEON_DOOR,
    CAVE_DOOR,
    BOOMBABLE_WALL,
    JAR
};

class Obstacle {
private:
    sf::RectangleShape shape;
    bool isWater;
    ObstacleType type;

    //nuevas variables para los graficos de arboles/troncos y demas

    sf::Sprite sprite;
    static sf::Texture texture;
    static bool textureLoaded;

    int health; // vida para los objetos

    // --- NUEVAS VARIABLES DE SISTEMAS ---
    bool destroyed; // Para saber si debemos borrarlo del mapa
    bool onFire;    // Estado de ignición
    float fireTimer;// Reloj interno para consumirse

    //identificador de door
    int targetId = -1;

public:
    Obstacle(float x=0.f, float y=0.f, float w=32.f, float h=32.f, bool water=false);

    //NUEVO CONSTRUCTOR PARA LOS NUEVOS TIPOS DE OBSTACULOS
    Obstacle(float x, float y, float w, float h, ObstacleType t, int target = -1);

    // NUEVO: El obstáculo ahora necesita "pensar" para quemarse
    void update(float dt);

    void render(sf::RenderTarget& window) const;
    sf::FloatRect getBounds() const;

    bool getIsWater() const { return type == ObstacleType::WATER; }

    bool isSolid() const {
        //si la plataforma o puerta de mazmorra, el jugador puede pasar encima de
        return type ==ObstacleType::WALL || type == ObstacleType::TREE || type == ObstacleType::TRUNK || type == ObstacleType::JAR || type == ObstacleType::BOOMBABLE_WALL;
    }

    //Categoria con cuevas
    void transformToDungeonDoor(); // <--- Mazmorras
    void transformToCaveDoor(int target); // <--- Cuevas
    int getTargetId() const { return targetId; }

    ObstacleType getType() const { return type; }
    bool isDestroyed() const { return destroyed; }
    bool isOnFire() const { return onFire; }

    void takeDamage();
    void move(const sf::Vector2f& offset);
    void transformToPlatform();
    void ignite();
};

#endif
