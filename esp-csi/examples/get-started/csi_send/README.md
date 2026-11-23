# CSI_SEND

This firmware makes the sending ESP32 a STA node that connects to the
`ESP-CSI-AP` SoftAP created by `csi_recv`.

After joining the AP, it continuously pings the AP gateway at 100 Hz.
Those Ping packets trigger CSI frames for the receiver.

Configure `csi_recv` for the public Wi-Fi network first, flash it, and then
flash this firmware.
