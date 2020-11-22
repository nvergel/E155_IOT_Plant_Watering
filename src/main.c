#include "STM32F401RE.h"
#include "main.h"
#include <string.h>
#include "UARTRingBuffer.h"
#include "Moisture_Sensor.h"

#define initial_probe_interval 60

uint8_t htmlOpen[] = 
"<!DOCTYPE html>\
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
<title>IoT Plant Watering</title>\
<style> html{height: 100%;} body {text-align: center; margin-top: 0; height: 100%;}\
        input {width:1.5rem;}\
        div {background-color: #1c87c9; width: 50%; margin-left: 25%; height: 100%; top: 10%;\
             box-shadow: 0 1px 6px rgba(0, 0, 0, 0.12), 0 1px 4px rgba(0, 0, 0, 0.24);}\
        button {margin-left: 10rem; width:revert;}\
        input {background-color: #7ec3ed; width: 2rem}\
        h3 {margin: 0; padding-top: 1rem}\
</style>\
<div>\
<h3>IoT Plant Watering</h3>";

uint8_t htmlClose[] = 
"</div>\
<script>\
function updateData(input) {\
    const params = new URLSearchParams();\
    params.append(input, document.getElementById(input).value);\
    fetch(new Request('', {headers: params})).then(console.log(\"done\"));\
}\
</script>";

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

void serveWebpage(uint8_t str []) {
    int str_length = strlen(str)+2;
    uint8_t cmd[512] = "";

    // Send HTML
    sprintf(cmd, "AT+CIPSENDBUF=0,%d", str_length);
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

/** TIM3 handles probing moisture sensor and watering plant
 */
void TIM3_IRQHandler() {
    if (pumpOn) {
        stopWaterPlant();
    } else {
        probe();
    }
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
        if (char1 == 'M' && char2 == 'T' && char3 == ':') {
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
        if (char1 == 'W' && char2 == 'T' && char3 == ':') {
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
    //set PA5 "WATER_PUMP" to output mode
    pinMode(GPIOA, WATER_PUMP, GPIO_OUTPUT);

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

    setWaterTime(30); // Initial water time set to 30 secs, can be changed through website
    setProbeInterval(initial_probe_interval); // Set probe interval

    // Initialize moisture threshold to 10%
    setMoistureThreshold(10);

    // Initialize ESP
    // delay_millis(DELAY_TIM, 1000);
    // initESP8266(ESP_USART, TERM_USART);
    // delay_millis(DELAY_TIM, 500);
    sendString(TERM_USART, "Uncomment initESP8266 if not initialized");


    // Set up temporary buffers for requests
    uint8_t volatile http_request[BUFFER_SIZE] = "";
    uint8_t volatile temp_str[BUFFER_SIZE] = "";

    GET_Request get_request;
    
    while(1) {
        memset(http_request, 0, BUFFER_SIZE);

        // Clear temp_str buffer
        get_request.GET = 0;
        get_request.FAV = 0;
        get_request.MT = 0;

        if (lowMoisture) {
            waterPlant();
        }

        // Loop through and read any data available in the buffer
        if (is_data_available()){
            
            do{
                memset(temp_str, 0, BUFFER_SIZE);
                readString(ESP_USART, temp_str); // Read in available bytes
                strcat(http_request, temp_str); // Append to current http_request string
                delay_millis(DELAY_TIM, CMD_DELAY_MS); // Delay
            } while(is_data_available()); // Check for end of transaction

            // Echo received string to the terminal
            sendString(TERM_USART, http_request);

            parseRequest(http_request, &get_request);

            if ( get_request.MT) {
                setMoistureThreshold(get_request.MT_val);
            }

            if ( get_request.MT) {
                setWaterTime(get_request.MT_val);
            }

            // If a GET request, process the request
            if(get_request.GET && !get_request.FAV){
                // Serve the individual HTML commands for the webpage
                serveWebpage(htmlOpen);

                // Adjustable params
                uint8_t paramHolder[15] = "";

                serveWebpage("<label for=\"MT\">Moisture Threshold(%):</label>\
                                <input type=\"number\" id=\"MT\" name=\"MT\" min=\"1\" max=\"99\"");
                sprintf(paramHolder, "value=\"%d\">", moistureThreshold);
                serveWebpage(paramHolder);
                serveWebpage("<br><br><button onclick=\"updateData(\'MT\')\">Submit</button>");

                serveWebpage("<br><br><label for=\"WT\">Water Time(sec):</label>\
                                <input type=\"number\" id=\"WT\" name=\"WT\" min=\"1\" max=\"99\"");
                sprintf(paramHolder, "value=\"%d\">", WATER_TIME_SECONDS);
                serveWebpage(paramHolder);

                serveWebpage("<br><br><button onclick=\"updateData(\'WT\')\">Submit</button>");

                // Stats
                serveWebpage("<p>Last measured value: ");
                sprintf(paramHolder, "%d", moisture);
                serveWebpage(paramHolder);
                serveWebpage("%</p>");

                serveWebpage(htmlClose);
            }

            if (get_request.GET) {
                // Close connection
                sendCommand("AT+CIPCLOSE=0");
            }
        }
    }
}