#!/bin/bash

# Script de compilación para Breakout en macOS/Linux
# Autor: RGiskard7
# Uso: ./build.sh [run|clean]

set -e  # Salir si hay errores

# Cambiar al directorio raíz del proyecto (espacio compartido con Space Invaders)
cd "$(dirname "$0")/../.." || exit 1

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_info()    { echo -e "${BLUE}ℹ ${NC}$1"; }
print_success() { echo -e "${GREEN}✅ ${NC}$1"; }
print_warning() { echo -e "${YELLOW}⚠️  ${NC}$1"; }
print_error()   { echo -e "${RED}❌ ${NC}$1"; }

echo -e "${BLUE}"
echo "═══════════════════════════════════════════════════"
echo "        Breakout - Build Script"
echo "═══════════════════════════════════════════════════"
echo -e "${NC}"

# Verificar dependencias
print_info "Verificando dependencias..."

if ! command -v gcc &> /dev/null; then
    print_error "GCC no está instalado"
    echo "  macOS: xcode-select --install"
    echo "  Linux: sudo apt install build-essential"
    exit 1
fi
print_success "GCC encontrado: $(gcc --version | head -n1)"

if ! command -v pkg-config &> /dev/null; then
    print_error "pkg-config no está instalado"
    echo "  macOS: brew install pkg-config"
    echo "  Linux: sudo apt install pkg-config"
    exit 1
fi
print_success "pkg-config encontrado"

if ! pkg-config --exists allegro-5; then
    print_error "Allegro 5 no está instalado"
    echo "  macOS: brew install allegro"
    echo "  Linux: sudo apt install liballegro5-dev"
    exit 1
fi
print_success "Allegro 5 encontrado: v$(pkg-config --modversion allegro-5)"

# Limpiar
if [ "$1" == "clean" ]; then
    print_info "Limpiando archivos anteriores..."
    rm -f breakout/src/*.o breakout/breakout
    print_success "Limpieza completada"
    exit 0
fi

# Compilar
print_info "Compilando Breakout..."

CFLAGS="-I breakout/include $(pkg-config --cflags allegro-5) -Wall -g"
LIBS="$(pkg-config --libs allegro-5 allegro_main-5 allegro_primitives-5 allegro_font-5 allegro_ttf-5 allegro_image-5 allegro_audio-5 allegro_acodec-5)"

gcc -c breakout/src/main.c   $CFLAGS -o breakout/src/main.o
gcc -c breakout/src/game.c   $CFLAGS -o breakout/src/game.o
gcc -c breakout/src/paddle.c $CFLAGS -o breakout/src/paddle.o
gcc -c breakout/src/ball.c   $CFLAGS -o breakout/src/ball.o
gcc -c breakout/src/brick.c  $CFLAGS -o breakout/src/brick.o

gcc -o breakout/breakout \
    breakout/src/main.o breakout/src/game.o breakout/src/paddle.o \
    breakout/src/ball.o breakout/src/brick.o $LIBS

print_success "Ejecutable creado: ./breakout/breakout"
chmod +x breakout/breakout

if [ "$1" == "run" ]; then
    print_info "Iniciando Breakout..."
    echo ""
    ./breakout/breakout
else
    echo ""
    print_info "Para ejecutar: ./breakout/scripts/build.sh run"
fi
