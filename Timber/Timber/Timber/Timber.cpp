//============================================================
// LLIBRERIES I ESPAIS DE NOMS
//============================================================
// S'inclouen les llibreries de SFML per poder utilitzar gràfics, àudio, i gestionar finestres.
// També s'importen llibreries estàndard de C++ per gestionar text, temps i consola.
// "using namespace sf" s'utilitza per evitar haver d'escriure "sf::" davant de cada objecte.
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <sstream>
#include <ctime>
#include <iostream>
using namespace sf;

//============================================================
// CONSTANTS GLOBALS I ENUMERACIONS
//============================================================
// S'utilitzen constants per evitar valors "màgics" dins del codi i fer-lo més clar i modificable.
// L'enumeració "Side" permet indicar clarament el costat (esquerra, dreta o cap) en lloc d'usar nombres.
const int NUM_CLOUDS = 3;
const float timeBarStartWidth = 400;
const float timeBarHeight = 80;
const int NUM_BRANCHES = 6;
const float AXE_POSITION_LEFT = 700;
const float AXE_POSITION_RIGHT = 1075;

enum class Side { LEFT, RIGHT, NONE };

//============================================================
// ESTRUCTURA NPC (ABELLA / NÚVOLS)
//============================================================
// Aquesta estructura permet reutilitzar el mateix codi per objectes que es mouen sols,
// com ara núvols o l’abella. Això evita duplicar codi i millora l’organització.
// Cada NPC té un sprite, una velocitat, una direcció (sentit) i un estat actiu o no.
struct NPC {
    Sprite sprite;
    bool active;
    float speed;
    int maxHeight;
    int maxSpeed;
    int sentit;
    float posicioInicialX;

    // Constructor: s’inicialitzen tots els valors en crear un NPC.
    NPC(Texture& texture, int maxHeight_, int maxSpeed_, int sentit_, float posicioInicialX_)
        : sprite(texture), active(false), speed(0),
        maxHeight(maxHeight_), maxSpeed(maxSpeed_),
        sentit(sentit_), posicioInicialX(posicioInicialX_) {
    }
};

//============================================================
// PROTOTIPS DE FUNCIONS
//============================================================
// Es declaren aquí perquè puguin ser cridades abans de la seva definició real al final del fitxer.
// Això millora la llegibilitat i permet mantenir la funció main més amunt.
void updateNPC(NPC&, float);
void updateBranchSprites(Side[], Sprite[]);
void updateBranches(Side[], int);

