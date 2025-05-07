#include "ripes_system.h"
#include <stdbool.h>

#define COLOR_SNAKE 0xFF0000
#define COLOR_FOOD  0x00FF00
#define COLOR_OFF   0x000000
#define BLOCK_SIZE 2
#define MAX_LENGTH 100

volatile unsigned int* led_matrix = (unsigned int*)LED_MATRIX_0_BASE;
volatile unsigned int* d_pad_up = (unsigned int*)D_PAD_0_UP;
volatile unsigned int* d_pad_down = (unsigned int*)D_PAD_0_DOWN;
volatile unsigned int* d_pad_left = (unsigned int*)D_PAD_0_LEFT;
volatile unsigned int* d_pad_right = (unsigned int*)D_PAD_0_RIGHT;

typedef enum {STOP, UP, DOWN, LEFT, RIGHT} Direction;

int main() {
    int snake_x[MAX_LENGTH], snake_y[MAX_LENGTH];
    int snake_length = 1;
    snake_x[0] = (LED_MATRIX_0_WIDTH / 2) - 1;
    snake_y[0] = (LED_MATRIX_0_HEIGHT / 2) - 1;
    
    int food_x, food_y;
    bool food_active = false;
    
    // Variables de control
    Direction current_dir = RIGHT;
    unsigned int seed = 123456789; // Para generación aleatoria
    
    // Inicialización
    for (int i = 0; i < LED_MATRIX_0_WIDTH * LED_MATRIX_0_HEIGHT; i++) {
        led_matrix[i] = COLOR_OFF;
    }
    
    // comida
    seed = (seed * 1103515245 + 12345) % 2147483648;
    food_x = (seed % (LED_MATRIX_0_WIDTH - BLOCK_SIZE + 1));
    seed = (seed * 1103515245 + 12345) % 2147483648;
    food_y = (seed % (LED_MATRIX_0_HEIGHT - BLOCK_SIZE + 1));
    food_active = true;
    
    for (int dy = 0; dy < BLOCK_SIZE; dy++) {
        for (int dx = 0; dx < BLOCK_SIZE; dx++) {
            int pos = (food_y + dy) * LED_MATRIX_0_WIDTH + (food_x + dx);
            led_matrix[pos] = COLOR_FOOD;
        }
    }
    
    // Bucle principal
    while (1) {
        // dirección
        Direction new_dir = current_dir;
        if (*d_pad_up && current_dir != DOWN) new_dir = UP;
        else if (*d_pad_down && current_dir != UP) new_dir = DOWN;
        else if (*d_pad_left && current_dir != RIGHT) new_dir = LEFT;
        else if (*d_pad_right && current_dir != LEFT) new_dir = RIGHT;
        
        // Actualizar movimiento
        if (new_dir != STOP) {
            // Mover cuerpo 
            for (int i = snake_length-1; i > 0; i--) {
                snake_x[i] = snake_x[i-1];
                snake_y[i] = snake_y[i-1];
            }
            
            // Mover cabeza
            switch(new_dir) {
                case UP: 
                    snake_y[0] = (snake_y[0] > 0) ? snake_y[0]-1 : LED_MATRIX_0_HEIGHT-BLOCK_SIZE;
                    break;
                case DOWN:
                    snake_y[0] = (snake_y[0] < LED_MATRIX_0_HEIGHT-BLOCK_SIZE) ? snake_y[0]+1 : 0;
                    break;
                case LEFT:
                    snake_x[0] = (snake_x[0] > 0) ? snake_x[0]-1 : LED_MATRIX_0_WIDTH-BLOCK_SIZE;
                    break;
                case RIGHT:
                    snake_x[0] = (snake_x[0] < LED_MATRIX_0_WIDTH-BLOCK_SIZE) ? snake_x[0]+1 : 0;
                    break;
                default: break;
            }
            current_dir = new_dir;
            
            // Verificar comida
            if (food_active &&
                snake_x[0] < food_x + BLOCK_SIZE &&
                snake_x[0] + BLOCK_SIZE > food_x &&
                snake_y[0] < food_y + BLOCK_SIZE &&
                snake_y[0] + BLOCK_SIZE > food_y) {
                
                // Crecer serpiente
                if (snake_length < MAX_LENGTH) {
                    snake_x[snake_length] = snake_x[snake_length-1];
                    snake_y[snake_length] = snake_y[snake_length-1];
                    snake_length++;
                }
                
                // Generar nueva comida
                food_active = false;
                seed = (seed * 1103515245 + 12345) % 2147483648;
                food_x = (seed % (LED_MATRIX_0_WIDTH - BLOCK_SIZE + 1));
                seed = (seed * 1103515245 + 12345) % 2147483648;
                food_y = (seed % (LED_MATRIX_0_HEIGHT - BLOCK_SIZE + 1));
                food_active = true;
            }
            
            // Limpiar pantalla 
            for (int i = 0; i < LED_MATRIX_0_WIDTH * LED_MATRIX_0_HEIGHT; i++) {
                led_matrix[i] = COLOR_OFF;
            }
            
            //  comida
            if (food_active) {
                for (int dy = 0; dy < BLOCK_SIZE; dy++) {
                    for (int dx = 0; dx < BLOCK_SIZE; dx++) {
                        int pos = (food_y + dy) * LED_MATRIX_0_WIDTH + (food_x + dx);
                        led_matrix[pos] = COLOR_FOOD;
                    }
                }
            }
            
            //  serpiente
            for (int i = 0; i < snake_length; i++) {
                for (int dy = 0; dy < BLOCK_SIZE; dy++) {
                    for (int dx = 0; dx < BLOCK_SIZE; dx++) {
                        int pos = (snake_y[i] + dy) * LED_MATRIX_0_WIDTH + (snake_x[i] + dx);
                        led_matrix[pos] = COLOR_SNAKE;
                    }
                }
            }
        }
        
        // Delay
        for (volatile int i = 0; i < 100; i++);
    }
    
    return 0;
}