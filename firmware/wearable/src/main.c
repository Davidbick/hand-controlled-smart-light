#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "imu.h"

LOG_MODULE_REGISTER(wearable, LOG_LEVEL_INF);

#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led =
    GPIO_DT_SPEC_GET(LED0_NODE, gpios);

static void blink_work_handler(struct k_work *work);

K_WORK_DELAYABLE_DEFINE(blink_work, blink_work_handler);

static void blink_work_handler(struct k_work *work)
{
    ARG_UNUSED(work);

    gpio_pin_toggle_dt(&led);

    LOG_INF("Uptime: %lld ms",
            (long long)k_uptime_get());

    k_work_reschedule(&blink_work, K_MSEC(500));
}

int main(void)
{
    if (!gpio_is_ready_dt(&led)) {
        return 0;
    }

    gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);

    LOG_INF("Hand Controlled Smart Light starting");

    k_work_schedule(&blink_work, K_NO_WAIT);

    k_msleep(3000);

    int imu_status = imu_init();

    LOG_INF("imu_init() returned %d", imu_status);

    while (1) {
        if (imu_status < 0) {

            LOG_ERR("IMU unavailable, status=%d", imu_status);

        } else {
            struct imu_accel_sample accel;

            int ret = imu_read_accel(&accel);

            if (ret == 0) {
                LOG_INF("Accel [mg]: x=%d y=%d z=%d",
                        accel.x_mg,
                        accel.y_mg,
                        accel.z_mg);
            } else {
                LOG_ERR("Accelerometer read failed: %d", ret);
            }
        }
        
        k_msleep(1000);
    }

    return 0;
}