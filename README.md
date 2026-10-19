# La petite place – Homework IOGL

Ma scène pour le devoir d'OpenGL : une petite place pavée qui passe du jour à la nuit, avec un robot-statue, des lanternes, des animaux qui se promènent et une partie de bowling. Je suis parti du code de la dernière leçon (C++, OpenGL 3.3, GLFW, GLEW, GLM).


## Compiler et lancer

J'ai travaillé sous WSL (Ubuntu dans Windows) :

```bash
sudo apt install cmake g++ libglfw3-dev libglew-dev libglm-dev
cmake -B build
cmake --build build -j4
WAYLAND_DISPLAY= ./build/VirtualScene
```

Sous WSL il faut lancer le programme avec `WAYLAND_DISPLAY=` devant, sinon GLFW passe par Wayland et GLEW n'arrive pas à s'initialiser. Sur un Linux classique ou sous Windows (Visual Studio), `./build/VirtualScene` suffit ; si GLFW/GLEW/GLM ne sont pas installés, CMake les télécharge tout seul.

`--demo` lance la visite automatique.

## Contrôles

| Touche | Action |
|---|---|
| ZQSD + souris | se déplacer / regarder |
| Espace / Shift | monter / descendre |
| C | caméra libre → orbite → visite |
| F | lampe torche |
| ← → | changer l'heure |
| 1 2 3 4 5 | ombres, bloom, brouillard, lucioles, shadow map |
| F1 | fil de fer |
| P | pause des animations |
| Échap | quitter |

## Ce que j'ai fait

- **Modèles** : les modèles du cours + une lanterne et un renard trouvés sur Internet (glTF convertis en OBJ). J'ai amélioré le chargeur OBJ.
- **Lumières** : soleil/lune, 8 lumières ponctuelles (lanternes, lampadaires, feux follets) et une lampe torche, en Blinn-Phong.
- **Matériaux** : textures diffuses, specular maps et textures émissives.
- **Caméras** : FPS, orbite et une visite cinématique (splines de Catmull-Rom).
- **Animations** : robot, lapin, renard, bowling, cycle jour/nuit.
- **Effets** : ombres (shadow mapping), HDR + bloom, ciel procédural, brouillard, lucioles (particules).

