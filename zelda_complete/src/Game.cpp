#include "Game.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <nlohmann/json.hpp>
#include <fstream>

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
        if (lvlJson.contains("playerStart")) {
            lvl.playerStart.x = lvlJson["playerStart"][0].get<float>();
            lvl.playerStart.y = lvlJson["playerStart"][1].get<float>();
        }
        if (lvlJson.contains("entryFromLeft")) {
            lvl.entryFromLeft.x  = lvlJson["entryFromLeft"][0].get<float>();
            lvl.entryFromLeft.y  = lvlJson["entryFromLeft"][1].get<float>();
        }
        if (lvlJson.contains("entryFromRight")) {
            lvl.entryFromRight.x = lvlJson["entryFromRight"][0].get<float>();
            lvl.entryFromRight.y = lvlJson["entryFromRight"][1].get<float>();
        }
        if (lvlJson.contains("entryFromTop")) {
            lvl.entryFromTop.x = lvlJson["entryFromTop"][0].get<float>();
            lvl.entryFromTop.y = lvlJson["entryFromTop"][1].get<float>();
        }
        if (lvlJson.contains("entryFromBottom")) {
            lvl.entryFromBottom.x = lvlJson["entryFromBottom"][0].get<float>();
            lvl.entryFromBottom.y = lvlJson["entryFromBottom"][1].get<float>();
        }

        lvl.obstacles.clear();

        if (lvlJson.contains("boombableWalls")) {
            for (auto& b : lvlJson["boombableWalls"]) {
                // Formato esperado en JSON: [x, y, w, h, targetId] (si targetId es -1 es dungeon door, si es >= 0 es cueva con ID)
                float x = b[0].get<float>();
                float y = b[1].get<float>();
                float w = b[2].get<float>();
                float h = b[3].get<float>();
                int target = b.size() > 4 ? b[4].get<int>() : -1; // -1 por defecto (mazmorra o sin cueva específica)
                
                lvl.obstacles.emplace_back(x, y, w, h, ObstacleType::BOOMBABLE_WALL, target);
            }
        }

        if (lvlJson.contains("obstacles")) {
            for (auto& o : lvlJson["obstacles"]) lvl.obstacles.emplace_back(o[0].get<float>(), o[1].get<float>(), o[2].get<float>(), o[3].get<float>());
        }
        if (lvlJson.contains("water")) {
            for (auto& w : lvlJson["water"]) lvl.obstacles.emplace_back(w[0].get<float>(), w[1].get<float>(), w[2].get<float>(), w[3].get<float>(), true);
        }
        if (lvlJson.contains("trees")) {
            for (auto& t : lvlJson["trees"]) lvl.obstacles.emplace_back(t[0].get<float>(), t[1].get<float>(), t[2].get<float>(), t[3].get<float>(), ObstacleType::TREE);
        }
        if (lvlJson.contains("jars")) {
            for (auto& j : lvlJson["jars"]) lvl.obstacles.emplace_back(j[0].get<float>(), j[1].get<float>(), j[2].get<float>(), j[3].get<float>(), ObstacleType::JAR);
        }
        if (lvlJson.contains("trunks")) {
            for (auto& t : lvlJson["trunks"]) lvl.obstacles.emplace_back(t[0].get<float>(), t[1].get<float>(), t[2].get<float>(), t[3].get<float>(), ObstacleType::TRUNK);
        }
        if (lvlJson.contains("dungeon_door")) {
            for (auto& d : lvlJson["dungeon_door"]) lvl.obstacles.emplace_back(d[0].get<float>(), d[1].get<float>(), d[2].get<float>(), d[3].get<float>(), ObstacleType::DUNGEON_DOOR);
        }
        if (lvlJson.contains("cave_door")) {
            for (auto& c : lvlJson["cave_door"]) lvl.obstacles.emplace_back(c[0].get<float>(), c[1].get<float>(), c[2].get<float>(), c[3].get<float>(), ObstacleType::CAVE_DOOR);
        }
        
        lvl.enemies.clear();
        if (lvlJson.contains("enemies")) {
            for (auto& e : lvlJson["enemies"]) lvl.enemies.emplace_back(e[0].get<float>(), e[1].get<float>());
        }
        levels.push_back(lvl);
    }
    std::cout << "Levels loaded: " << levels.size() << "\n";
}

Game::Game() : window(sf::VideoMode(800, 600), "Zelda Prototype"), player(400.f, 300.f) {
    window.setFramerateLimit(60);
    loadLevelsFromJSON();
    currentLevel = 5; 
    loadLevel(currentLevel);
    hud.emplace_back(0.f,0.f,800.f, 150.f);
}

void Game::loadLevel(int index) {
    if (index < 0 || index >= levels.size()) return;
    obstacles = levels[index].obstacles;
    enemies = levels[index].enemies;

    // Solo creamos Zoras si hay agua en la habitación
    zoras.clear();
    bool hasWater = false;
    for (auto& obs : obstacles) {
        if (obs.getIsWater()) { hasWater = true; break; }
    }
    if (hasWater) zoras.emplace_back(obstacles);

    

    currentLevel = index; 
}

