#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>

class SensorData {
public:
    std::vector<int> readings;
    
    void process() {
        void* ptr = malloc(1024);
        std::cout << "Allocated memory: " << ptr << std::endl;
    }
};

bool verify_checksum(int data) {
    return data > 0;
}

void parse_raw_payload() {
    SensorData sensor;
    sensor.process(); 
}

void read_sensor_bus() {
    verify_checksum(42);
    parse_raw_payload();
}

#define ISR __attribute__((interrupt("IRQ")))

ISR void hardware_interrupt_handler() {
    read_sensor_bus();
}