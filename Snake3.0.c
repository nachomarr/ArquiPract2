/*
    ***  Practica 2 ***
    *** Juego Snake ***

    - Ignacio Marquez
    - Renata Tejeda

*/


#include "ripes_system.h"
#include <stdbool.h>

// Definición de colores en formato RGB para los elementos del juego
#define COLOR_SNAKE 0xFF0000  // Rojo para la serpiente
#define COLOR_FOOD  0x00FF00  // Verde para la comida
#define COLOR_OFF   0x000000  // Negro para apagar LEDs

// Tamaño de cada bloque (pixeles) y longitud máxima de la serpiente
#define BLOCK_SIZE 2          
#define MAX_LENGTH 50         
// Contador de retardo para controlar la velocidad del juego
#define DELAY_COUNT 100      

// Mapeo de las direcciones posibles
typedef enum {UP, DOWN, LEFT, RIGHT} Direction;

// Punteros a los periféricos de hardware (matriz de LEDs, D-Pad y switches)
volatile unsigned int* led_matrix = (unsigned int*)LED_MATRIX_0_BASE;
volatile unsigned int* d_pad_up    = (unsigned int*)D_PAD_0_UP;
volatile unsigned int* d_pad_down  = (unsigned int*)D_PAD_0_DOWN;
volatile unsigned int* d_pad_left  = (unsigned int*)D_PAD_0_LEFT;
volatile unsigned int* d_pad_right = (unsigned int*)D_PAD_0_RIGHT;
volatile unsigned int* switches     = (unsigned int*)SWITCHES_0_BASE;

// Limpia toda la matriz de LEDs, poniendo todos los píxeles en COLOR_OFF.
// Se recorre cada posición de la pantalla y se apaga.

void clear_matrix() {
    for (int i = 0; i < LED_MATRIX_0_WIDTH * LED_MATRIX_0_HEIGHT; i++) {
        led_matrix[i] = COLOR_OFF;
    }
}

/*
 Func: Genera una posición aleatoria para la comida.
 seed:Variable que mantiene el estado del generador de números.
 food_x,food_y Punteros donde se almacenará la posición generada.
 
 Utiliza una fórmula lineal congruencial para generar nuevos valores de semilla,
 y calcula coordenadas alineadas al tamaño de bloque.
*/
void spawn_food(unsigned int *seed, int *food_x, int *food_y) {
    // Actualiza la semilla con fórmula LCG
    *seed = (*seed * 1103515245 + 12345) % 2147483648;
    // Calcula posición X válida y alineada al bloque
    int max_x = (LED_MATRIX_0_WIDTH - BLOCK_SIZE);
    *food_x = (*seed % (max_x / 2 + 1)) * 2;
    
    // Actualiza la semilla nuevamente
    *seed = (*seed * 1103515245 + 12345) % 2147483648;
    // Calcula posición Y válida y alineada al bloque
    int max_y = (LED_MATRIX_0_HEIGHT - BLOCK_SIZE);
    *food_y = (*seed % (max_y / 2 + 1)) * 2;
}

/*
 func: Verifica si dos bloques colisionan.
 hx,hy Coordenadas del primer bloque (cabeza de la serpiente).
 x,y   Coordenadas del segundo bloque (otro segmento o comida).
 return true si hay solapamiento, false en caso contrario.
*/
bool check_collision(int hx, int hy, int x, int y) {
    return (hx < x + BLOCK_SIZE && hx + BLOCK_SIZE > x &&
            hy < y + BLOCK_SIZE && hy + BLOCK_SIZE > y);
}

