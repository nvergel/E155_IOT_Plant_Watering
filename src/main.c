#include "STM32F401RE.h"
#include "main.h"
#include <string.h>
#include "UARTRingBuffer.h"
#include "Moisture_Sensor.h"

#define initial_probe_interval 60

uint8_t htmlPage[] = 
"<!DOCTYPE html>\
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
<title>IoT Plant Watering</title>\
<style> html{height: 100%;} body {text-align: center; margin-top: 0; height: 100%;}\
        input {width:1.5rem;}\
        div {background-color: #1c87c9; width: 50%; margin-left: 25%; height: 100%; top: 10%;\
             box-shadow: 0 1px 6px rgba(0, 0, 0, 0.12), 0 1px 4px rgba(0, 0, 0, 0.24);}\
        button {margin-left: 10rem; width:revert;}\
        input {background-color: #7ec3ed; width: 2rem}\
        h3 {margin: 0; padding: 1rem}\
        p {display: inline}\
</style>\
<div>\
<h3>IoT Plant Watering</h3>\
    <label for=\"MT\">Moisture Threshold (current value at xxx%):</label>\
    <input type=\"number\" id=\"MT\" name=\"MT\" min=\"1\" max=\"255\" value=\"1\">\
    <br><br><button onclick=\"updateData(\'MT\')\">Submit</button><br><br>\
    <label for=\"WT\">Water Time (current value at xxxsec):</label>\
    <input type=\"number\" id=\"WT\" name=\"WT\" min=\"1\" max=\"255\" value=\"1\">\
    <br><br><button onclick=\"updateData(\'WT\')\">Submit</button><br><br>\
    <p>Last measured value: <p id=\"LMV\">xxx</p>%</p>\
</div>\
<script>\
function updateData(input) {\
    const params = new URLSearchParams();\
    params.append(input, document.getElementById(input).value);\
    fetch(new Request('', {headers: params})).then(window.alert(\"Value successfully updated\"));\
}\
const updateParams = new URLSearchParams();\
updateParams.append(\"LMV\", \"\");\
setInterval(function(){\
    fetch(new Request('', {headers: updateParams})).then(response => response.text())\
    .then(text => {if (text.length < 4) document.getElementById(\"LMV\").innerText = text});\
}, 10000);\
</script>\r\n";

// void sendCommand(uint8_t* cmd) {
//     USART_TypeDef * ESP_USART = id2Port(ESP_USART_ID);
//     USART_TypeDef * TERM_USART = id2Port(TERM_USART_ID);

//     sendString(ESP_USART, cmd);
//     sendString(ESP_USART, "\r\n");
//     while(!transmitBufferEmpty());

//     uint8_t volatile str[512] = "";
//     readString(ESP_USART, str);
//     sendString(TERM_USART, str);
//     delay_millis(DELAY_TIM, CMD_DELAY_MS);
// }

/** Initialize the ESP and print out IP address to terminal
 */
void initESP8266(USART_TypeDef * ESP_USART, USART_TypeDef * TERM_USART){
    // Disable echo
    sendData("ATE0\r\n", 6, ESP_USART);
    delay_millis(DELAY_TIM, CMD_DELAY_MS);
    
    // Enable multiple connections
    sendData("AT+CIPMUX=1\r\n", 13, ESP_USART);
    delay_millis(DELAY_TIM, CMD_DELAY_MS);

    // Create TCP server on port 80
    sendData("AT+CIPSERVER=1,80\r\n", 19, ESP_USART);
    delay_millis(DELAY_TIM, CMD_DELAY_MS);

    // Change to mode 3 (AP + station )
    sendData("AT+CWMODE=3\r\n", 13, ESP_USART);
    delay_millis(DELAY_TIM, CMD_DELAY_MS);
    
    // Connect WiFi
    uint8_t connect_cmd[128] = "";
    sprintf(connect_cmd,"AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, PASSWORD);
    uint16_t cmdLen = strlen(connect_cmd);
    sendData(connect_cmd, cmdLen, ESP_USART);

    // Wait for connection
    delay_millis(DELAY_TIM, 10000);
    sendData("AT+CIFSR\r\n", 10, ESP_USART);
}

/** Map USART1 IRQ handler to our custom ISR
 */
void USART1_IRQHandler(){
    USART_TypeDef * ESP_USART = id2Port(ESP_USART_ID);
    usart_ISR(ESP_USART);
}

// /** Map USART2 IRQ handler to our custom ISR
//  */
// void USART2_IRQHandler(){
//     USART_TypeDef * TERM_USART = id2Port(TERM_USART_ID);
//     usart_ISR2(TERM_USART);
// }

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

uint8_t parseValue(uint8_t *buffer, uint32_t i){
    uint8_t value = 0;
    while (buffer[i] != '\r') { // Single digit number
        value = value*10 + buffer[i] - 48;
        ++i;
    } 
    return value;
}

void parseRequest(uint8_t *buffer, GET_Request *get_request){
	uint32_t bufferlength = strlen(buffer);
    uint8_t char1 = buffer[0];
    uint8_t char2 = buffer[1];
    uint8_t char3 = buffer[2];
	uint32_t i = 3;
    // i always ahead of char3, buffer[i] =  char4
    
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
        }
        // Search for moisture threshold
        else if (char1 == 'M' && char2 == 'T' && char3 == ':') {
            get_request->MT = 1;
            get_request->MT_val = parseValue(buffer, i+1);
        }
        // Search for water time
        else if (char1 == 'W' && char2 == 'T' && char3 == ':') {
            get_request->WT = 1;
            get_request->WT_val = parseValue(buffer, i+1);
        }
        // Search for water time
        else if (char1 == 'L' && char2 == 'M' && char3 == 'V') {
            get_request->LMV = 1;
        }
        char1 = char2;
        char2 = char3;
        char3 = buffer[i];
        ++i;
    }
}