//============================================================
// FUNCIÓ PRINCIPAL
//============================================================
int main()
{
    //--------------------------------------------------------
    // CONFIGURACIÓ BRANQUES
    //--------------------------------------------------------
    // Es carrega una sola textura per a totes les branques (eficiència de memòria).
    // Es crea un array amb 6 sprites, ja que les branques es mouen visualment però comparteixen la mateixa textura.
    Texture textureBranch("graphics/branch.png");
    Side branchPositions[NUM_BRANCHES];
    Sprite branches[NUM_BRANCHES] = {
        Sprite(textureBranch), Sprite(textureBranch), Sprite(textureBranch),
        Sprite(textureBranch), Sprite(textureBranch), Sprite(textureBranch)
    };
    // Posició inicial fora de pantalla per evitar que es vegin al principi.
    for (int i = 0; i < NUM_BRANCHES; i++) {
        branches[i].setPosition({ -2000, -2000 });
        branches[i].setOrigin({ 220, 20 }); // Punt central de rotació, ajustat al centre del tronc.
        branchPositions[i] = Side::LEFT;
    }

    //--------------------------------------------------------
    // TEXTOS I MARCADOR
    //--------------------------------------------------------
    // Es carrega la font i es creen textos per mostrar missatges, puntuació i dades de depuració (FPS).
    Font font("fonts/KOMIKAP_.ttf");
    Text messageText(font);
    messageText.setString("Press Enter to start!");
    messageText.setCharacterSize(75);
    messageText.setFillColor(Color::White);

    // Centrat del text a la pantalla (justificació: el text ha d’estar centrat per millor llegibilitat).
    FloatRect textRect = messageText.getLocalBounds();
    messageText.setOrigin({ textRect.position.x + textRect.size.x / 2.0f,
                            textRect.position.y + textRect.size.y / 2.0f });
    messageText.setPosition({ 1920 / 2, 1080 / 2 });

    // Text per al marcador (puntuació).
    Text scoreText(font);
    scoreText.setCharacterSize(100);
    scoreText.setString("Score = 0");
    scoreText.setFillColor(Color::White);
    scoreText.setPosition({ 20, 20 });

    // Text per mostrar FPS i temps restant (ajuda per depuració i optimització).
    Text debugText(font);
    debugText.setCharacterSize(30);
    debugText.setFillColor(Color::Cyan);
    debugText.setPosition({ 1600, 20 });

    Side playerSide = Side::LEFT; // Posició inicial del jugador.

    //--------------------------------------------------------
    // CREACIÓ I ELEMENTS VISUALS
    //--------------------------------------------------------
    // Variables que controlen l’estat del joc (pausa, temps, puntuació...).
    bool paused = true;
    Clock clock;        // Control del temps entre fotogrames.
    Clock fpsClock;     // Control específic per calcular els FPS.
    int score = 0;
    float timeRemaining = 6.0f;
    float fps = 0.0f;

    // Barra de temps: indica el temps restant visualment.
    RectangleShape timeBar(Vector2f(timeBarStartWidth, timeBarHeight));
    timeBar.setFillColor(Color::Red);
    timeBar.setPosition({ 1920 / 2 - timeBarStartWidth / 2, 980 });
    float timeBarWidthPerSecond = timeBarStartWidth / timeRemaining;

    // Es crea la finestra del joc en mode pantalla completa per immersió.
    VideoMode vm({ 1920, 1080 });
    RenderWindow window(vm, "Timber!!!", State::Fullscreen);

    // Fons i decoració: textures separades per facilitar el “depth layering” (capas de profunditat).
    Texture textureBackground("graphics/background.png");
    Sprite spriteBackground(textureBackground);

    Texture textureTree1("graphics/tree.png");
    Sprite spriteTree1(textureTree1);
    spriteTree1.setPosition({ 810, 0 });

    Texture textureTree2("graphics/tree2.png");
    Sprite spriteTreeBack1(textureTree2);
    Sprite spriteTreeBack2(textureTree2);
    spriteTreeBack1.setPosition({ 100, 0 });
    spriteTreeBack2.setPosition({ 1500, 0 });
    Sprite spriteTreeFront1(textureTree2);
    Sprite spriteTreeFront2(textureTree2);
    spriteTreeFront1.setPosition({ 50, 0 });
    spriteTreeFront2.setPosition({ 1600, 0 });

    // Jugador i altres objectes del joc.
    Texture texturePlayer("graphics/player.png");
    Sprite spritePlayer(texturePlayer);
    spritePlayer.setPosition({ 580, 720 });

    // Creació dels NPC (abella i núvols). Es reutilitza la mateixa estructura NPC per simplificar.
    Texture textureBee("graphics/bee.png");
    Texture textureCloud("graphics/cloud.png");
    NPC bee(textureBee, 500, 400, -1, 2000);
    bee.sprite.setScale({ -5.f, 5.f }); // Invertim l’abella horitzontalment perquè miri cap a l’esquerra.
    NPC clouds[NUM_CLOUDS] = {
        NPC(textureCloud, 100, 200, 1, -200),
        NPC(textureCloud, 250, 200, 1, -200),
        NPC(textureCloud, 500, 200, 1, -200)
    };

    // Objectes addicionals: làpida, destral i tronc volant.
    Texture textureLapida("graphics/rip.png");
    Sprite spriteLapida(textureLapida);

    Texture textureAxe("graphics/axe.png");
    Sprite spriteAxe(textureAxe);
    spriteAxe.setPosition({ 580, 720 });

    Texture textureLog("graphics/log.png");
    Sprite spriteLog(textureLog);
    spriteLog.setPosition({ 810, 720 });
    bool logActive = false;
    float logSpeedX = 0;
    float logSpeedY = -1500; // Tronc surt volant cap amunt per animació.

    //--------------------------------------------------------
    // AUDIOS
    //--------------------------------------------------------
    // Es carreguen i associen els efectes de so per a cada acció (cop, mort, temps esgotat).
    SoundBuffer chopAudioBuffer("sound/chop.wav");
    Sound chopSound(chopAudioBuffer);
    SoundBuffer deathAudioBuffer("sound/death.wav");
    Sound deathSound(deathAudioBuffer);
    SoundBuffer ootAudioBuffer("sound/out_of_time.wav");
    Sound ootSound(ootAudioBuffer);

    //--------------------------------------------------------
    // BUCLE PRINCIPAL DEL JOC
    //--------------------------------------------------------
    // Aquest bucle controla tot el joc. Mentre la finestra estigui oberta:
    // - Processa esdeveniments (tecles, tancament...)
    // - Actualitza la lògica del joc
    // - Redibuixa tots els elements
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            // Tancament de la finestra.
            if (event->is<Event::Closed>())
                window.close();

            // Control de tecles.
            if (const auto* key = event->getIf<Event::KeyPressed>()) {
                if (key->scancode == Keyboard::Scancode::Escape)
                    window.close();

                // Quan es prem ENTER: es reinicia el joc.
                if (key->scancode == Keyboard::Scancode::Enter) {
                    paused = false;
                    score = 0;
                    timeRemaining = 6;
                    for (int i = 0; i < NUM_BRANCHES; i++)
                        branchPositions[i] = Side::NONE;
                    spriteLapida.setPosition({ 675, 2000 });
                    spritePlayer.setPosition({ 675, 660 });
                }

                // Tall a l’esquerra
                if (key->scancode == Keyboard::Scancode::Left) {
                    playerSide = Side::LEFT;
                    score++;
                    timeRemaining += (2 / score) + .15f; // Petita recompensa de temps.
                    spriteAxe.setPosition({ AXE_POSITION_LEFT, spriteAxe.getPosition().y });
                    spritePlayer.setPosition({ 580, 720 });
                    updateBranches(branchPositions, score);
                    spriteLog.setPosition({ 810, 720 });
                    logSpeedX = 5000; // El tronc surt disparat cap a la dreta.
                    logActive = true;
                    chopSound.play();
                }

                // Tall a la dreta
                if (key->scancode == Keyboard::Scancode::Right) {
                    playerSide = Side::RIGHT;
                    score++;
                    timeRemaining += (2 / score) + .15f;
                    spriteAxe.setPosition({ AXE_POSITION_RIGHT, spriteAxe.getPosition().y });
                    spritePlayer.setPosition({ 1200, 720 });
                    updateBranches(branchPositions, score);
                    spriteLog.setPosition({ 810, 720 });
                    logSpeedX = -5000;
                    logActive = true;
                    chopSound.play();
                }
            }
        }

        //--------------------------------------------------------
        // ACTUALITZACIÓ DE LÒGICA DEL JOC
        //--------------------------------------------------------
        // Si el joc no està pausat, es fan els càlculs d’animació, temps i col·lisions.
        if (!paused) {
            Time dt = clock.restart();
            fps = 1.0f / fpsClock.restart().asSeconds();

            timeRemaining -= dt.asSeconds();
            timeBar.setSize(Vector2f(timeBarWidthPerSecond * timeRemaining, 80));

            // Si s’esgota el temps, es pausa el joc i sona el so corresponent.
            if (timeRemaining <= 0) {
                paused = true;
                messageText.setString("Out of time!");
                textRect = messageText.getLocalBounds();
                messageText.setOrigin({
                    textRect.position.x + textRect.size.x / 2.0f,
                    textRect.position.y + textRect.size.y / 2.0f
                    });
                ootSound.play();
            }

            // Actualització dels NPC (núvols i abella).
            updateNPC(clouds[0], dt.asSeconds());
            updateNPC(clouds[1], dt.asSeconds());
            updateNPC(clouds[2], dt.asSeconds());
            updateNPC(bee, dt.asSeconds());
            updateBranchSprites(branchPositions, branches);

            // Si el jugador queda sota una branca → mort.
            if (branchPositions[5] == playerSide) {
                paused = true;
                spriteLapida.setPosition({ 525, 760 });
                spritePlayer.setPosition({ 2000, 660 });
                messageText.setString("SQUISHED!");
                textRect = messageText.getLocalBounds();
                messageText.setOrigin({
                    textRect.position.x + textRect.size.x / 2.0f,
                    textRect.position.y + textRect.size.y / 2.0f
                    });
                deathSound.play();
            }

            // Moviment del tronc després de tallar.
            if (logActive) {
                spriteLog.setPosition({
                    spriteLog.getPosition().x + logSpeedX * dt.asSeconds(),
                    spriteLog.getPosition().y + logSpeedY * dt.asSeconds()
                    });
                // Quan surt de pantalla es reinicia la seva posició.
                if (spriteLog.getPosition().x < -100 || spriteLog.getPosition().x > 2000) {
                    logActive = false;
                    spriteLog.setPosition({ 810, 720 });
                }
            }

            // Actualització del marcador i text de depuració.
            std::stringstream ss;
            ss << "Score = " << score;
            scoreText.setString(ss.str());

            std::stringstream debug;
            debug << "FPS: " << static_cast<int>(fps) << "\n"
                << "Time: " << static_cast<int>(timeRemaining);
            debugText.setString(debug.str());
        }

        //--------------------------------------------------------
        // DIBUIX DELS ELEMENTS (DEPTH LAYERING)
        //--------------------------------------------------------
        // Es dibuixen els objectes en l’ordre correcte per crear profunditat visual.
        window.clear();

        window.draw(spriteBackground);
        window.draw(spriteTreeBack1);
        window.draw(spriteTreeBack2);
        for (int i = 0; i < NUM_CLOUDS; i++) window.draw(clouds[i].sprite);
        window.draw(bee.sprite);
        for (int i = 0; i < NUM_BRANCHES; i++) window.draw(branches[i]);
        window.draw(spriteTree1);
        window.draw(spriteLog);
        window.draw(spriteAxe);
        window.draw(spritePlayer);
        window.draw(spriteLapida);
        window.draw(spriteTreeFront1);
        window.draw(spriteTreeFront2);

        // Dibuix dels elements de la interfície.
        window.draw(scoreText);
        window.draw(timeBar);
        window.draw(debugText);
        if (paused) window.draw(messageText);

        window.display();
    }
    return 0;
}

