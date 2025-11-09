# Timber!

Un petit videojoc d’acció desenvolupat amb **C++** i **SFML**, inspirat en el clàssic “Timberman”.  
L’objectiu és tallar el tronc el màxim de vegades possible abans que s’acabi el temps o una branca caigui sobre el jugador!

---

## Autors

**Jordi Rubau Font**

---

## Descripció del joc

El jugador controla un llenyataire que pot tallar a l’esquerra o a la dreta del tronc.  
Cada cop correcte incrementa la puntuació i afegeix una mica de temps addicional al temporitzador.

Però cal anar amb compte! Les branques van baixant cada cop que es talla, i si una d’elles cau sobre el jugador, **el joc s’acaba**.  
També s’acaba si s’esgota el temps.

---

## Controls

| Fletxa esquerra | Tallar a l’esquerra |
| Fletxa dreta | Tallar a la dreta |
| Enter | Iniciar / Reiniciar partida |
| Escape | Sortir del joc |

---

## Característiques tècniques

- Motor gràfic: **SFML**
- Llenguatge: **C++**
- Mode: **Pantalla completa**
- FPS i temporitzador visibles per depuració

---

## Mecàniques principals

- **Tall**: cada cop augmenta la puntuació i el temps disponible.  
- **Branques**: es mouen una posició cap avall en cada tall, amb aparició aleatòria.  
- **Tronc volant**: després de cada tall, un tronc surt disparat amb efecte visual.  
- **Barra de temps**: indica visualment el temps restant.  
---
