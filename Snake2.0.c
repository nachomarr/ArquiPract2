#include "ripes_system.h"
#include <stdbool.h>

#define COLOR_SNAKE 0xFF0000
#define COLOR_FOOD  0x00FF00
#define COLOR_OFF   0x000000
#define BLOCK_SIZE 2
#define MAX_LENGTH 50
#define DELAY_COUNT 100

volatile unsigned int* led_matrix = (unsigned int*)LED_MATRIX_0_BASE;
volatile unsigned int* d_pad_up    = (unsigned int*)D_PAD_0_UP;
volatile unsigned int* d_pad_down  = (unsigned int*)D_PAD_0_DOWN;
volatile unsigned int* d_pad_left  = (unsigned int*)D_PAD_0_LEFT;
volatile unsigned int* d_pad_right = (unsigned int*)D_PAD_0_RIGHT;
volatile unsigned int* switches = (unsigned int*)SWITCHES_0_BASE;

typedef enum {UP, DOWN, LEFT, RIGHT} Direction;

void clear_matrix() {
    for (int i = 0; i < LED_MATRIX_0_WIDTH * LED_MATRIX_0_HEIGHT; i++) {
        led_matrix[i] = COLOR_OFF;
    }
}

void spawn_food(unsigned int *seed, int *food_x, int *food_y) {
    *seed = (*seed * 1103515245 + 12345) % 2147483648;
    int max_x = (LED_MATRIX_0_WIDTH - BLOCK_SIZE);
    *food_x = (*seed % (max_x / 2 + 1)) * 2;
    
    *seed = (*seed * 1103515245 + 12345) % 2147483648;
    int max_y = (LED_MATRIX_0_HEIGHT - BLOCK_SIZE);
    *food_y = (*seed % (max_y / 2 + 1)) * 2;
}

bool check_collision(int hx, int hy, int x, int y) {
    return (hx < x + BLOCK_SIZE && hx + BLOCK_SIZE > x &&
            hy < y + BLOCK_SIZE && hy + BLOCK_SIZE > y);
}

int main() {
    while (1) {
        int snake_x[MAX_LENGTH], snake_y[MAX_LENGTH];
        int snake_length = 1;
        Direction dir = RIGHT;
        bool game_over = false;

        // Posición inicial centrada y alineada
        snake_x[0] = (LED_MATRIX_0_WIDTH / 2) - BLOCK_SIZE;
        snake_y[0] = (LED_MATRIX_0_HEIGHT / 2) - BLOCK_SIZE;

        int food_x = 0, food_y = 0;
        bool food_active = false;
        unsigned int seed = 123456789;

        spawn_food(&seed, &food_x, &food_y);
        food_active = true;
        clear_matrix();

        while (!game_over) {
            // Lectura de controles
            if (*d_pad_up    && dir != DOWN)  dir = UP;
            else if (*d_pad_down  && dir != UP)    dir = DOWN;
            else if (*d_pad_left  && dir != RIGHT) dir = LEFT;
            else if (*d_pad_right && dir != LEFT)  dir = RIGHT;

            // Mover cuerpo
            for (int i = snake_length - 1; i > 0; i--) {
                snake_x[i] = snake_x[i-1];
                snake_y[i] = snake_y[i-1];
            }
            
            // Movimiento de 2 pixeles
            switch (dir) {
                case UP:    snake_y[0] -= 2; break;
                case DOWN:  snake_y[0] += 2; break;
                case LEFT:  snake_x[0] -= 2; break;
                case RIGHT: snake_x[0] += 2; break;
            }

            // Colisiones 
            if (snake_x[0] < 0 || snake_x[0] > LED_MATRIX_0_WIDTH - BLOCK_SIZE ||
                snake_y[0] < 0 || snake_y[0] > LED_MATRIX_0_HEIGHT - BLOCK_SIZE) {
                game_over = true;
                break;
            }

            for (int i = 1; i < snake_length; i++) {
                if (snake_x[0] == snake_x[i] && snake_y[0] == snake_y[i]) {
                    game_over = true;
                    break;
                }
            }
            if (game_over) break;

            // Comprobación de comida 
            if (food_active && check_collision(snake_x[0], snake_y[0], food_x, food_y)) {
                if (snake_length < MAX_LENGTH) snake_length++;
                spawn_food(&seed, &food_x, &food_y);
                food_active = true;
            }

            // Dibujado
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

            for (volatile int t = 0; t < DELAY_COUNT; t++);
        }

        // reinicio
        while (1) {
            if (*switches & 0x1) {
                while (*switches & 0x1);
                break;
            }
        }
    }
    return 0;
}
