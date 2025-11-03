# Renode setup
The Raspberry Pico needs configuration files for Renode to work properly.

* On MacOS, the installation location is `/Applications/Renode.app/Contents/MacOs`
* On Linux, the location for Debian, Fedora, and Arch is `/opt/renode`
* On Windows, the location is `C://Program Files/Renode`

To add the Pico configuration files:
1. Copy `rp2040_spinlock.py` and `rp2040_divider.py` to the `scripts/pydev` directory of your Renode installation.
1. Copy `rpi_pico_rp2040_w.repl` to the `platforms/cpus` directory.


# Babbling Node Results
- If one controller is set to send messages continuously with high priority over the receiving task (no vTaskDelay), the other controller receives a constant stream of messages, while the controller that is babbling receives no messages.
- Adding in a vTaskDelay of any number of ticks allows the babbling controller to receive messages again. This makes sense with the way FreeRTOS scheduling works as this would place the higher priority task in a Blocked state, allowing the lower priority receiving function to run.

-If the receiving and sending threads are set to the same priority, a busy_wait_us of 5ms allows the babbling controller to receive messages again.
-It seems at a value of 0.3ms for the busy_wait loop, the babbling controller is unable to receive messages again. Or maybe occasionally, but at a very low frequency than is expected for the period set on the other controller.
