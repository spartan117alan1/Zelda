#include "Game.h"
#include <iostream>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <fstream>
 /**void Game::initLevel(){
    // --- Nivel 1 ---
    Level level1;
    level1.playerStart = {400.f, 300.f};

    // obstaculos
    level1.obstacles.emplace_back(300.f, 400.f, 64.f, 32.f);
    level1.obstacles.emplace_back(500.f, 350.f, 32.f, 64.f);
    level1.obstacles.emplace_back(150.f, 250.f, 128.f, 32.f);
    level1.obstacles.emplace_back(0.f, 0.f, 800.f, 32.f);

    //enemigos
    level1.enemies.emplace_back(200.f, 200.f);
    level1.enemies.emplace_back(600.f, 200.f);

    levels.push_back(level1);

    // --- Nivel 2 ---
    Level level2;
    level2.playerStart = {32.f, 500.f};
    level2.obstacles.emplace_back(100.f, 300.f, 32.f, 200.f);
    level2.obstacles.emplace_back(400.f, 150.f, 200.f, 32.f);
    level2.enemies.emplace_back(400.f, 400.f);
    level2.enemies.emplace_back(700.f, 100.f);

    levels.push_back(level2);
}
**/


void Game::loadLevelsFromJSON() {
    using json = nlohmann::json;

    std::ifstream file("assets/levels/levels.json");
    if (!file.is_open()) {
        std::cerr << "ERROR: No se pudo abrir levels.json\n";
        return;
    }

    json data;
    file >> data;

    for (auto& lvlJson : data["levels"]) {
        Level lvl;

        // --- PLAYER START ---
        lvl.playerStart.x = lvlJson["playerStart"][0].get<float>();
        lvl.playerStart.y = lvlJson["playerStart"][1].get<float>();

        // --- ENTRADAS ---
        lvl.entryFromLeft.x  = lvlJson["entryFromLeft"][0].get<float>();
        lvl.entryFromLeft.y  = lvlJson["entryFromLeft"][1].get<float>();

        lvl.entryFromRight.x = lvlJson["entryFromRight"][0].get<float>();
        lvl.entryFromRight.y = lvlJson["entryFromRight"][1].get<float>();

        // --- OBSTACLES ---
        lvl.obstacles.clear();
        for (auto& o : lvlJson["obstacles"]) {
            lvl.obstacles.emplace_back(
                o[0].get<float>(), // x
                o[1].get<float>(), // y
                o[2].get<float>(), // w
                o[3].get<float>()  // h
            );
        }

        // --- ENEMIES ---
        lvl.enemies.clear();
        for (auto& e : lvlJson["enemies"]) {
            lvl.enemies.emplace_back(
                e[0].get<float>(), // x
                e[1].get<float>()  // y
            );
        }

        levels.push_back(lvl);
    }

    std::cout << "Levels loaded: " << levels.size() << "\n";
}

Game::Game() 
    : window(sf::VideoMode(800, 600), "Zelda Prototype"), player(400.f, 300.f) 
{
    window.setFramerateLimit(60);
    loadLevelsFromJSON();
    currentLevel = 0;
    loadLevel(currentLevel);
    hud.emplace_back(0.f,0.f,800.f, 150.f);
}

void Game::loadLevel(int index) {
    if (index < 0 || index >= levels.size()) return;
    obstacles = levels[index].obstacles;
    enemies = levels[index].enemies;

    //no volver a crear un nuevo jugador, solo reposicionar el existente
    //player = Player(levels[index].playerStart.x, levels[index].playerStart.y);
    currentLevel = index; 
}

void Game::run() {
    sf::Font font;
    if (!font.loadFromFile("assets/fonts/arial.ttf")) {
        std::cerr << "No se pudo cargar la fuente.\n";
    }
    bool isPaused = false;
    std::vector<sf::Vector2f> droppedHearts;
    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed)
                window.close();
        }
        //Pausa
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
                isPaused = !isPaused;
                sf::sleep(sf::milliseconds(200)); // prevent rapid toggle
            }

            float dt = clock.restart().asSeconds();


        // --- Reinicio manual ---
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
            resetGame();
            continue;
        }

        // --- Si el jugador muere, mostrar Game Over ---
        if (player.getHearts() <= 0) {
            window.clear(sf::Color::Black);
            sf::Text gameOver("GAME OVER\nPress R to restart", font, 40);
            gameOver.setFillColor(sf::Color::White);
            gameOver.setPosition(200.f, 250.f);
            window.draw(gameOver);
            window.display();
            continue;
        }

        //UPDATE (si no esta pausado)
         if (!isPaused) {
            // Input + movimiento
            player.handleInput(dt);
            player.update(dt, window, obstacles, hud, enemies);

            // --- Cambio de nivel ---
            if (player.getPosition().x > 780.f) { // derecha
                droppedHearts.clear();
                int next = (currentLevel + 1) % levels.size();
                loadLevel(next);
                player.setPosition(levels[next].entryFromLeft);
                sf::sleep(sf::milliseconds(300));
            }
            else if (player.getPosition().x < 20.f) { // izquierda
                droppedHearts.clear();
                int prev = (currentLevel - 1 + levels.size()) % levels.size();
                loadLevel(prev);
                player.setPosition(levels[prev].entryFromRight);
                sf::sleep(sf::milliseconds(300));
            }

        // --- Disparo ---
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            player.shoot();
        }
        // --- Attack ---
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            player.attack();
        }

        // --- Actualización de enemigos ---
        for (auto& e : enemies) {
                e.update(dt, player.getPosition(), obstacles, hud);

                if (e.getBounds().intersects(player.getBounds()) && e.canAttack()) {
                    sf::Vector2f knock = player.getPosition() - e.getPosition();
                    float len = std::sqrt(knock.x * knock.x + knock.y * knock.y);
                    if (len > 0) knock /= len;
                    player.takeDamage(1);
                    player.applynockback(knock);
                }
            }

        // --- Repulsión entre enemigos ---
        for (size_t i = 0; i < enemies.size(); ++i) {
            for (size_t j = i + 1; j < enemies.size(); ++j) {
                if (enemies[i].getBounds().intersects(enemies[j].getBounds())) {
                    sf::Vector2f dir = enemies[i].getPosition() - enemies[j].getPosition();
                    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                    if (len > 0.f) dir /= len;
                    enemies[i].move(dir * 2.f);
                    enemies[j].move(-dir * 2.f);
                }
            }
        }

        // --- Comprobar proyectiles contra enemigos ---
        // --- Comprobar daños a enemigos ---