//============================================================
// FUNCIONS AUXILIARS
//============================================================
// Aquestes funcions mantenen el codi principal més net i modular,
// separant la lògica dels moviments i actualitzacions.

// Actualitza la posició dels NPC (abella i núvols).
void updateNPC(NPC& npc, float dt) {
    if (!npc.active) {
        npc.speed = (rand() % npc.maxSpeed) * npc.sentit;
        float height = rand() % npc.maxHeight;
        npc.sprite.setPosition({ npc.posicioInicialX, height });
        npc.active = true;
    }
    else {
        npc.sprite.setPosition({
            npc.sprite.getPosition().x + npc.speed * dt,
            npc.sprite.getPosition().y
            });
        // Quan surten de pantalla, es desactiven per reutilitzar-los després.
        if (npc.sprite.getPosition().x < -200 || npc.sprite.getPosition().x > 2000)
            npc.active = false;
    }
}

// Actualitza la posició de les branques segons el costat on han de sortir.
void updateBranchSprites(Side branchPositions[], Sprite branches[]) {
    for (int i = 0; i < NUM_BRANCHES; i++) {
        float height = i * 150;
        if (branchPositions[i] == Side::LEFT) {
            branches[i].setPosition({ 610, height });
            branches[i].setRotation(degrees(180));
        }
        else if (branchPositions[i] == Side::RIGHT) {
            branches[i].setPosition({ 1330, height });
            branches[i].setRotation(degrees(0));
        }
        else {
            branches[i].setPosition({ 3000, height });
        }
    }
}

// Mou les branques una posició cap avall i genera una nova a dalt.
// Es fa servir una llavor (seed) per obtenir moviments aleatoris però controlats.
void updateBranches(Side branchPositions[], int seed) {
    for (int j = NUM_BRANCHES - 1; j > 0; j--) {
        branchPositions[j] = branchPositions[j - 1];
    }
    srand((int)time(0) + seed);
    int r = rand() % 5;
    switch (r) {
    case 0: branchPositions[0] = Side::LEFT; break;
    case 1: branchPositions[0] = Side::RIGHT; break;
    default: branchPositions[0] = Side::NONE; break;
    }
}
