# Data leakage in Wi-Fi CSI human action recognition

- **Sources:** [MDPI Sensors 24(10):3159](https://www.mdpi.com/1424-8220/24/10/3159), [MDPI Information 9(4):90](https://www.mdpi.com/2411-5134/9/4/90)
- **Use here:** evaluation requirements

## Algorithm finding

Adjacent CSI windows from the same recording are highly correlated. Randomly
splitting those windows can put nearly identical samples in both training and
test sets and produce an overly optimistic score.

## Required evaluation

For any future learned detector, report at least:

- leave-one-person-out or person-disjoint testing;
- leave-one-room-out testing where applicable;
- separate recording sessions;
- empty room, static person, moving person, fan, door, and Wi-Fi traffic
  interference cases.