void updateVal(uint8_t* htmlPos, uint8_t* val) {
    for (uint8_t i = 0; i < 3; ++i) {
        htmlPos[i] = val[i];
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

    initADC();

    initDMA();

    // Enable interrupts globally
    __enable_irq();

    // Configure interrupt for TIM3, USART1 and USART2
    *NVIC_ISER0 |= (1 << 29);
    *NVIC_ISER1 |= (1 << 5);
    // *NVIC_ISER1 |= (1 << 6);
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
    delay_millis(DELAY_TIM, 1000);
    initESP8266(ESP_USART, TERM_USART);
    delay_millis(DELAY_TIM, 500);
    printData("Ready");


    // Set up temporary buffers for requests
    uint8_t volatile http_request[BUFFER_SIZE] = "";
    uint8_t volatile temp_str[BUFFER_SIZE] = "";

    GET_Request get_request;
    get_request.htmlLen = strlen(htmlPage) + 1;
    uint8_t cmd[25] = "";
    // Send HTML
    sprintf(cmd, "AT+CIPSENDBUF=0,%d\r\n", get_request.htmlLen);
    uint16_t cmdLen = strlen(cmd);
    uint8_t j = 0;
    for (uint16_t i = 0; j < 3; ++i) {
        if (htmlPage[i] == 'x' && htmlPage[i+1] == 'x' && htmlPage[i+2] == 'x') {
            switch (j) {
                case 0:
                    get_request.ptrMT = htmlPage+i;
                case 1:
                    get_request.ptrWT = htmlPage+i;
                case 2:
                    get_request.ptrLMV = htmlPage+i;
            }
            ++j;
        }
    }

    uint8_t paramHolder[5] = "";
    sprintf(paramHolder, "%d  ", moistureThreshold);
    updateVal(get_request.ptrMT, paramHolder);

    sprintf(paramHolder, "%d  ", WATER_TIME_SECONDS);
    updateVal(get_request.ptrWT, paramHolder);

    sprintf(paramHolder, "%d  ", moisture);
    updateVal(get_request.ptrLMV, paramHolder);

    while(1) {
        memset(http_request, 0, BUFFER_SIZE);
        //http_request[0] = "\0";

        // Clear temp_str buffer
        get_request.GET = 0;
        get_request.FAV = 0;
        get_request.MT = 0;
        get_request.WT = 0;
        get_request.LMV = 0;

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
            printData(http_request);

            parseRequest(http_request, &get_request);
            uint8_t paramHolder[15] = "";

            if ( get_request.MT) {
                setMoistureThreshold(get_request.MT_val);
                // sprintf(paramHolder, "value=\"%d\">", moistureThreshold);
                // sendString(TERM_USART, paramHolder);
                // serveWebpage("");
                sendData("AT+CIPCLOSE=0\r\n", 15, ESP_USART);
            }
            
            if ( get_request.WT) {
                setWaterTime(get_request.WT_val);
                //sprintf(paramHolder, "value=\"%d\">", WATER_TIME_SECONDS);
                //sendString(TERM_USART, paramHolder);
                //serveWebpage("");
                sendData("AT+CIPCLOSE=0\r\n", 15, ESP_USART);
            }

            if (get_request.LMV) {
                sprintf(paramHolder, "%d", moisture);
                sendData(paramHolder, strlen(paramHolder), ESP_USART);
                sendData("AT+CIPCLOSE=0\r\n", 15, ESP_USART);
            } else if (get_request.GET && !get_request.FAV && !get_request.WT && !get_request.MT){
                printData("Get request received, sending html page");
                sendData(cmd, cmdLen, ESP_USART);
                delay_millis(DELAY_TIM, CMD_DELAY_MS);
                sendData(htmlPage, get_request.htmlLen, ESP_USART);
                delay_millis(DELAY_TIM, CMD_DELAY_MS);
                sendData("AT+CIPCLOSE=0\r\n", 15, ESP_USART);
                delay_millis(DELAY_TIM, CMD_DELAY_MS);
            }
        }
    }
}