for (auto ei = enemies.begin(); ei != enemies.end();) {

    bool damaged = false;

    // --- Proyectiles ---
    auto &projectiles = player.getProjectiles();
    for (auto pi = projectiles.begin(); pi != projectiles.end();) {

        if (pi->getBounds().intersects(ei->getBounds())) {

            ei->takeDamage(1);   // daño al enemigo
            pi = projectiles.erase(pi);   // eliminar proyectil
            damaged = true;

            if (ei->isDead()) {           // eliminar enemigo
                player.addScore(100);
                if (rand() % 100 < 30) {
                    droppedHearts.push_back(ei->getPosition());
                }
                ei = enemies.erase(ei);
            }
            break; // salir del loop de proyectiles
        }
        else {
            ++pi;
        }
    }

    if (damaged) {
        if (ei != enemies.end()) ++ei;
        continue;
    }

    // --- Espada melee ---
    if (player.isAttacking() && player.getSwordBounds().intersects(ei->getBounds())) {

        ei->takeDamage(1);

        if (ei->isDead()) {
            player.addScore(100);
            if (rand() % 100 < 30) {
                    droppedHearts.push_back(ei->getPosition());
                }
            ei = enemies.erase(ei);
        } else {
            ++ei;
        }

        continue;
    }

    // nada lo golpeó
    ++ei;
}
}
// --- Recoger corazones caídos ---
        for (auto it = droppedHearts.begin(); it != droppedHearts.end(); ) {
            // Creamos una hitbox imaginaria de 10x10 para el corazón en el suelo
            sf::FloatRect heartRect(it->x, it->y, 10.f, 10.f);
            
            if (player.getBounds().intersects(heartRect)) {
                player.heal(1); // Link se cura
                it = droppedHearts.erase(it); // El corazón desaparece del piso
            } else {
                ++it;
            }
        }

        // --- Renderizado ---
        window.clear(sf::Color(235, 175, 66));
        for (auto &ob : obstacles) ob.render(window);
        for (auto &hd : hud) hd.render(window);
        player.render(window);

        // Dibujar corazones tirados en el mapa
        for (const auto& pos : droppedHearts) {
            sf::CircleShape drop(6.f);
            drop.setFillColor(sf::Color(255, 50, 50));
            drop.setPosition(pos);
            window.draw(drop);
        }

        for (auto &e : enemies) e.render(window);

        // --- Dibujo de corazones de vida ---
        for (int i = 0; i < player.getHearts(); ++i) {
            sf::CircleShape heart(10.f);
            heart.setFillColor(sf::Color::Red);
            heart.setPosition(10.f + i * 25.f, 10.f);
            window.draw(heart);
        }
        // Etiqueta
        sf::Text heartLabel("x " + std::to_string(player.getHearts()), font, 20);
        heartLabel.setFillColor(sf::Color::White);
        heartLabel.setPosition(10.f, 40.f);
        window.draw(heartLabel);

        // Etiqueta del Marcador de Puntos
        sf::Text scoreLabel("Score: " + std::to_string(player.getScore()), font, 20);
        scoreLabel.setFillColor(sf::Color::White);
        scoreLabel.setPosition(150.f, 40.f); // Acomodado a la derecha de los corazones
        window.draw(scoreLabel);

        // Pause overlay
        if (isPaused) {
            sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
            overlay.setFillColor(sf::Color(0, 0, 0, 150));
            window.draw(overlay);

            sf::Text pauseText("PAUSED\nPress P to Resume", font, 36);
            pauseText.setFillColor(sf::Color::White);
            pauseText.setPosition(200.f, 250.f);
            window.draw(pauseText);
        }


        window.display();
    }
}

void Game::resetGame() {
    // Reset player state 
    player = Player(levels[currentLevel].playerStart.x, levels[currentLevel].playerStart.y);
    loadLevel(currentLevel);
}