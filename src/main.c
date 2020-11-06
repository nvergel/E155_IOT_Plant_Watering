/**
    Main: Contains main function
    @file main.c
    @author Josh Brake
    @version 1.0 10/7/2020
*/

#include "STM32F401RE.h"
#include "main.h"
#include <string.h>
#include "UARTRingBuffer.h"

void sendCommand(uint8_t* cmd) {
    USART_TypeDef * ESP_USART = id2Port(ESP_USART_ID);
    USART_TypeDef * TERM_USART = id2Port(TERM_USART_ID);

    sendString(ESP_USART, cmd);
    sendString(ESP_USART, "\r\n");
    while(!transmitBufferEmpty());

    uint8_t volatile str[512] = "";
    readString(ESP_USART, str);
    sendString(TERM_USART, str);
    delay_millis(DELAY_TIM, CMD_DELAY_MS);
}

/** Initialize the ESP and print out IP address to terminal
 */
void initESP8266(USART_TypeDef * ESP_USART, USART_TypeDef * TERM_USART){
    // Disable echo
    sendCommand("ATE0");
    
    // Enable multiple connections
    sendCommand("AT+CIPMUX=1");

    // Create TCP server on port 80
    sendCommand("AT+CIPSERVER=1,80");

    // Change to mode 3 (AP + station )
    sendCommand("AT+CWMODE=3");


    // Connect to WiFi network
    uint8_t connect_cmd[128] = "";
    sprintf(connect_cmd,"AT+CWJAP=\"%s\",\"%s\"", SSID, PASSWORD);
    sendCommand(connect_cmd);

    // Wait for connection
    delay_millis(DELAY_TIM, 10000);

    // Print out status
    sendCommand("AT+CIFSR");
}

/** Send command to ESP and echo to the terminal.
    @param C-string (i.e., pointer to start of a null-terminated array
        of characters.
*/
void serveWebpage(uint8_t str []) {
    int str_length = strlen(str)+2;
    uint8_t cmd[512] = "";

    // Send HTML
    sprintf(cmd, "AT+CIPSEND=0,%d", str_length);
    sendCommand(cmd);

    sendCommand(str);
}

/** Map USART1 IRQ handler to our custom ISR
 */
void USART1_IRQHandler(){
    USART_TypeDef * ESP_USART = id2Port(ESP_USART_ID);
    usart_ISR(ESP_USART);
}

/** Map USART2 IRQ handler to our custom ISR
 */
void USART2_IRQHandler(){
    USART_TypeDef * TERM_USART = id2Port(TERM_USART_ID);
    usart_ISR2(TERM_USART);
}

int main(void) {
    // Configure flash latency and set clock to run at 84 MHz
    configureFlash();
    configureClock();

    // Enable GPIOA clock
    RCC->AHB1ENR.GPIOAEN = 1;

    // Initialize timer
    RCC->APB1ENR |= (1 << 0); // TIM2_EN
    initTIM(DELAY_TIM);

    // Set up LED pin as output 
    pinMode(GPIOA, GPIO_PA5, GPIO_INPUT);
    pinMode(GPIOA, GPIO_PA6, GPIO_OUTPUT);

    // Configure ESP and Terminal UARTs
    USART_TypeDef * ESP_USART = initUSART(ESP_USART_ID, 115200);
    USART_TypeDef * TERM_USART = initUSART(TERM_USART_ID, 115200);

    ADCInit();

    // Configure USART1 interrupt
    // Enable interrupts globally
    __enable_irq();

    // Configure interrupt for USART1 and USART2
    *NVIC_ISER1 |= (1 << 5);
    *NVIC_ISER1 |= (1 << 6);
    ESP_USART->CR1.RXNEIE = 1;
    
    // Initialize ring buffer
    init_ring_buffer();
    flush_buffer();

    // Initialize ESP
    delay_millis(DELAY_TIM, 1000);
    initESP8266(ESP_USART, TERM_USART);
    delay_millis(DELAY_TIM, 500);

    // Set up temporary buffers for requests
    uint8_t volatile http_request[BUFFER_SIZE] = "";
    uint8_t volatile temp_str[BUFFER_SIZE] = "";
    uint16_t samples = 1;
    uint16_t sound[samples];
    uint16_t moisture;
    
    while(1) {
        // Clear temp_str buffer
        memset(http_request, 0, BUFFER_SIZE);
        // volatile uint32_t http_req_len;

        // Loop through and read any data available in the buffer
        if(is_data_available()) {
            do{
                memset(temp_str, 0, BUFFER_SIZE);
                readString(ESP_USART, temp_str); // Read in available bytes
                strcat(http_request, temp_str); // Append to current http_request string
                // http_req_len = strlen(http_request); // Store length of request
                delay_millis(DELAY_TIM, CMD_DELAY_MS); // Delay
            } while(is_data_available()); // Check for end of transaction

            // Echo received string to the terminal
            sendString(TERM_USART, http_request);

            // Search to see if there was a GET request
            volatile uint8_t get_request = look_for_substring("GET", http_request);
            volatile uint8_t favicon_req = look_for_substring("favicon", http_request);

            // If a GET request, process the request
            if(get_request && !favicon_req){
                // Look for "REQ" in http_request
                // volatile uint8_t button_req = look_for_substring("REQ", http_request);
                volatile uint8_t record_req = look_for_substring("=REC", http_request);

                // Serve the individual HTML commands for the webpage
                serveWebpage("<!DOCTYPE html>");
                serveWebpage("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
                serveWebpage("<title>ESP8266 Demo</title>");
                serveWebpage("<style> body {background-color: #1c87c9; text-align: center;}</style>");
                serveWebpage("<h3>ESP8266</h3>");
                serveWebpage("<p>Welcome to MicroPs IoT lab!</p>");

                // Read if LED is on or off and display to webpage.
                if(record_req){
                    serveWebpage("<p>Recording in process, wait a few seconds</p>");
                    serveWebpage("<form action=\"REQ=REL\"><input type=\"submit\" value = \"Reload\"></form>");
                } else {
                    serveWebpage("<form action=\"REQ=REC\"><input type=\"submit\" value = \"Begin Recording\"></form>");
                }

                // Close connection
                sendCommand("AT+CIPCLOSE=0");
                /*
                if (record_req) {
                    ADCmeasure(sound, samples);
                    sendString(TERM_USART, "\r\nV[:100]=");
                    for (int i = 0; i < 200; ++i) {
                        uint8_t cmd[10] = "";
                        sprintf(cmd, "%d, ", sound[i]);
                        sendString(TERM_USART, cmd);
                    }
                    sendString(TERM_USART, "\r\n");
                }
                */
               if (record_req) {
                    moisture = ADCsingle_measure();
                    //sendString(TERM_USART, "\r\nV[:100]=");
                    
                    uint8_t cmd[10] = "";
                    sprintf(cmd, "%d, ", moisture);
                    sendString(TERM_USART, cmd);
                    
                    sendString(TERM_USART, "\r\n");
                }
            }
        }
    }
}