int main() {
    while (1) {
        // Vectores para almacenar las coordenadas de cada segmento
        int snake_x[MAX_LENGTH], snake_y[MAX_LENGTH];
        int snake_length = 1;       // Longitud inicial de la serpiente
        Direction dir = RIGHT;      // Dirección de movimiento inicial
        bool game_over = false;     // Bandera de fin de juego

        // Posición inicial: centro de la pantalla, alineada a bloques
        snake_x[0] = (LED_MATRIX_0_WIDTH / 2) - BLOCK_SIZE;
        snake_y[0] = (LED_MATRIX_0_HEIGHT / 2) - BLOCK_SIZE;

        int food_x = 0, food_y = 0;
        bool food_active = false;   // Indica si hay comida presente
        unsigned int seed = 123456789;  // Semilla inicial para RNG

        // Genera la primera comida y limpia la pantalla
        spawn_food(&seed, &food_x, &food_y);
        food_active = true;
        clear_matrix();

        // Bucle principal del juego (hasta game_over)
        while (!game_over) {
            // Lectura de controles y cambio de dirección
            if (*d_pad_up    && dir != DOWN)  dir = UP;
            else if (*d_pad_down  && dir != UP)    dir = DOWN;
            else if (*d_pad_left  && dir != RIGHT) dir = LEFT;
            else if (*d_pad_right && dir != LEFT)  dir = RIGHT;

            // Actualiza posición de cada segmento (sigue a la cabeza)
            for (int i = snake_length - 1; i > 0; i--) {
                snake_x[i] = snake_x[i-1];
                snake_y[i] = snake_y[i-1];
            }
            // Move la cabeza según la dirección actual, en pasos de 2 píxeles
            switch (dir) {
                case UP:    snake_y[0] -= BLOCK_SIZE; break;
                case DOWN:  snake_y[0] += BLOCK_SIZE; break;
                case LEFT:  snake_x[0] -= BLOCK_SIZE; break;
                case RIGHT: snake_x[0] += BLOCK_SIZE; break;
            }

            // Verifica colisión con bordes de la pantalla
            if (snake_x[0] < 0 || snake_x[0] > LED_MATRIX_0_WIDTH - BLOCK_SIZE ||
                snake_y[0] < 0 || snake_y[0] > LED_MATRIX_0_HEIGHT - BLOCK_SIZE) {
                game_over = true;
                break;
            }

            // Verifica colisión de la cabeza con el cuerpo
            for (int i = 1; i < snake_length; i++) {
                if (snake_x[0] == snake_x[i] && snake_y[0] == snake_y[i]) {
                    game_over = true;
                    break;
                }
            }
            if (game_over) break;

            // Comprueba si la cabeza colisiona con la comida
            if (food_active && check_collision(snake_x[0], snake_y[0], food_x, food_y)) {
                if (snake_length < MAX_LENGTH) snake_length++;  // Crece la serpiente
                spawn_food(&seed, &food_x, &food_y);             // Nueva comida
                food_active = true;
            }

            // Renderizado: primero limpia, luego dibuja comida y serpiente
            clear_matrix();
            if (food_active) {
                for (int dy = 0; dy < BLOCK_SIZE; dy++) {
                    for (int dx = 0; dx < BLOCK_SIZE; dx++) {
                        int pos = (food_y + dy) * LED_MATRIX_0_WIDTH + (food_x + dx);
                        led_matrix[pos] = COLOR_FOOD;
                    }
                }
            }
            for (int i = 0; i < snake_length; i++) {
                for (int dy = 0; dy < BLOCK_SIZE; dy++) {
                    for (int dx = 0; dx < BLOCK_SIZE; dx++) {
                        int pos = (snake_y[i] + dy) * LED_MATRIX_0_WIDTH + (snake_x[i] + dx);
                        led_matrix[pos] = COLOR_SNAKE;
                    }
                }
            }

            // Pequeña espera para controlar la velocidad del juego
            for (volatile int t = 0; t < DELAY_COUNT; t++);
        }

        // Espera a que el usuario presione el switch 0 para reiniciar
        while (1) {
            if (*switches & 0x1) {
                while (*switches & 0x1);  // Evita rebotes
                break;
            }
        }
    }
    return 0;
}
