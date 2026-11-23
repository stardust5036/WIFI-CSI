# CSI_RECV

This firmware uses AP+STA mode:

- creates the `ESP-CSI-AP` SoftAP for `csi_send`;
- connects its STA interface to the configured public/local Wi-Fi network;
- receives CSI frames triggered by Ping traffic from `csi_send`;
- broadcasts raw CSI payloads in `CSIU` UDP packets on port `5005`.

Configure the public Wi-Fi SSID, password, SoftAP channel, UDP destination IP,
and UDP port through `idf.py menuconfig` under `CSI Receive Configuration`.

Set `CSI SoftAP channel` to the same channel used by the public Wi-Fi AP.
APSTA mode requires the SoftAP and STA interfaces to use compatible channels.

Set `UDP destination IP` to `255.255.255.255` for broadcast, or enter a
specific reachable IPv4 address for unicast. The UDP packet is sent through
the STA interface. For broadcast, the receiver host must be on the same
public/local Wi-Fi subnet.
