#!/bin/bash

# Empaqueta Breakout en una carpeta portable (dist) para macOS/Linux.
# A diferencia de Windows, aqui Allegro es una libreria del sistema, asi que
# no hay ningun .dll/.so que copiar: la carpeta lleva el binario y los recursos,
# y la maquina destino solo necesita tener Allegro 5 instalado.
# Autor: RGiskard7

set -e

cd "$(dirname "$0")/.." || exit 1

DIST="dist"

echo "═══════════════════════════════════════════════════"
echo "        Empaquetando Breakout (dist)"
echo "═══════════════════════════════════════════════════"

# 1. Compilar usando el script de build
bash scripts/build.sh

# 2. Preparar carpeta limpia
rm -rf "$DIST"
mkdir -p "$DIST/resources/fonts" "$DIST/resources/sounds"

# 3. Copiar binario y solo los recursos que usa Breakout
cp breakout "$DIST/"
cp resources/fonts/space_invaders.ttf "$DIST/resources/fonts/"
cp resources/sounds/shoot.wav "$DIST/resources/sounds/"
cp resources/sounds/invaderkilled.wav "$DIST/resources/sounds/"

echo ""
echo "✅ Carpeta portable lista en: $DIST"
echo "La maquina destino necesita Allegro 5 instalado (brew/apt)."
