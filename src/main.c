#include "STM32F401RE.h"
#include "main.h"
#include <string.h>
#include "UARTRingBuffer.h"
#include "Moisture_Sensor.h"

#define initial_probe_interval 60

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

void TIM3_IRQHandler() {
    probe();
    TIM3->SR &= ~(0x1); // Clear UIF
}

void parseRequest(uint8_t *buffer, GET_Request *get_request){
	uint32_t bufferlength = strlen(buffer);
    uint8_t char1 = buffer[0];
    uint8_t char2 = buffer[1];
    uint8_t char3 = buffer[2];
	uint32_t i = 3;
    
    while (i < bufferlength-2) {
        if (char1 == 'G' && char2 == 'E' && char3 == 'T') {
            get_request->GET = 1;
            break;
        }
        char1 = char2;
        char2 = char3;
        char3 = buffer[i];
        ++i;
    }
    while (i < bufferlength-2 && get_request->GET) {
        if (char1 == 'f' && char2 == 'a' && char3 == 'v') {
            get_request->FAV = 1;
            break;
        }
        // Search for moisture threshold
        if (char1 == 'M' && char2 == 'T' && char3 == '=') {
            get_request->MT = 1;
            if (buffer[i] == ' ') { // Empty form
                get_request->MT = 0;
            }else if(buffer[i+1] == ' ') { // Single digit number
                get_request->MT_val = buffer[i] - 48; // Convert char to int
            } else {
                volatile uint8_t decimalValue = (buffer[i] - 48)*10;
                get_request->MT_val = decimalValue + buffer[i+1] - 48; // Convert char to int
            }
        }
        // Search for water time
        if (char1 == 'W' && char2 == 'T' && char3 == '=') {
            get_request->WT = 1;
            if (buffer[i] == ' ') { // Empty form
                get_request->WT = 0;
            }else if(buffer[i+1] == ' ') { // Single digit number
                get_request->WT_val = buffer[i] - 48; // Convert char to int
            } else {
                volatile uint8_t decimalValue = (buffer[i] - 48)*10;
                get_request->WT_val = decimalValue + buffer[i+1] - 48; // Convert char to int
            }
        }
        char1 = char2;
        char2 = char3;
        char3 = buffer[i];
        ++i;
    }
}

int main(void) {
    // Configure flash latency and set clock to run at 84 MHz
    configureFlash();
    configureClock();

    // Enable GPIOA clock
    RCC->AHB1ENR.GPIOAEN = 1;

    // Initialize timer
    RCC->APB1ENR |= (1 << 0); // TIM2_EN
    RCC->APB1ENR |= (1 << 1); // TIM3_EN
    initTIM(DELAY_TIM);
    setTimer(TIM3, initial_probe_interval); // Probe every minute

    // Configure ESP and Terminal UARTs
    USART_TypeDef * ESP_USART = initUSART(ESP_USART_ID, 115200);
    USART_TypeDef * TERM_USART = initUSART(TERM_USART_ID, 115200);

    ADCInit();

    // Configure USART1 interrupt
    // Enable interrupts globally
    __enable_irq();

    // Configure interrupt for TIM3, USART1 and USART2
    *NVIC_ISER0 |= (1 << 29);
    *NVIC_ISER1 |= (1 << 5);
    *NVIC_ISER1 |= (1 << 6);
    ESP_USART->CR1.RXNEIE = 1;
    TIM3->DIER |= 1;
    
    // Initialize ring buffer
    init_ring_buffer();
    flush_buffer();

    init_moisture_sensor();
    flush_moisture_buffer();
    setWaterTime(30); // Initial water time set to 30 secs, can be changed through website
    setProbeInterval(initial_probe_interval); // Set probe interval

    // Initialize moisture threshold to 10%
    setMoistureThreshold(10);

    // Initialize ESP
    delay_millis(DELAY_TIM, 1000);
    initESP8266(ESP_USART, TERM_USART);
    delay_millis(DELAY_TIM, 500);

    // Set up temporary buffers for requests
    uint8_t volatile http_request[BUFFER_SIZE] = "";
    uint8_t volatile temp_str[BUFFER_SIZE] = "";

    GET_Request get_request;
    
    while(1) {
        // Clear temp_str buffer
        memset(http_request, 0, BUFFER_SIZE);
        get_request.GET = 0;
        get_request.FAV = 0;
        get_request.MT = 0;

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

            parseRequest(http_request, &get_request);

            // Debug code
            uint8_t cmd[15] = "";
            sprintf(cmd, "\nGET %d\n", get_request.GET);
            sendString(TERM_USART, cmd);
            sprintf(cmd, "Fav %d\n", get_request.FAV);
            sendString(TERM_USART, cmd);
            sprintf(cmd, "MT %d\n", get_request.MT);
            sendString(TERM_USART, cmd);
            sprintf(cmd, "Val %d\n", get_request.MT_val);
            sendString(TERM_USART, cmd);

            if ( get_request.MT) {
                setMoistureThreshold(get_request.MT_val);
            }

            if ( get_request.MT) {
                setWaterTime(get_request.MT_val);
            }

            // Search to see if there was a GET request
            // volatile uint8_t get_request1 = look_for_substring("GET", http_request);
            // volatile uint8_t favicon_req = look_for_substring("favicon", http_request);

            // If a GET request, process the request
            if(get_request.GET && !get_request.FAV){
                // Look for "REQ" in http_request
                // volatile uint8_t button_req = look_for_substring("REQ", http_request);
                // volatile uint8_t record_req = look_for_substring("=REC", http_request);

                // Serve the individual HTML commands for the webpage
                serveWebpage("<!DOCTYPE html><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
                serveWebpage("<title>IoT Plant Watering</title><style> body {background-color: #1c87c9; text-align: center;} input {width:1.5rem;}</style>");
                serveWebpage("<h3>IoT Plant Watering</h3><form>");

                serveWebpage("<label for=\"MT\">Moisture Threshold:</label>");
                serveWebpage("<input type=\"number\" id=\"MT\" name=\"MT\" min=\"1\" max=\"99\"");
                uint8_t val[15] = "";
                sprintf(val, "value=\"%d\">", moistureThreshold);
                serveWebpage(val);

                serveWebpage("<br><br><label for=\"WT\">Water Time:</label>");
                serveWebpage("<input type=\"number\" id=\"WT\" name=\"WT\" min=\"1\" max=\"99\"");
                sprintf(val, "value=\"%d\">", WATER_TIME_SECONDS);
                serveWebpage(val);

                serveWebpage("<br><br><input type=\"submit\" style=\"margin-left: 10rem; width:revert;\"></form>");

                serveWebpage("<p>Last measured values: ");
                for (int i = 0; i < _moisture_buffer->tail; ++i) {
                    uint8_t val[10] = "";
                    sprintf(val, "%d, ", _moisture_buffer->buffer[i]);
                    serveWebpage(val);
                }
                serveWebpage("</p>");

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
            }

            if (get_request.GET) {
                // Close connection
                sendCommand("AT+CIPCLOSE=0");
            }
        }
    }
}