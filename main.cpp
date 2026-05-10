#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
#define UART_ID uart0
#define BAUD_RATE 9600

// Use pins 4 and 5 for UART1
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define UART_TX_PIN 0
#define UART_RX_PIN 1



int main()
{
    stdio_init_all();

    printf("Starting GPS");
    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    // Use some the various UART functions to send out data
    // In a default system, printf will also output via the default UART
    
    // Send out a string, with CR/LF conversions
    // uart_puts(UART_ID, " Hello, UART!\n");
    char line[255];
    int i = 0;
    
    // For more examples of UART use see https://github.com/raspberrypi/pico-examples/tree/master/uart

    while (true) {
        while (uart_is_readable(UART_ID)) {
            char c = uart_getc(UART_ID);

            if (c == '\r') {
                continue;
            }

            if (c == '\n') {
                line[i] = '\0';
                if (i > 0) {
                    printf("%s\n", line);
                }
                i = 0;
            }
            else {
                if (i < (int)sizeof(line) - 1) {
                    line[i++] = c;
                } else {
                    printf("%s", line);
                    i = 0;
                }
            }
        }
        sleep_ms(5);
    }

    return 0;
}
