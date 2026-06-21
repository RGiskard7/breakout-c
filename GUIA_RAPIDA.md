# Guia Rapida - Breakout

Arranca el juego en menos de 5 minutos.

---

## El camino mas corto

### Windows

```cmd
scripts\install-deps.bat
scripts\build.bat run
```

### macOS / Linux

```bash
scripts/install-deps.sh
scripts/build.sh run
```

---

## Controles

| Tecla        | Accion              |
|--------------|---------------------|
| Left / Right | Mover la pala       |
| Space        | Sacar la bola       |
| Enter        | Iniciar / Continuar |
| Escape       | Salir               |

---

## Reglas del original (Atari 2600)

- Tres bolas por partida
- Dos pantallas completas de ladrillos; si limpias las dos, ganas
- La bola acelera tras 4 golpes, tras 12, y al tocar las filas naranja y roja
- La pala se reduce a la mitad al romper hasta el muro superior
- Puntos: rojo/naranja 7, amarillas 4, verde/azul 1 (maximo 864)

---

## Si algo falla

**"undefined reference" al compilar**  
Faltan las librerias de audio. Revisa que `allegro_audio-5` y `allegro_acodec-5` esten instaladas.

**No se ve nada o no carga la fuente/sonidos**  
Ejecuta siempre desde la raiz del proyecto (`breakout-c/`). Las fuentes y el sonido están en la carpeta `resources/`.

**Falta `allegro_monolith-5.2.dll` al ejecutar**  
`scripts\build.bat run` la copia sola. Para repartir el juego usa `scripts\dist.bat`.

---

## Verificar que todo esta bien

```bash
gcc --version
pkg-config --modversion allegro-5
```
