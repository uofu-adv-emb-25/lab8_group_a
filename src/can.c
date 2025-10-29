#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <pico/stdlib.h>
#include <can2040.h>

#define QUEUE_LENGTH 32

static QueueHandle_t queue;
static struct can2040 cbus;

static void can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg)
{
    if (notify == CAN2040_NOTIFY_RX) {
        xQueueSend(queue, msg, portMAX_DELAY);
    }
}

static void PIOx_IRQHandler(void)
{
    can2040_pio_irq_handler(&cbus);
}

static void canbus_setup(void)
{
    uint32_t pio_num = 0;
    uint32_t sys_clock = 125000000, bitrate = 500000;
    uint32_t gpio_rx = 4, gpio_tx = 5;

    // Setup queue
    queue = xQueueCreate(QUEUE_LENGTH, sizeof (struct can2040_msg));

    // Setup canbus
    can2040_setup(&cbus, pio_num);
    can2040_callback_config(&cbus, can2040_cb);

    // Enable irqs
    irq_set_exclusive_handler(PIO0_IRQ_0, PIOx_IRQHandler);
    irq_set_priority(PIO0_IRQ_0, PICO_DEFAULT_IRQ_PRIORITY - 1);
    irq_set_enabled(PIO0_IRQ_0, 1);

    // Start canbus
    can2040_start(&cbus, sys_clock, bitrate, gpio_rx, gpio_tx);
}

static void send_task(void *params)
{
    while (1) {
        struct can2040_msg msg;
        msg.id = 0;
        msg.dlc = 8;
        msg.data32[0] = 0x01234567;
        msg.data32[1] = 0x89abcdef;
        can2040_transmit(&cbus, &msg);
        vTaskDelay(1000);
    }
}

static void recv_task(void *params)
{
    while (1) {
        struct can2040_msg msg;
        xQueueReceive(queue, &msg, portMAX_DELAY);
        printf("received message: id=%d dlc=%d data32[0]=%d data32[1]=%d\n", msg.id, msg.dlc, msg.data32[0], msg.data32[1]);
    }
}

int main(void)
{
    stdio_init_all();
    canbus_setup();

    xTaskCreate(send_task, "send", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL);
    xTaskCreate(recv_task, "recv", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL);
    vTaskStartScheduler();
}
