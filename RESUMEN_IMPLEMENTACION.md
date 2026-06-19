# Resumen de Implementacion - Breakout

## Codigo

| Archivo | Lineas | Que hace |
|---------|--------|----------|
| `src/main.c` | 224 | Entrada, inicializa Allegro, bucle principal |
| `src/game.c` | 930 | Logica central, estados, colisiones, render, marcador |
| `src/paddle.c` | 192 | Pala del jugador: movimiento y reduccion a la mitad |
| `src/ball.c` | 434 | Bola: fisica, rebotes y progresion de velocidad |
| `src/brick.c` | 249 | Ladrillos: rejilla, colores por fila y puntos |
| `include/config.h` | 111 | Constantes, geometria, colores y reglas |

## Arquitectura

- Structs opacos con getters/setters (igual que Space Invaders)
- Configuracion centralizada en `config.h`
- Maquina de 5 estados: titulo, jugando, bola perdida, game over, victoria
- Recursos creados y destruidos explicitamente, en orden correcto

## Fidelidad al Atari 2600

- Geometria medida pixel a pixel sobre `resources/Breakout2600.png` (escala 0.5)
- Marco gris: barra superior de ancho completo + dos paredes laterales
- Marcador con digitos de bloque estirados: puntuacion, bola y jugador
- 6 filas x 18 ladrillos, dibujadas como bandas solidas (los huecos aparecen al romper)
- Colores exactos por fila (rojo, naranja, ocre, oliva, verde, azul)
- Puntos 7/7/4/4/1/1 -> 432 por pantalla, 864 el maximo (dos pantallas)
- La bola acelera tras 4 golpes, tras 12, y al tocar las filas naranja y roja
- La pala se reduce a la mitad al romper hasta el muro superior
- Tres bolas y dos pantallas por partida

## Sistema de compilacion

- `Makefile` (Windows/MinGW)
- `scripts/build.bat` / `scripts/build.sh`
- `scripts/install-deps.bat` / `scripts/install-deps.sh`
- `scripts/dist.bat` / `scripts/dist.sh` (carpeta portable: Windows lleva .exe + DLL + recursos; macOS/Linux solo binario + recursos)

## Sobre la DLL de Allegro

El ejecutable solo necesita `allegro_monolith-5.2.dll`; el resto de dependencias
(`KERNEL32`, `api-ms-win-crt-*`) ya vienen con Windows 10/11. No se puede generar
un .exe sin esa DLL porque esta distribucion de Allegro no incluye los `.a`
estaticos de sus dependencias (FLAC, Vorbis, FreeType, libpng...): solo existen
dentro del propio monolito. Por eso `scripts/dist.bat` la empaqueta junto al juego.

## Constantes principales (config.h)

| Constante | Valor | Descripcion |
|-----------|-------|-------------|
| `DISPLAY_WIDTH/HEIGHT` | 640 x 420 | Tamano de ventana (proporcion del 2600) |
| `BRICK_COLS / BRICK_ROWS` | 18 x 6 | Rejilla de ladrillos |
| `MAX_LIVES` | 3 | Bolas por partida |
| `MAX_SCREENS` | 2 | Pantallas de ladrillos por partida |
| `SPEEDUP_HITS_1 / _2` | 4 / 12 | Golpes que aceleran la bola |
| `PADDLE_WIDTH / _SMALL` | 64 / 32 | Pala normal y reducida |
| `BALL_SPD_0 .. _4` | 3.0 .. 7.0 | Niveles de velocidad de la bola |
