#include "ripes_system.h"
#include <stdbool.h>


#define COLOR_LED   0xFF0000  // Rojo
#define COLOR_OFF   0x000000  // Apagado


#define BLOCK_SIZE 2


volatile unsigned int* led_matrix = (unsigned int*)LED_MATRIX_0_BASE;
volatile unsigned int* d_pad_up = (unsigned int*)D_PAD_0_UP;
volatile unsigned int* d_pad_down = (unsigned int*)D_PAD_0_DOWN;
volatile unsigned int* d_pad_left = (unsigned int*)D_PAD_0_LEFT;
volatile unsigned int* d_pad_right = (unsigned int*)D_PAD_0_RIGHT;

// dirección
typedef enum {
    STOP,
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;

// limpiar matriz
void clear_leds() {
    for (int i = 0; i < LED_MATRIX_0_WIDTH * LED_MATRIX_0_HEIGHT; i++) {
        led_matrix[i] = COLOR_OFF;
    }
}

// delay
void delay(int cycles) {
    for (volatile int i = 0; i < cycles; i++);
}


void draw_block(int x, int y, unsigned int color) {
    for (int dy = 0; dy < BLOCK_SIZE; dy++) {
        for (int dx = 0; dx < BLOCK_SIZE; dx++) {
            int pos_x = x + dx;
            int pos_y = y + dy;
            
            if (pos_x >= 0 && pos_x < LED_MATRIX_0_WIDTH && 
                pos_y >= 0 && pos_y < LED_MATRIX_0_HEIGHT) {
                led_matrix[pos_y * LED_MATRIX_0_WIDTH + pos_x] = color;
            }
        }
    }
}

int main() {
    // centro 
    int x = (LED_MATRIX_0_WIDTH / 2) - 1;
    int y = (LED_MATRIX_0_HEIGHT / 2) - 1;
    
    Direction current_dir = STOP;
    Direction new_dir = STOP;
    
    // Inicializar
    clear_leds();
    draw_block(x, y, COLOR_LED);
    
    while (1) {
        // Leer dirección del D-Pad
        if (*d_pad_up) {
            new_dir = UP;
        } else if (*d_pad_down) {
            new_dir = DOWN;
        } else if (*d_pad_left) {
            new_dir = LEFT;
        } else if (*d_pad_right) {
            new_dir = RIGHT;
        }
        
        // Solo actualizar dirección si hay un cambio
        if (new_dir != STOP && new_dir != current_dir) {
            current_dir = new_dir;
        }
        
        // Mover
        if (current_dir != STOP) {
            draw_block(x, y, COLOR_OFF);
            
            // Calcular nueva posición
            switch (current_dir) {
                case UP:
                    y = (y > 0) ? y - 1 : LED_MATRIX_0_HEIGHT - BLOCK_SIZE;
                    break;
                case DOWN:
                    y = (y < LED_MATRIX_0_HEIGHT - BLOCK_SIZE) ? y + 1 : 0;
                    break;
                case LEFT:
                    x = (x > 0) ? x - 1 : LED_MATRIX_0_WIDTH - BLOCK_SIZE;
                    break;
                case RIGHT:
                    x = (x < LED_MATRIX_0_WIDTH - BLOCK_SIZE) ? x + 1 : 0;
                    break;
                default:
                    break;
            }
            
            // Encender nueva posición
            draw_block(x, y, COLOR_LED);
        }
        
        // Delay
        delay(1000);
    }
    
    return 0;
}