void Game::run() {
    sf::Font font;
    if (!font.loadFromFile("assets/fonts/Minecraft.ttf")) std::cerr << "No se pudo cargar la fuente.\n";

    if (fieldMusic.openFromFile("assets/audio/field.wav")) { fieldMusic.setLoop(true); fieldMusic.play(); }
    if (rupeeBuffer.loadFromFile("assets/audio/pickRupee.wav")) rupeeSound.setBuffer(rupeeBuffer);
    if (lowHealthBuffer.loadFromFile("assets/audio/lowHearth.wav")) { lowHealthSound.setBuffer(lowHealthBuffer); lowHealthSound.setLoop(true); }
    if (healBuffer.loadFromFile("assets/audio/heal.wav")) healSound.setBuffer(healBuffer);
    if (swordBuffer.loadFromFile("assets/audio/sword.wav")) swordSound.setBuffer(swordBuffer);
    if (damageBuffer.loadFromFile("assets/audio/damage.wav")) damageSound.setBuffer(damageBuffer);

    sf::Texture hudTexture;
    if (!hudTexture.loadFromFile("assets/textures/link.png")) std::cerr << "Error cargando textura para HUD.\n";

    sf::Sprite heartSprite(hudTexture); heartSprite.setTextureRect(sf::IntRect(266, 242, 12, 12)); heartSprite.setScale(2.f, 2.f); 
    sf::Sprite heartContainerSprite(hudTexture); heartContainerSprite.setTextureRect(sf::IntRect(266, 255, 12, 12)); heartContainerSprite.setScale(2.f, 2.f);
    sf::Sprite emptyHeartSprite(hudTexture); emptyHeartSprite.setTextureRect(sf::IntRect(266, 268, 12, 12)); emptyHeartSprite.setScale(2.f, 2.f);

    sf::Sprite rupee1Sprite(hudTexture); rupee1Sprite.setTextureRect(sf::IntRect(280, 242, 12, 12)); rupee1Sprite.setScale(2.f, 2.f);
    sf::Sprite rupee5Sprite(hudTexture); rupee5Sprite.setTextureRect(sf::IntRect(280, 255, 12, 12)); rupee5Sprite.setScale(2.f, 2.f);
    sf::Sprite rupee20Sprite(hudTexture); rupee20Sprite.setTextureRect(sf::IntRect(280, 268, 12, 12)); rupee20Sprite.setScale(2.f, 2.f);
    sf::Sprite rupee50Sprite(hudTexture); rupee50Sprite.setTextureRect(sf::IntRect(280, 281, 12, 12)); rupee50Sprite.setScale(2.f, 2.f);
    sf::Sprite rupee100Sprite(hudTexture); rupee100Sprite.setTextureRect(sf::IntRect(280, 294, 12, 12)); rupee100Sprite.setScale(2.f, 2.f);

    // 1. DEFINIR EL CATÁLOGO DE ÍTEMS FÍSICOS (Aquí agregamos Sword, Candle y Bow)
    enum class ItemType { Heart, HeartContainer, Rupee1, Rupee5, Rupee20, Rupee50, Rupee100, Sword, MagicalSword, Candle, Bow, Key, Bomb , IceRod, Boomerang, Triforce};
    struct DroppedItem { sf::Vector2f pos; ItemType type; };
    std::vector<DroppedItem> droppedItems;

    // --- NUEVAS ESTRUCTURAS PARA ITEMS SECUNDARIOS ---
    struct ActiveBomb {
        sf::Vector2f pos;
        float fuseTimer = 2.f; // 2 segundos antes de explotar
        bool exploded = false;
    };

    struct ActiveArrow {
        sf::Vector2f pos;
        sf::Vector2f velocity;
        int direction; // 0: Arriba, 1: Abajo, 2: Izquierda, 3: Derecha
    };

    struct ThrownBoomerang {
        sf::Vector2f pos;
        sf::Vector2f targetPos;
        bool returning = false;
        sf::RectangleShape hitbox;
    };

    struct ActiveIce {
        sf::Vector2f pos;
        sf::Vector2f velocity;
    };
    std::vector<ActiveIce> activeIces;

    // Colecciones activas en el nivel
    std::vector<ActiveBomb> activeBombs;
    std::vector<ActiveArrow> activeArrows;
    std::vector<ThrownBoomerang> activeBoomerangs;

    // 2. VARIABLES DE PAUSA E INVENTARIO (Aquí declaramos cursor e inputDelay)
    bool isPaused = false;
    int cursorX = 0; 
    float scrollY = 0.f;
    float inputDelay = 0.f; 
    sf::Vector2f lastDir(0.f, 1.f); // Dirección para el fuego


    // 3. CÓDIGO DE PRUEBA: Soltar los ítems en el piso al arrancar el juego
    //BORRAR CUANDO ESTEN ESTABLECIDOS LOS ÍTEMS EN EL JUEGO
    droppedItems.push_back({sf::Vector2f(350.f, 400.f), ItemType::Sword});
    droppedItems.push_back({sf::Vector2f(400.f, 400.f), ItemType::Candle});
    droppedItems.push_back({sf::Vector2f(450.f, 400.f), ItemType::Bow});
    droppedItems.push_back({sf::Vector2f(500.f, 400.f), ItemType::Key});
    droppedItems.push_back({sf::Vector2f(550.f, 400.f), ItemType::HeartContainer});
    droppedItems.push_back({sf::Vector2f(600.f, 400.f), ItemType::IceRod});
    droppedItems.push_back({sf::Vector2f(650.f, 400.f), ItemType::Boomerang});
    droppedItems.push_back({sf::Vector2f(700.f, 400.f), ItemType::Bomb});
    droppedItems.push_back({sf::Vector2f(300.f, 400.f), ItemType::MagicalSword});


    sf::RectangleShape fireHitbox(sf::Vector2f(48.f, 48.f));
    fireHitbox.setFillColor(sf::Color(255, 50, 0, 180));
    fireHitbox.setOutlineThickness(2.f);
    fireHitbox.setOutlineColor(sf::Color::Yellow);
    bool fireActive = false;
    float fireDuration = 0.f;

   auto drawHUD = [&]() {
        sf::RectangleShape hudBg(sf::Vector2f(800.f, 150.f)); hudBg.setFillColor(sf::Color::Black); window.draw(hudBg);
        sf::Text levelText("ROOM " + std::to_string(currentLevel), font, 18); levelText.setFillColor(sf::Color::White); levelText.setPosition(40.f, 20.f); window.draw(levelText);
        sf::RectangleShape mapBox(sf::Vector2f(120.f, 70.f)); mapBox.setFillColor(sf::Color(100, 100, 100)); mapBox.setOutlineThickness(2.f); mapBox.setOutlineColor(sf::Color::White); mapBox.setPosition(40.f, 50.f); window.draw(mapBox);
        sf::RectangleShape mapPlayerPoint(sf::Vector2f(15.f, 15.f)); mapPlayerPoint.setFillColor(sf::Color::Green);
        int col = currentLevel % 4; int row = currentLevel / 4; 
        mapPlayerPoint.setPosition(40.f + (col * 30.f) + 12.f, 50.f + (row * 15.f) + 4.5f); window.draw(mapPlayerPoint);

       // Rupias
        rupee1Sprite.setPosition(220.f, 30.f); window.draw(rupee1Sprite);
        sf::Text rupeeText("X " + std::to_string(player.getRupees()), font, 18); 
        rupeeText.setFillColor(sf::Color::White); rupeeText.setPosition(260.f, 30.f); window.draw(rupeeText);

        // Llaves
        sf::Sprite keyHudSprite(hudTexture);
        keyHudSprite.setTextureRect(sf::IntRect(295, 257, 12, 12));
        keyHudSprite.setScale(2.f, 2.f);
        keyHudSprite.setPosition(220.f, 60.f);
        window.draw(keyHudSprite);
        
        sf::Text keysText("X " + std::to_string(player.getKeys()), font, 18); 
        keysText.setFillColor(sf::Color::White); keysText.setPosition(260.f, 60.f); window.draw(keysText);

        // Bombas
        sf::Sprite bombHudSprite(hudTexture);
        bombHudSprite.setTextureRect(sf::IntRect(295, 270, 12, 12));
        bombHudSprite.setScale(2.f, 2.f);
        bombHudSprite.setPosition(220.f, 90.f);
        window.draw(bombHudSprite);
        
        sf::Text bombsText("X " + std::to_string(player.getBombs()), font, 18); 
        bombsText.setFillColor(sf::Color::White); bombsText.setPosition(260.f, 90.f); window.draw(bombsText);

        sf::Text textB("B", font, 16); textB.setFillColor(sf::Color::White); textB.setPosition(415.f, 25.f); window.draw(textB);
        sf::RectangleShape boxB(sf::Vector2f(40.f, 50.f)); boxB.setFillColor(sf::Color::Transparent); boxB.setOutlineColor(sf::Color::White); boxB.setOutlineThickness(2.f); boxB.setPosition(400.f, 50.f); window.draw(boxB);
        sf::Text textA("A", font, 16); textA.setFillColor(sf::Color::White); textA.setPosition(485.f, 25.f); window.draw(textA);
        sf::RectangleShape boxA(sf::Vector2f(40.f, 50.f)); boxA.setFillColor(sf::Color::Transparent); boxA.setOutlineColor(sf::Color::White); boxA.setOutlineThickness(2.f); boxA.setPosition(470.f, 50.f); window.draw(boxA);
        
        // --- DIBUJAR ÍTEM EQUIPADO EN B ---
        sf::Sprite bSprite(hudTexture);
        bSprite.setScale(2.f, 2.f); 
        bool hasBItem = true;

        if (player.getItemB() == Player::EquipItem::CANDLE) {
            bSprite.setTextureRect(sf::IntRect(319, 186, 8, 15));
            bSprite.setPosition(412.f, 57.f); 
        } else if (player.getItemB() == Player::EquipItem::BOW) {
            bSprite.setTextureRect(sf::IntRect(309, 186, 8, 15));
            bSprite.setPosition(412.f, 57.f);
        } else if (player.getItemB() == Player::EquipItem::BOMB) {
            bSprite.setTextureRect(sf::IntRect(295, 269, 12, 12));
            bSprite.setPosition(408.f, 60.f);
        } else if (player.getItemB() == Player::EquipItem::ICE_ROD) {
            bSprite.setTextureRect(sf::IntRect(145, 155, 8, 15));
            bSprite.setPosition(412.f, 57.f);
        }   else if (player.getItemB() == Player::EquipItem::BOOMERANG) {
            bSprite.setTextureRect(sf::IntRect(64, 187, 8, 12));
            bSprite.setPosition(412.f, 57.f);
        } 
        else {
            hasBItem = false;
        }

        if (hasBItem) window.draw(bSprite);


        // --- DIBUJAR ÍTEM EQUIPADO EN A (ESPADAS) ---
        if (player.getItemA() == Player::EquipItem::WOODEN_SWORD || player.getItemA() == Player::EquipItem::MAGICAL_SWORD) {
            sf::Sprite swordHudSprite(hudTexture); 
            swordHudSprite.setTextureRect(sf::IntRect(1, 155, 8, 16)); 
            swordHudSprite.setScale(2.f, 2.f); 
            swordHudSprite.setPosition(482.f, 57.f); 
            
            if (player.getItemA() == Player::EquipItem::MAGICAL_SWORD) swordHudSprite.setTextureRect(sf::IntRect(36, 155, 8, 16)); // Cambiar a la espada mágica
            window.draw(swordHudSprite);
        }

        sf::Text lifeText("- LIFE -", font, 18); lifeText.setFillColor(sf::Color::Red); lifeText.setPosition(630.f, 25.f); window.draw(lifeText);
        for (int i = 0; i < player.getMaxHealth(); ++i) {
            if (i >= player.getHearts()) { emptyHeartSprite.setPosition(630.f + (i * 26.f), 55.f); window.draw(emptyHeartSprite); } 
            else { heartSprite.setPosition(630.f + (i * 26.f), 55.f); window.draw(heartSprite); }
        }
        sf::Text scoreLabel("SCORE: " + std::to_string(player.getScore()), font, 18); scoreLabel.setFillColor(sf::Color::White); scoreLabel.setPosition(630.f, 95.f); window.draw(scoreLabel);
    };

    auto transitionScreen = [&](int nextLvl, sf::Vector2f startPos, float dx, float dy) {
        sf::Vector2f oldPlayerPos = player.getPosition();
        sf::RenderTexture oldTex, newTex; oldTex.create(800, 450); newTex.create(800, 450);
        sf::View view(sf::FloatRect(0, 150, 800, 450)); oldTex.setView(view); newTex.setView(view);

        oldTex.clear(sf::Color(235, 175, 66));
        for (auto &ob : obstacles) ob.render(oldTex);
        for (auto &e : enemies) e.render(oldTex);
        for (const auto& item : droppedItems) {
            if (item.type == ItemType::Heart) { heartSprite.setPosition(item.pos); oldTex.draw(heartSprite); }
            else if (item.type == ItemType::HeartContainer) { heartContainerSprite.setPosition(item.pos); oldTex.draw(heartContainerSprite); }
            else if (item.type == ItemType::Rupee1) { rupee1Sprite.setPosition(item.pos); oldTex.draw(rupee1Sprite); }
            // Dibujamos las demás igual (simplificado en la pantalla de transición)
        }
        oldTex.display();

        // --- MAGIA: GUARDAR EL ESTADO DEL CUARTO ANTES DE SALIR ---
        //levels[currentLevel].obstacles = obstacles; // Guarda los árboles talados
        //levels[currentLevel].enemies = enemies;     // Guarda los enemigos que dejaste vivos (los muertos ya no están)
        
        loadLevel(nextLvl); 
        player.setPosition(startPos); 
        droppedItems.clear();
        enemyProjectiles.clear();
        player.getProjectiles().clear();
        fireActive = false;

        newTex.clear(sf::Color(235, 175, 66));
        for (auto &ob : obstacles) ob.render(newTex);
        for (auto &e : enemies) e.render(newTex);
        newTex.display();

        sf::Sprite oldSpr(oldTex.getTexture()); sf::Sprite newSpr(newTex.getTexture());
        oldSpr.setPosition(0, 150); newSpr.setPosition(dx * 800.f, 150 + dy * 450.f);

        int frames = 35;
        float moveX = -(dx * 800.f) / frames; float moveY = -(dy * 450.f) / frames;
        sf::Vector2f playerAnimatedPos = oldPlayerPos;
        float playerMoveX = (startPos.x - oldPlayerPos.x) / frames; float playerMoveY = (startPos.y - oldPlayerPos.y) / frames;

        for (int i = 0; i < frames; ++i) {
            oldSpr.move(moveX, moveY); newSpr.move(moveX, moveY);
            playerAnimatedPos.x += playerMoveX; playerAnimatedPos.y += playerMoveY;

            window.clear(sf::Color::Black);
            window.draw(oldSpr); window.draw(newSpr);
            
            player.setPosition(playerAnimatedPos); player.render(window); 
            drawHUD(); window.display(); sf::sleep(sf::milliseconds(16)); 
        }
        player.setPosition(startPos); clock.restart(); 
    };

    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) { if (ev.type == sf::Event::Closed) window.close(); }
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) { isPaused = !isPaused; sf::sleep(sf::milliseconds(200)); }

        float dt = clock.restart().asSeconds();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) { resetGame(); continue; }

        // --- ANIMACIÓN DE BARRIDO ---
        if (isPaused && scrollY < 600.f) {
            scrollY += 500.f * dt; // Velocidad de deslizamiento
            if (scrollY > 600.f) scrollY = 600.f;
        }
        else if (!isPaused && scrollY > 0.f) {
            scrollY -= 500.f * dt;
            if (scrollY < 0.f) scrollY = 0.f;
        }

        if (player.getHearts() <= 0) {
            window.clear(sf::Color::Black);
            sf::Text gameOver("GAME OVER\nPress R to restart", font, 40); gameOver.setFillColor(sf::Color::White); gameOver.setPosition(200.f, 250.f);
            window.draw(gameOver); window.display();
            continue;
        }

        if (!isPaused) {
            player.handleInput(dt);
            player.update(dt, window, obstacles, hud, enemies);

            // --- ACTUALIZAR DIRECCIÓN DE LINK ---
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) lastDir = sf::Vector2f(0.f, -1.f);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) lastDir = sf::Vector2f(0.f, 1.f);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) lastDir = sf::Vector2f(-1.f, 0.f);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) lastDir = sf::Vector2f(1.f, 0.f);

            // MODO DIOS (Tecla G)
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::G)) { 
                    player.heal(player.getMaxHealth());
                    player.addRupees(999);
                    player.addBombs(999);
                    player.addKeys(999);
                    player.addScore(99999);
                    player.addHeartContainer();
                    player.setItemA(Player::EquipItem::MAGICAL_SWORD);
                    player.hasMagicalSword = true;
                    player.hasCandle = true;
                    player.hasBow = true;
                    player.hasBomb = true;
                    player.hasBoomerang = true;
                    player.hasIceRod = true;
                    player.setItemB(Player::EquipItem::BOW);
                }

            // --- ACTUALIZAR OBSTÁCULOS (FUEGO Y PROPAGACIÓN) ---
            for (size_t i = 0; i < obstacles.size(); i++) {
                obstacles[i].update(dt);
                
                if (obstacles[i].isOnFire()) {
                    // 1. Daño a Link (Expandimos la hitbox del fuego 4 pixeles para atravesar la colisión sólida)
                    sf::FloatRect burnBox = obstacles[i].getBounds();
                    burnBox.left -= 4.f; 
                    burnBox.top -= 4.f;
                    burnBox.width += 8.f; 
                    burnBox.height += 8.f;

                    if (burnBox.intersects(player.getBounds())) {
                        sf::Vector2f knock = player.getPosition() - sf::Vector2f(obstacles[i].getBounds().left, obstacles[i].getBounds().top);
                        float len = std::sqrt(knock.x * knock.x + knock.y * knock.y);
                        if (len > 0) knock /= len;
                        player.takeDamage(1); 
                        player.applynockback(knock); 
                        damageSound.play();
                    }

                    // 2. Propagación lenta a otros árboles
                    sf::FloatRect heatAura = obstacles[i].getBounds();
                    heatAura.left -= 16.f; heatAura.top -= 16.f; 
                    heatAura.width += 32.f; heatAura.height += 32.f;

                    for (size_t j = 0; j < obstacles.size(); j++) {
                        if (i != j && obstacles[j].getType() == ObstacleType::TREE && !obstacles[j].isOnFire()) {
                            if (heatAura.intersects(obstacles[j].getBounds())) {
                                if (rand() % 1000 < 10) obstacles[j].ignite(); 
                            }
                        }
                    }
                }
            }

            // --- ACTUALIZAR HIELO ---
            for (auto it = activeIces.begin(); it != activeIces.end(); ) {
                it->pos += it->velocity * dt;
                sf::FloatRect iceBox(it->pos.x, it->pos.y, 16.f, 16.f);
                bool hit = false;

                for (auto& e : enemies) {
                    if (iceBox.intersects(e.getBounds())) {
                        e.applyStun(7.f); // Aplica aturdimiento por 2 segundos
                        e.takeDamage(0); // O aplica congelación
                        hit = true;
                        break;
                    }
                }

                if (hit || it->pos.x < 0 || it->pos.x > 800 || it->pos.y < 150 || it->pos.y > 600) {
                    it = activeIces.erase(it);
                } else {
                    ++it;
                }
            }

           // --- LÓGICA DE LA HITBOX DE FUEGO ---
            if (fireActive) {
                fireDuration -= dt; // Esto hace que el fuego se acabe
                for (auto& obs : obstacles) {
                    if (obs.getType() == ObstacleType::TREE && fireHitbox.getGlobalBounds().intersects(obs.getBounds())) obs.ignite();
                }
                for (auto& e : enemies) {
                    if (fireHitbox.getGlobalBounds().intersects(e.getBounds())) e.takeDamage(10); 
                }
                for (auto& z : zoras) {
                    if (fireHitbox.getGlobalBounds().intersects(z.getBounds())) z.takeDamage(10);
                }
                if (fireDuration <= 0.f) fireActive = false; // Se apaga cuando se acaba el tiempo
            }

           // --- ACTUALIZAR FLECHAS ---
            for (auto it = activeArrows.begin(); it != activeArrows.end(); ) {
                it->pos += it->velocity * dt;
                
                sf::FloatRect arrowBox(it->pos.x, it->pos.y, 16.f, 16.f);
                bool hit = false;
                
                // Daño a enemigos terrestres (NO toca árboles ni jarras)
                for (auto& e : enemies) {
                    if (arrowBox.intersects(e.getBounds())) {
                        e.takeDamage(2);
                        hit = true;
                        break;
                    }
                }
                // Daño a Zoras
                if (!hit) {
                    for (auto& z : zoras) {
                        if (arrowBox.intersects(z.getBounds())) {
                            z.takeDamage(2);
                            hit = true;
                            break;
                        }
                    }
                }
                
                if (hit || it->pos.x < 0 || it->pos.x > 800 || it->pos.y < 150 || it->pos.y > 600) {
                    it = activeArrows.erase(it);
                } else {
                    ++it;
                }
            }

            // --- ACTUALIZAR BOMBAS (CUENTA REGRESIVA Y EXPLOSIÓN) ---
            for (auto it = activeBombs.begin(); it != activeBombs.end(); ) {
                it->fuseTimer -= dt;
                if (it->fuseTimer <= 0.f && !it->exploded) {
                    it->exploded = true;
                    
                    // Rango de explosión (destruir obstáculos / abrir puertas secretas)
                    sf::FloatRect blastZone(it->pos.x - 32.f, it->pos.y - 32.f, 64.f, 64.f);
                    
                    for (auto& obs : obstacles) {
                        if (blastZone.intersects(obs.getBounds())) {
                            if (obs.getType() == ObstacleType::BOOMBABLE_WALL) {
                                int caveTarget = obs.getTargetId();
                                if (caveTarget >= 0) {
                                    obs.transformToCaveDoor(caveTarget); // Se convierte en entrada de cueva con ID
                                } else {
                                    obs.transformToDungeonDoor();        // Se convierte en puerta de mazmorra
                                }
                            }
                        }
                    }
                    
                    // Daño a enemigos cercanos por explosión
                    for (auto& e : enemies) {
                        if (blastZone.contains(e.getPosition())) {
                            e.takeDamage(5); // Gran daño de bomba
                        }
                    }
                }
                
                if (it->fuseTimer <= -0.3f) { // 0.3 segundos de animación de explosión
                    it = activeBombs.erase(it);
                } else {
                    ++it;
                }
            }

            // --- ACTUALIZAR BUMERÁN ---
            for (auto it = activeBoomerangs.begin(); it != activeBoomerangs.end(); ) {
                if (!it->returning) {
                    // Va hacia adelante
                    sf::Vector2f dir = it->targetPos - it->pos;
                    float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                    if (dist < 10.f) {
                        it->returning = true;
                    } else {
                        it->pos += (dir / dist) * 400.f * dt;
                    }
                } else {
                    // Regresa hacia Link
                    sf::Vector2f dir = player.getPosition() - it->pos;
                    float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                    if (dist < 15.f) {
                        it = activeBoomerangs.erase(it); // Lo atrapa de vuelta
                        continue;
                    } else {
                        it->pos += (dir / dist) * 400.f * dt;
                    }
                }

                // Aturdir enemigos al contacto (teñir de azul temporalmente en su render)
                it->hitbox.setPosition(it->pos);
                for (auto& e : enemies) {
                    if (it->hitbox.getGlobalBounds().intersects(e.getBounds())) {
                        // Aquí puedes activar un flag de aturdimiento en tu clase Enemy si lo deseas
                        e.applyStun(3.f); // Aturde por 3 segundos
                    }
                }
                for (auto& z : zoras) {
                    if (it->hitbox.getGlobalBounds().intersects(z.getBounds())) {
                        z.applyStun(3.f); // Aturde por 3 segundos
                    }
                }
                ++it;
            }

            // --- USO DEL BOTÓN B (Ítems Secundarios - Tecla C) ---
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) {
                
                // VELA (CANDLE) -> Fuego direccional
                if (player.getItemB() == Player::EquipItem::CANDLE && !fireActive) {
                    fireActive = true;
                    fireDuration = 0.3f; 
                    float fireX = player.getPosition().x + (lastDir.x * 40.f) - 8.f; 
                    float fireY = player.getPosition().y + (lastDir.y * 40.f) - 8.f;
                    fireHitbox.setPosition(fireX, fireY);
                }
                
                // ARCO (BOW) -> Consume 1 rupia y dispara flecha
                else if (player.getItemB() == Player::EquipItem::BOW) {
                    if (player.getRupees() >= 1 && activeArrows.empty()) { // Límite de 1 flecha a la vez por ahora
                        player.addRupees(-1); // Gasta 1 rupia
                        
                        sf::Vector2f arrowVel(0.f, 0.f);
                        int dirInt = 0;
                        if (lastDir.y < 0.f) { arrowVel = {0.f, -400.f}; dirInt = 0; }
                        else if (lastDir.y > 0.f) { arrowVel = {0.f, 400.f}; dirInt = 1; }
                        else if (lastDir.x < 0.f) { arrowVel = {-400.f, 0.f}; dirInt = 2; }
                        else if (lastDir.x > 0.f) { arrowVel = {400.f, 0.f}; dirInt = 3; }

                        activeArrows.push_back({player.getPosition(), arrowVel, dirInt});
                        rupeeSound.play(); // Sonido temporal de disparo
                    }
                }
                
                // BOMBA (BOMB) -> Coloca bomba en los pies de Link
                else if (player.getItemB() == Player::EquipItem::BOMB) {
                    if (player.getBombs() > 0 && activeBombs.empty()) {
                        player.addBombs(-1); // Gasta 1 bomba del inventario
                        activeBombs.push_back({player.getPosition(), 2.0f, false});
                    }
                }

                // VARITA DE HIELO (ICE ROD) -> Dispara proyectil de hielo
                else if (player.getItemB() == Player::EquipItem::ICE_ROD) {
                    if (activeIces.empty()) {
                        sf::Vector2f iceVel(0.f, 0.f);
                        if (lastDir.y < 0.f) iceVel = {0.f, -300.f};
                        else if (lastDir.y > 0.f) iceVel = {0.f, 300.f};
                        else if (lastDir.x < 0.f) iceVel = {-300.f, 0.f};
                        else if (lastDir.x > 0.f) iceVel = {300.f, 0.f};

                        activeIces.push_back({player.getPosition(), iceVel});
                    }
                }

                // BUMERÁN (BOOMERANG) -> Lanza el bumerán
                else if (player.getItemB() == Player::EquipItem::BOOMERANG) {
                    if (activeBoomerangs.empty()) {
                        sf::Vector2f target = player.getPosition() + lastDir * 150.f; // Distancia máxima
                        activeBoomerangs.push_back({player.getPosition(), target, false, sf::RectangleShape(sf::Vector2f(16.f, 16.f))});
                    }
                }
            }

            // --- EMPUJAR TRONCOS ---
            for (auto& obs : obstacles) {
                if (obs.getType() == ObstacleType::TRUNK) {
                    sf::FloatRect pushBox = player.getBounds();
                    pushBox.left -= 4.f; pushBox.top -= 4.f; pushBox.width += 8.f; pushBox.height += 8.f;
                    
                    if (pushBox.intersects(obs.getBounds())) {
                        sf::Vector2f pushDir(0.f, 0.f);
                        sf::FloatRect pBounds = player.getBounds();
                        sf::FloatRect oBounds = obs.getBounds();
                        
                        // Validamos que Link esté empujando desde el lado correcto
                        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) && pBounds.top > oBounds.top) pushDir.y = -1.f;
                        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) && pBounds.top < oBounds.top) pushDir.y = 1.f;
                        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) && pBounds.left > oBounds.left) pushDir.x = -1.f;
                        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) && pBounds.left < oBounds.left) pushDir.x = 1.f;
                        
                        obs.move(pushDir * 60.f * dt); 
                        
                        sf::FloatRect bounds = obs.getBounds();
                        sf::Vector2f center(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
                        
                        // 1. Revisar si está resbalando sobre un puente (PLATFORM) ya creado
                        bool overPlatform = false;
                        for (auto& plat : obstacles) {
                            if (plat.getType() == ObstacleType::PLATFORM && plat.getBounds().contains(center)) {
                                overPlatform = true;
                                break;
                            }
                        }
                        
                        // 2. Si NO está sobre otro puente, revisar si su centro cayó al agua
                        if (!overPlatform) {
                            for (auto& waterObs : obstacles) {
                                if (waterObs.getIsWater() && waterObs.getBounds().contains(center)) {
                                    // SNAP MATEMÁTICO: Alineación milimétrica con la orilla o el siguiente tronco
                                    float wTop = waterObs.getBounds().top;
                                    float dy = obs.getBounds().top - wTop;
                                    float snapY = wTop + std::round(dy / 32.f) * 32.f;
                                    
                                    float wLeft = waterObs.getBounds().left;
                                    float dx = obs.getBounds().left - wLeft;
                                    float snapX = wLeft + std::round(dx / 32.f) * 32.f;
                                    
                                    obs.move(sf::Vector2f(snapX - obs.getBounds().left, snapY - obs.getBounds().top));
                                    obs.transformToPlatform(); 
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            int gridWidth = 4; int totalLevels = levels.size();

            // --- LÍMITES DE PANTALLA ---
            if (player.getPosition().x > 760.f) { 
                if (currentLevel % gridWidth != gridWidth - 1) { 
                    int next = currentLevel + 1; if (next < totalLevels) transitionScreen(next, levels[next].entryFromLeft, 1, 0); 
                } else { player.move(-5.f, 0); }
            }
            else if (player.getPosition().x < 15.f) { 
                if (currentLevel % gridWidth != 0) { 
                    int next = currentLevel - 1; if (next >= 0) transitionScreen(next, levels[next].entryFromRight, -1, 0); 
                } else { player.move(5.f, 0); }
            }
            else if (player.getPosition().y < 170.f) { 
                if (currentLevel >= gridWidth) { 
                    int next = currentLevel - gridWidth; if (next >= 0) transitionScreen(next, levels[next].entryFromBottom, 0, -1); 
                } else { player.move(0, 5.f); }
            }
            else if (player.getPosition().y > 560.f) { 
                if (currentLevel < totalLevels - gridWidth) { 
                    int next = currentLevel + gridWidth; if (next < totalLevels) transitionScreen(next, levels[next].entryFromTop, 0, 1); 
                } else { player.move(0, -5.f); }
            }

            // --- USO DEL BOTÓN A (Espada) ---
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                player.attack();
                if (player.isAttacking() && swordSound.getStatus() != sf::Sound::Playing) swordSound.play();
            }

            // --- COLISIONES CON OBSTÁCULOS (ESPADAS / JARRONES) ---
            for (auto oi = obstacles.begin(); oi != obstacles.end(); ) {
                bool obstacleHit = false;
                if (oi->isSolid()) {
                    auto &projectiles = player.getProjectiles();
                    for (auto pi = projectiles.begin(); pi != projectiles.end(); ) {
                        if (pi->getBounds().intersects(oi->getBounds())) {
                            if (oi->getType() == ObstacleType::TREE || oi->getType() == ObstacleType::JAR) { oi->takeDamage(); obstacleHit = true; }
                            pi = projectiles.erase(pi); break;
                        } else { ++pi; }
                    }

                    if (!obstacleHit && player.isAttacking() && player.getSwordBounds().intersects(oi->getBounds())) {
                        if (oi->getType() == ObstacleType::TREE || oi->getType() == ObstacleType::JAR) { oi->takeDamage(); obstacleHit = true; }
                    }
                }

                if (oi->isDestroyed()) {
                    if (oi->getType() == ObstacleType::JAR) {
                        int roll = rand() % 100;
                        if (roll < 20) droppedItems.push_back({sf::Vector2f(oi->getBounds().left, oi->getBounds().top), ItemType::Heart});
                        else if (roll < 60) {
                            int rupeeRoll = rand() % 100; ItemType rType = ItemType::Rupee1;
                            if (rupeeRoll >= 90) rType = ItemType::Rupee20; else if (rupeeRoll >= 60) rType = ItemType::Rupee5;
                            droppedItems.push_back({sf::Vector2f(oi->getBounds().left, oi->getBounds().top), rType});
                        }
                    }
                    oi = obstacles.erase(oi);
                } else { ++oi; }
            }

            // --- ACTUALIZAR ENEMIGOS TERRESTRES ---
            for (auto& e : enemies) {
                e.update(dt, player.getPosition(), obstacles, hud);
                if (e.getBounds().intersects(player.getBounds()) && e.canAttack()) {
                    sf::Vector2f knock = player.getPosition() - e.getPosition();
                    float len = std::sqrt(knock.x * knock.x + knock.y * knock.y);
                    if (len > 0) knock /= len;
                    player.takeDamage(1); player.applynockback(knock); damageSound.play(); 
                }
            }

            for (size_t i = 0; i < enemies.size(); ++i) {
                for (size_t j = i + 1; j < enemies.size(); ++j) {
                    if (enemies[i].getBounds().intersects(enemies[j].getBounds())) {
                        sf::Vector2f dir = enemies[i].getPosition() - enemies[j].getPosition();
                        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                        if (len > 0.f) dir /= len;
                        enemies[i].move(dir * 2.f); enemies[j].move(-dir * 2.f);
                    }
                }
            }

           // --- ACTUALIZAR ZORAS Y PROYECTILES ---
            for (auto& zora : zoras) zora.update(dt, player.getPosition(), enemyProjectiles, obstacles);

            for (size_t i = 0; i < enemyProjectiles.size();) {
                enemyProjectiles[i].update(dt); 
                bool erase = false;
                sf::Vector2f pos = enemyProjectiles[i].getPosition();
                
                if (pos.x < 0 || pos.y < 0 || pos.x > 800 || pos.y > 600) erase = true;
                
                if (!erase && enemyProjectiles[i].getBounds().intersects(player.getBounds())) { 
                    // Revisar si el escudo detiene el golpe
                    if (player.canBlock(enemyProjectiles[i].getBounds())) {
                        swordSound.play(); // Usamos el sonido de espada como "Cling!" metálico por ahora
                        erase = true;
                    } else {
                        player.takeDamage(1); 
                        damageSound.play(); 
                        erase = true; 
                    }
                }
                if (erase) enemyProjectiles.erase(enemyProjectiles.begin() + i); else i++;
            }

            // --- COMBATE: DAÑO A ENEMIGOS TERRESTRES Y MODO DIOS ---
            for (auto ei = enemies.begin(); ei != enemies.end(); ) {
                bool enemyDied = ei->isDead(); 
                
                auto &projectiles = player.getProjectiles();
                for (auto pi = projectiles.begin(); pi != projectiles.end(); ) {
                    if (pi->getBounds().intersects(ei->getBounds())) { ei->takeDamage(1); pi = projectiles.erase(pi); if (ei->isDead()) enemyDied = true; break; } 
                    else { ++pi; }
                }

                if (!enemyDied && player.isAttacking() && player.getSwordBounds().intersects(ei->getBounds())) { 
                    ei->takeDamage(1); if (ei->isDead()) enemyDied = true; 
                }
                
                // MODO DIOS (Tecla G)
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::G)) { 
                    enemyDied = true; 
                }

                if (enemyDied) {
                    player.addScore(100); 
                    int roll = rand() % 100;
                    if (roll < 5) droppedItems.push_back({ei->getPosition(), ItemType::HeartContainer});
                    else if (roll < 20) droppedItems.push_back({ei->getPosition(), ItemType::Heart});
                    else if (roll < 50) { 
                        int rupeeRoll = rand() % 100; ItemType rType = ItemType::Rupee1;
                        if (rupeeRoll >= 95) rType = ItemType::Rupee100; else if (rupeeRoll >= 85) rType = ItemType::Rupee50;  
                        else if (rupeeRoll >= 70) rType = ItemType::Rupee20; else if (rupeeRoll >= 40) rType = ItemType::Rupee5;   
                        droppedItems.push_back({ei->getPosition(), rType});
                    }
                    ei = enemies.erase(ei); 
                } else { ++ei; }
            }

            // --- COMBATE: DAÑO A ZORAS ---
            for (auto zi = zoras.begin(); zi != zoras.end(); ) {
                bool zoraDied = zi->isDead(); 
                auto &projectiles = player.getProjectiles();
                for (auto pi = projectiles.begin(); pi != projectiles.end(); ) {
                    if (pi->getBounds().intersects(zi->getBounds())) { zi->takeDamage(1); pi = projectiles.erase(pi); if (zi->isDead()) zoraDied = true; break; } 
                    else { ++pi; }
                }
                
                if (!zoraDied && player.isAttacking() && player.getSwordBounds().intersects(zi->getBounds())) { 
                    zi->takeDamage(1); if (zi->isDead()) zoraDied = true; 
                }
                
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::G)) { zoraDied = true; }

                if (zoraDied) { player.addScore(200); zi = zoras.erase(zi); } else { ++zi; }
            }

            // --- RECOGER ÍTEMS ---
            for (auto it = droppedItems.begin(); it != droppedItems.end(); ) {
                sf::FloatRect itemRect(it->pos.x, it->pos.y, 16.f, 16.f);
                if (player.getBounds().intersects(itemRect)) {
                    if (it->type == ItemType::Heart) { player.heal(1); healSound.play(); }
                    else if (it->type == ItemType::HeartContainer) { player.addHeartContainer(); healSound.play(); }
                    else if (it->type == ItemType::Rupee1) { player.addRupees(1); rupeeSound.play(); }
                    else if (it->type == ItemType::Rupee5) { player.addRupees(5); rupeeSound.play(); }
                    else if (it->type == ItemType::Rupee20) { player.addRupees(20); rupeeSound.play(); }
                    else if (it->type == ItemType::Rupee50) { player.addRupees(50); rupeeSound.play(); }
                    else if (it->type == ItemType::Rupee100) { player.addRupees(100); rupeeSound.play(); }

                    
                    else if (it->type == ItemType::Rupee50) { player.addRupees(50); rupeeSound.play(); }
                    else if (it->type == ItemType::Rupee100) { player.addRupees(100); rupeeSound.play(); }
                    
                    // NUEVOS OBJETOS DEL INVENTARIO
                    else if (it->type == ItemType::Sword) { 
                        player.hasWoodenSword = true; 
                        player.setItemA(Player::EquipItem::WOODEN_SWORD); // Te la equipa automáticamente
                        rupeeSound.play(); // Sonido provisional de "Item Get"
                    }
                    else if (it->type == ItemType::Candle) { 
                        player.hasCandle = true; 
                        if (player.getItemB() == Player::EquipItem::NONE) player.setItemB(Player::EquipItem::CANDLE);
                        rupeeSound.play(); 
                    }
                    else if (it->type == ItemType::Bow) { 
                        player.hasBow = true; 
                        if (player.getItemB() == Player::EquipItem::NONE) player.setItemB(Player::EquipItem::BOW);
                        rupeeSound.play(); 
                    }
                    else if (it->type == ItemType::Bomb) { 
                        player.hasBomb = true; // <-- Esto hace que aparezca en el menú
                        player.addBombs(1); // Suma la munición al HUD
                        if (player.getItemB() == Player::EquipItem::NONE) player.setItemB(Player::EquipItem::BOMB);
                        rupeeSound.play(); 
                    }
                    else if (it->type == ItemType::Boomerang) { 
                        player.hasBoomerang = true; 
                        if (player.getItemB() == Player::EquipItem::NONE) player.setItemB(Player::EquipItem::BOOMERANG);
                        rupeeSound.play(); 
                    }
                    else if (it->type == ItemType::IceRod) { 
                        player.hasIceRod = true; 
                        if (player.getItemB() == Player::EquipItem::NONE) player.setItemB(Player::EquipItem::ICE_ROD);
                        rupeeSound.play(); 
                    }
                    else if (it->type == ItemType::Triforce) { 
                        player.hasTriforce = true; 
                        rupeeSound.play(); 
                    }
                    else if (it->type == ItemType::Key) { 
                        player.addKeys(1); 
                        rupeeSound.play(); 
                    }
                    else if (it->type == ItemType::MagicalSword) { 
                        player.hasMagicalSword = true; 
                        player.setItemA(Player::EquipItem::MAGICAL_SWORD); // Sobrescribe la de madera automáticamente
                        rupeeSound.play(); 
                    }
                    
                    it = droppedItems.erase(it);
                } else { ++it; }
            }
        }
        
        if (player.getHearts() == 1 && lowHealthSound.getStatus() != sf::Sound::Playing) lowHealthSound.play();
        else if ((player.getHearts() > 1 || player.getHearts() == 0) && lowHealthSound.getStatus() == sf::Sound::Playing) lowHealthSound.stop();
        
        // --- APLICAR CÁMARA PARA EL BARRIDO DEL JUEGO ---
        sf::View defaultView = window.getDefaultView();
        sf::View gameView = defaultView;
        gameView.move(0.f, -scrollY);
        window.setView(gameView);

        // Como movimos la cámara, el fondo amarillo debe ser un rectángulo fijo
        window.clear(sf::Color::Black); 
        sf::RectangleShape gameBg(sf::Vector2f(800.f, 600.f));
        gameBg.setFillColor(sf::Color(235, 175, 66));
        window.draw(gameBg);
        
        for (auto &ob : obstacles) ob.render(window);
        for (const auto& zora : zoras) zora.render(window);
        for (const auto& ep : enemyProjectiles) ep.render(window);
        
        for (const auto& item : droppedItems) {
            if (item.type == ItemType::Heart) { heartSprite.setPosition(item.pos); window.draw(heartSprite); }
            else if (item.type == ItemType::HeartContainer) { heartContainerSprite.setPosition(item.pos); window.draw(heartContainerSprite); }
            else if (item.type == ItemType::Rupee1) { rupee1Sprite.setPosition(item.pos); window.draw(rupee1Sprite); }
            else if (item.type == ItemType::Rupee5) { rupee5Sprite.setPosition(item.pos); window.draw(rupee5Sprite); }
            else if (item.type == ItemType::Rupee20) { rupee20Sprite.setPosition(item.pos); window.draw(rupee20Sprite); }
            else if (item.type == ItemType::Rupee50) { rupee50Sprite.setPosition(item.pos); window.draw(rupee50Sprite); }
            else if (item.type == ItemType::Rupee100) { rupee100Sprite.setPosition(item.pos); window.draw(rupee100Sprite); }

            // DIBUJAR LOS NUEVOS OBJETOS EN EL SUELO
            else if (item.type == ItemType::Sword || item.type == ItemType::MagicalSword || 
                     item.type == ItemType::Candle || item.type == ItemType::Bow || 
                     item.type == ItemType::Bomb || item.type == ItemType::IceRod || 
                     item.type == ItemType::Triforce || item.type == ItemType::Boomerang ||
                     item.type == ItemType::Key) {
                
                // 1. AQUI NACE LA VARIABLE (Esto faltaba)
                sf::Sprite dropSprite(hudTexture);
                dropSprite.setScale(2.f, 2.f);
                dropSprite.setPosition(item.pos);

                // 2. Coordenadas con Y+1
                if (item.type == ItemType::Sword) dropSprite.setTextureRect(sf::IntRect(1, 153, 8, 16));
                else if (item.type == ItemType::MagicalSword) dropSprite.setTextureRect(sf::IntRect(36, 156, 8, 12));
                else if (item.type == ItemType::Candle) dropSprite.setTextureRect(sf::IntRect(319, 187, 8, 12));
                else if (item.type == ItemType::Bow) dropSprite.setTextureRect(sf::IntRect(309, 187, 8, 16));
                else if (item.type == ItemType::Bomb) dropSprite.setTextureRect(sf::IntRect(295, 270, 12, 12));
                else if (item.type == ItemType::IceRod) dropSprite.setTextureRect(sf::IntRect(145, 156, 8, 16));
                // else if (item.type == ItemType::Triforce) dropSprite.setTextureRect(sf::IntRect(295, 244, 12, 12));
                else if (item.type == ItemType::Boomerang) dropSprite.setTextureRect(sf::IntRect(64, 187, 8, 12));
                else if (item.type == ItemType::Key) dropSprite.setTextureRect(sf::IntRect(295, 256, 12, 12));
                
                // 3. SE DIBUJA EN PANTALLA (Esto también faltaba)
                window.draw(dropSprite);
            }
        }

        // --- DIBUJAR FLECHAS ACTIVAS (CON ROTACIÓN) ---
        for (const auto& arrow : activeArrows) {
            sf::Sprite arrowSprite(hudTexture);
            arrowSprite.setTextureRect(sf::IntRect(1, 187, 8, 15));
            arrowSprite.setScale(2.f, 2.f);
            arrowSprite.setOrigin(4.f, 7.5f); // Centro para rotar bien
            arrowSprite.setPosition(arrow.pos);

            if (arrow.direction == 0) arrowSprite.setRotation(0.f);       // Arriba
            else if (arrow.direction == 1) arrowSprite.setRotation(180.f);  // Abajo
            else if (arrow.direction == 2) arrowSprite.setRotation(270.f);  // Izquierda
            else if (arrow.direction == 3) arrowSprite.setRotation(90.f);   // Derecha

            window.draw(arrowSprite);
        }

        // --- DIBUJAR BOMBAS ACTIVAS ---
        for (const auto& bomb : activeBombs) {
            sf::Sprite bombSprite(hudTexture);
            if (!bomb.exploded) {
                bombSprite.setTextureRect(sf::IntRect(295, 270, 12, 11)); // Sprite de bomba normal
            } else {
                bombSprite.setTextureRect(sf::IntRect(191, 187, 12, 11)); // Sprite de explosión/fuego temporal
            }
            bombSprite.setScale(2.5f, 2.5f);
            bombSprite.setPosition(bomb.pos);
            window.draw(bombSprite);
        }

        // --- DIBUJAR BUMERÁN ACTIVO (GIRANDO) ---
        for (const auto& boom : activeBoomerangs) {
            sf::Sprite boomSprite(hudTexture);
            boomSprite.setTextureRect(sf::IntRect(64, 187, 8, 15));
            boomSprite.setScale(2.f, 2.f);
            boomSprite.setOrigin(4.f, 7.5f);
            boomSprite.setPosition(boom.pos);
            
            // Hace que gire constantemente sobre su eje (velocidad de 720 grados por segundo)
            static float boomRotation = 0.f;
            boomRotation += 720.f * dt;
            boomSprite.setRotation(boomRotation);

            window.draw(boomSprite);
        }

        player.render(window);

        // --- DIBUJAR EL FUEGO ---
        if (fireActive) {
            sf::Sprite fireSprite(hudTexture);
            fireSprite.setTextureRect(sf::IntRect(191, 187, 12, 11));
            fireSprite.setScale(4.f, 4.f); 
            fireSprite.setPosition(fireHitbox.getPosition());
            window.draw(fireSprite);
        }
        // --- DIBUJAR HIELO ACTIVO ---
        for (const auto& ice : activeIces) {
            sf::Sprite iceSprite(hudTexture);
            iceSprite.setTextureRect(sf::IntRect(138, 186, 12, 11)); // Coordenada corregida Y+1
            iceSprite.setScale(2.5f, 2.5f);
            iceSprite.setPosition(ice.pos);
            window.draw(iceSprite);
        }

        for (auto &e : enemies) e.render(window);
        drawHUD();

     // --- PANTALLA DE INVENTARIO DINÁMICO (CUADRÍCULA CON BARRIDO) ---
        if (scrollY > 0.f) {
            // --- APLICAR CÁMARA PARA EL BARRIDO DEL INVENTARIO ---
            sf::View invView = defaultView;
            invView.move(0.f, 600.f - scrollY);
            window.setView(invView);

            // Fondo totalmente negro estilo NES
            sf::RectangleShape invBg(sf::Vector2f(800.f, 600.f)); 
            invBg.setFillColor(sf::Color::Black); 
            window.draw(invBg);
            
            sf::Text invTitle("INVENTORY", font, 36); 
            invTitle.setFillColor(sf::Color::Red); 
            invTitle.setPosition(300.f, 50.f); 
            window.draw(invTitle);

            // 1. Escanear la mochila
            std::vector<Player::EquipItem> ownedItems;
            if (player.hasBoomerang) ownedItems.push_back(Player::EquipItem::BOOMERANG);
            if (player.hasBomb) ownedItems.push_back(Player::EquipItem::BOMB);
            if (player.hasBow) ownedItems.push_back(Player::EquipItem::BOW);
            if (player.hasCandle) ownedItems.push_back(Player::EquipItem::CANDLE);
            if (player.hasIceRod) ownedItems.push_back(Player::EquipItem::ICE_ROD);
            if (player.hasTriforce) ownedItems.push_back(Player::EquipItem::TRIFORCE);
            
            // 2. Dibujar cuadricula (4 columnas por fila)
            int cols = 4;
            for (size_t i = 0; i < ownedItems.size(); i++) {
                int col = i % cols;
                int row = i / cols;
                float x = 200.f + (col * 100.f);
                float y = 150.f + (row * 100.f);
                
                sf::RectangleShape itemBox(sf::Vector2f(60.f, 60.f));
                itemBox.setFillColor(sf::Color::Transparent);
                itemBox.setOutlineThickness(3.f);
                itemBox.setOutlineColor(sf::Color(100, 100, 255));
                itemBox.setPosition(x, y);
                window.draw(itemBox);

                sf::Sprite invSprite(hudTexture);
                invSprite.setScale(2.5f, 2.5f); // Un poco más grande para que luzca en la caja
                bool drawSprite = true;

                // Centrar sprites dependiendo si son de 8x16 o de 12x12 (con Y+1 aplicado)
                if (ownedItems[i] == Player::EquipItem::CANDLE) { invSprite.setTextureRect(sf::IntRect(319, 187, 8, 12)); invSprite.setPosition(x + 20.f, y + 10.f); }
                else if (ownedItems[i] == Player::EquipItem::BOW) { invSprite.setTextureRect(sf::IntRect(309, 187, 8, 12)); invSprite.setPosition(x + 20.f, y + 10.f); }
                else if (ownedItems[i] == Player::EquipItem::BOMB) { invSprite.setTextureRect(sf::IntRect(295, 270, 12, 12)); invSprite.setPosition(x + 15.f, y + 15.f); }
                else if (ownedItems[i] == Player::EquipItem::ICE_ROD) { invSprite.setTextureRect(sf::IntRect(145, 156, 8, 12)); invSprite.setPosition(x + 20.f, y + 10.f); }
                else if (ownedItems[i] == Player::EquipItem::TRIFORCE) { invSprite.setTextureRect(sf::IntRect(295, 244, 12, 11)); invSprite.setPosition(x + 15.f, y + 15.f); }
                else if (ownedItems[i] == Player::EquipItem::BOOMERANG) { invSprite.setTextureRect(sf::IntRect(64, 187, 8, 12)); invSprite.setPosition(x + 20.f, y + 10.f); }
                else { drawSprite = false; }

                if (drawSprite) window.draw(invSprite);
            }

            // 3. Lógica del cursor 2D (Solo responde cuando la pantalla terminó de deslizarse)
            if (scrollY == 600.f) inputDelay -= dt;
            if (!ownedItems.empty() && scrollY == 600.f) {
                if (cursorX >= ownedItems.size()) cursorX = ownedItems.size() - 1; 
                
                if (inputDelay <= 0.f) {
                    // Derecha
                    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D))) {
                        if (cursorX + 1 < ownedItems.size()) cursorX++; 
                        inputDelay = 0.2f;
                    }
                    // Izquierda
                    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A))) {
                        if (cursorX > 0) cursorX--; 
                        inputDelay = 0.2f;
                    }
                    // Abajo
                    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S))) {
                        if (cursorX + cols < ownedItems.size()) cursorX += cols; 
                        else cursorX = ownedItems.size() - 1; 
                        inputDelay = 0.2f;
                    }
                    // Arriba
                    if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))) {
                        if (cursorX - cols >= 0) cursorX -= cols; 
                        else cursorX = cursorX % cols; 
                        inputDelay = 0.2f;
                    }
                    
                    // Equipar
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) || sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
                        player.setItemB(ownedItems[cursorX]);
                        inputDelay = 0.3f;
                    }
                }

                // Dibujar cursor rojo dinámico
                int col = cursorX % cols;
                int row = cursorX / cols;
                sf::RectangleShape cursor(sf::Vector2f(68.f, 68.f));
                cursor.setFillColor(sf::Color::Transparent); 
                cursor.setOutlineThickness(4.f); 
                cursor.setOutlineColor(sf::Color::Red);
                cursor.setPosition(196.f + (col * 100.f), 146.f + (row * 100.f)); 
                window.draw(cursor);
            } else if (scrollY == 600.f) {
                sf::Text emptyText("NO ITEMS YET", font, 24); 
                emptyText.setPosition(300.f, 150.f); 
                window.draw(emptyText);
            }

            // 4. Mostrar estado actual
            sf::Text equipText("EQUIPPED B: NONE", font, 24);
            equipText.setPosition(250.f, 380.f);
            if (player.getItemB() == Player::EquipItem::CANDLE) equipText.setString("EQUIPPED B: CANDLE");
            else if (player.getItemB() == Player::EquipItem::BOW) equipText.setString("EQUIPPED B: BOW");
            else if (player.getItemB() == Player::EquipItem::BOMB) equipText.setString("EQUIPPED B: BOMB");
            else if (player.getItemB() == Player::EquipItem::BOOMERANG) equipText.setString("EQUIPPED B: BOOMERANG");
            else if (player.getItemB() == Player::EquipItem::ICE_ROD) equipText.setString("EQUIPPED B: ICE ROD");
            window.draw(equipText);
            
            sf::Text resumeText("Press P to Resume", font, 18); 
            resumeText.setPosition(300.f, 500.f); 
            window.draw(resumeText);
        }
        
        window.setView(defaultView); // Restaurar vista original antes de mostrar
        window.display();
    }
}

void Game::resetGame() {
    player = Player(levels[currentLevel].playerStart.x, levels[currentLevel].playerStart.y);
    loadLevel(currentLevel);
}