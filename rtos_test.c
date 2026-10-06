typedef unsigned long size_t;
void *malloc(size_t size);

// Add the mock declarations so Clang knows they exist
void sleep(int seconds);
void vTaskDelay(unsigned int ticks);

__attribute__((interrupt)) void my_isr_handler() {
    // Let's break all the rules your JSON file is looking for
    void* data = malloc(16);
    sleep(10);
    vTaskDelay(100);
}