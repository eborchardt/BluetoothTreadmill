# BluetoothTreadmill

The objective of this project was to reverse engineer the serial communication from the treadmill user interface to the treadmill motor control board to see if I can extract the current speed and incline data. Then, send that information to Zwift using BLE FTMS so I can accurately track my treadmill workouts. I have a Livestrong LS8.0T, which is manufactored by Johnson Fitness. I was initially intending to use a magnet or rotary encoder to collect the speed, but upon disassembly, I discovered a MAX3085 that appeared to connect the controls to the motor control board. After a little investigating, I found the serial communication between the controls and the motor controller to be easy to decode.

I wrote this program for an ESP32 in Arduino, since it has three serial ports and I had one laying around. The Serial2 Rx pin of the ESP32 is connected directly to the low side of the MAX3085 Tx IC inside the control panel and I also connected the ground pins to improve the reliability of the data.

# TODO
The code desperately needs to be cleaned up, but that's for another day.

# Bluetooth FTMS
I was able to find several helpful sources in getting this working. Obviously, the Bluetooth FTMS Specifications were instrumental.
* https://www.bluetooth.com/specifications/specs/fitness-machine-service-1-0/

This project is not written in Arduino, but I referred to it quite a bit.
* https://github.com/lefty01/ESP32_TTGO_FTMS

There is also a discussion in the Zwift commmunity that was helpful:
* https://forums.zwift.com/t/show-us-your-zwift-setup/59647/27

# Serial Commands from Treadmill
I found what appears to be four types of commands being transmitted to the motor controller, speed, incine, stop, and one that I'm assuming is simply a keep-alive heartbeat. The basic idea seems to be that all commands begin with `0 255`. Byte3 will determine the type of command, speed is 241, incline is 246. It's possible that byte4 carries some meaning as well, but I didn't investigate any further. 

There is no terminating character on each command, so I needed to look for the pattern of `0 255` to detect the start of a command. Then I used byte3 to determine how many more bytes to expect and grabbed that many more bytes from the serial buffer. In the case of the speed and incline commands, they are both a total of 7 bits. 

EDIT: After more observations, I think I've come to find the meaning of byte4. It seems to be an indicator of how many more bytes to expect. I don't think I'll rework my functions, but I'll leave this here for future reference.
```
byte4 = 255 | No more bytes
      = 0   | 1 more byte
      = 1   | 2 more bytes
      = 2   | 3 more bytes
```
Here is what I found for my treadmill:

# Speed Table
| byte1 | byte2 | byte3 | byte4 | byte5 | byte6 | byte7 | Observed Speed When Sent (mph) |
|-------|-------|-------|-------|-------|-------|-------|-----------------------------------|
| 0     | 255   | 241   | 2     | 0     | 0     | 221   | 0                                 |
| 0     | 255   | 241   | 2     | 0     | 161   | 16    | 0.5                               |
| 0     | 255   | 241   | 2     | 0     | 195   | 201   | 0.6                               |
| 0     | 255   | 241   | 2     | 0     | 230   | 186   | 0.7                               |
| 0     | 255   | 241   | 2     | 1     | 8     | 144   | 0.8                               |
| 0     | 255   | 241   | 2     | 1     | 42    | 116   | 0.9                               |
| 0     | 255   | 241   | 2     | 1     | 76    | 105   | 1                                 |
| 0     | 255   | 241   | 2     | 1     | 111   | 188   | 1.1                               |
| 0     | 255   | 241   | 2     | 1     | 145   | 33    | 1.2                               |
| 0     | 255   | 241   | 2     | 1     | 179   | 197   | 1.3                               |
| 0     | 255   | 241   | 2     | 1     | 213   | 216   | 1.4                               |
| 0     | 255   | 241   | 2     | 1     | 248   | 18    | 1.5                               |
| 0     | 255   | 241   | 2     | 2     | 26    | 156   | 1.6                               |
| 0     | 255   | 241   | 2     | 2     | 60    | 188   | 1.7                               |
| 0     | 255   | 241   | 2     | 2     | 94    | 101   | 1.8                               |
| 0     | 255   | 241   | 2     | 2     | 129   | 79    | 1.9                               |
| 0     | 255   | 241   | 2     | 2     | 163   | 171   | 2                                 |
| 0     | 255   | 241   | 2     | 2     | 197   | 182   | 2.1                               |
| 0     | 255   | 241   | 2     | 2     | 231   | 82    | 2.2                               |
| 0     | 255   | 241   | 2     | 3     | 10    | 43    | 2.3                               |
| 0     | 255   | 241   | 2     | 3     | 44    | 11    | 2.4                               |
| 0     | 255   | 241   | 2     | 3     | 78    | 210   | 2.5                               |
| 0     | 255   | 241   | 2     | 3     | 112   | 8     | 2.6                               |
| 0     | 255   | 241   | 2     | 3     | 147   | 154   | 2.7                               |
| 0     | 255   | 241   | 2     | 3     | 181   | 186   | 2.8                               |
| 0     | 255   | 241   | 2     | 3     | 215   | 99    | 2.9                               |
| 0     | 255   | 241   | 2     | 3     | 249   | 250   | 3                                 |
| 0     | 255   | 241   | 2     | 4     | 28    | 96    | 3.1                               |
| 0     | 255   | 241   | 2     | 4     | 62    | 132   | 3.2                               |
| 0     | 255   | 241   | 2     | 4     | 96    | 229   | 3.3                               |
| 0     | 255   | 241   | 2     | 4     | 130   | 70    | 3.4                               |
| 0     | 255   | 241   | 2     | 4     | 165   | 87    | 3.5                               |
| 0     | 255   | 241   | 2     | 4     | 199   | 142   | 3.6                               |
| 0     | 255   | 241   | 2     | 4     | 233   | 23    | 3.7                               |
| 0     | 255   | 241   | 2     | 5     | 11    | 64    | 3.8                               |
| 0     | 255   | 241   | 2     | 5     | 46    | 51    | 3.9                               |
| 0     | 255   | 241   | 2     | 5     | 80    | 212   | 4                                 |
| 0     | 255   | 241   | 2     | 5     | 114   | 48    | 4.1                               |
| 0     | 255   | 241   | 2     | 5     | 148   | 87    | 4.2                               |
| 0     | 255   | 241   | 2     | 5     | 183   | 130   | 4.3                               |
| 0     | 255   | 241   | 2     | 5     | 217   | 38    | 4.4                               |
| 0     | 255   | 241   | 2     | 5     | 251   | 194   | 4.5                               |
| 0     | 255   | 241   | 2     | 6     | 29    | 136   | 4.6                               |
| 0     | 255   | 241   | 2     | 6     | 64    | 186   | 4.7                               |
| 0     | 255   | 241   | 2     | 6     | 98    | 94    | 4.8                               |
| 0     | 255   | 241   | 2     | 6     | 132   | 57    | 4.9                               |
| 0     | 255   | 241   | 2     | 6     | 166   | 221   | 5                                 |
| 0     | 255   | 241   | 2     | 6     | 200   | 121   | 5.1                               |
| 0     | 255   | 241   | 2     | 6     | 235   | 172   | 5.2                               |
| 0     | 255   | 241   | 2     | 7     | 13    | 63    | 5.3                               |
| 0     | 255   | 241   | 2     | 7     | 47    | 219   | 5.4                               |
| 0     | 255   | 241   | 2     | 7     | 81    | 60    | 5.5                               |
| 0     | 255   | 241   | 2     | 7     | 116   | 79    | 5.6                               |
| 0     | 255   | 241   | 2     | 7     | 150   | 236   | 5.7                               |
| 0     | 255   | 241   | 2     | 7     | 184   | 117   | 5.8                               |
| 0     | 255   | 241   | 2     | 7     | 218   | 172   | 5.9                               |
| 0     | 255   | 241   | 2     | 7     | 253   | 189   | 6                                 |
| 0     | 255   | 241   | 2     | 8     | 31    | 135   | 6.1                               |
| 0     | 255   | 241   | 2     | 8     | 65    | 230   | 6.2                               |
| 0     | 255   | 241   | 2     | 8     | 99    | 2     | 6.3                               |
| 0     | 255   | 241   | 2     | 8     | 134   | 54    | 6.4                               |
| 0     | 255   | 241   | 2     | 8     | 168   | 175   | 6.5                               |
| 0     | 255   | 241   | 2     | 8     | 202   | 118   | 6.6                               |
| 0     | 255   | 241   | 2     | 8     | 236   | 86    | 6.7                               |
| 0     | 255   | 241   | 2     | 9     | 15    | 48    | 6.8                               |
| 0     | 255   | 241   | 2     | 9     | 49    | 234   | 6.9                               |
| 0     | 255   | 241   | 2     | 9     | 83    | 51    | 7                                 |
| 0     | 255   | 241   | 2     | 9     | 117   | 19    | 7.1                               |
| 0     | 255   | 241   | 2     | 9     | 152   | 158   | 7.2                               |
| 0     | 255   | 241   | 2     | 9     | 186   | 122   | 7.3                               |
| 0     | 255   | 241   | 2     | 9     | 220   | 103   | 7.4                               |
| 0     | 255   | 241   | 2     | 9     | 254   | 131   | 7.5                               |
| 0     | 255   | 241   | 2     | 10    | 33    | 132   | 7.6                               |
| 0     | 255   | 241   | 2     | 10    | 67    | 93    | 7.7                               |
| 0     | 255   | 241   | 2     | 10    | 101   | 125   | 7.8                               |
| 0     | 255   | 241   | 2     | 10    | 135   | 222   | 7.9                               |
| 0     | 255   | 241   | 2     | 10    | 170   | 20    | 8                                 |
| 0     | 255   | 241   | 2     | 10    | 204   | 9     | 8.1                               |
| 0     | 255   | 241   | 2     | 10    | 238   | 237   | 8.2                               |
| 0     | 255   | 241   | 2     | 11    | 16    | 132   | 8.3                               |
| 0     | 255   | 241   | 2     | 11    | 51    | 81    | 8.4                               |
| 0     | 255   | 241   | 2     | 11    | 85    | 76    | 8.5                               |
| 0     | 255   | 241   | 2     | 11    | 119   | 168   | 8.6                               |
| 0     | 255   | 241   | 2     | 11    | 153   | 118   | 8.7                               |
| 0     | 255   | 241   | 2     | 11    | 188   | 5     | 8.8                               |
| 0     | 255   | 241   | 2     | 11    | 222   | 220   | 8.9                               |
| 0     | 255   | 241   | 2     | 12    | 0     | 105   | 9                                 |
| 0     | 255   | 241   | 2     | 12    | 34    | 141   | 9.1                               |
| 0     | 255   | 241   | 2     | 12    | 69    | 161   | 9.2                               |
| 0     | 255   | 241   | 2     | 12    | 103   | 69    | 9.3                               |
| 0     | 255   | 241   | 2     | 12    | 137   | 155   | 9.4                               |
| 0     | 255   | 241   | 2     | 12    | 171   | 127   | 9.5                               |
| 0     | 255   | 241   | 2     | 12    | 206   | 49    | 9.6                               |
| 0     | 255   | 241   | 2     | 12    | 240   | 235   | 9.7                               |
| 0     | 255   | 241   | 2     | 13    | 18    | 188   | 9.8                               |
| 0     | 255   | 241   | 2     | 13    | 52    | 156   | 9.9                               |
| 0     | 255   | 241   | 2     | 13    | 87    | 116   | 10                                |
| 0     | 255   | 241   | 2     | 13    | 121   | 237   | 10.1                              |
| 0     | 255   | 241   | 2     | 13    | 155   | 78    | 10.2                              |
| 0     | 255   | 241   | 2     | 13    | 189   | 110   | 10.3                              |
| 0     | 255   | 241   | 2     | 13    | 224   | 92    | 10.4                              |
| 0     | 255   | 241   | 2     | 14    | 2     | 210   | 10.5                              |
| 0     | 255   | 241   | 2     | 14    | 36    | 242   | 10.6                              |
| 0     | 255   | 241   | 2     | 14    | 70    | 43    | 10.7                              |
| 0     | 255   | 241   | 2     | 14    | 105   | 131   | 10.8                              |
| 0     | 255   | 241   | 2     | 14    | 139   | 32    | 10.9                              |
| 0     | 255   | 241   | 2     | 14    | 173   | 0     | 11                                |
| 0     | 255   | 241   | 2     | 14    | 207   | 217   | 11.1                              |
| 0     | 255   | 241   | 2     | 14    | 242   | 80    | 11.2                              |
| 0     | 255   | 241   | 2     | 15    | 20    | 195   | 11.3                              |
| 0     | 255   | 241   | 2     | 15    | 54    | 39    | 11.4                              |
| 0     | 255   | 241   | 2     | 15    | 88    | 131   | 11.5                              |
| 0     | 255   | 241   | 2     | 15    | 123   | 86    | 11.6                              |
| 0     | 255   | 241   | 2     | 15    | 157   | 49    | 11.7                              |
| 0     | 255   | 241   | 2     | 15    | 191   | 213   | 11.8                              |
| 0     | 255   | 241   | 2     | 15    | 225   | 180   | 11.9                              |
| 0     | 255   | 241   | 2     | 16    | 4     | 119   | 12                                |

# Incline Table
| byte1 | byte2 | byte3 | byte4 | byte5 | byte6 | byte7 | Observed Incline when Sent (%) |
|-------|-------|-------|-------|-------|-------|-------|--------------------------------|
| 0     | 255   | 246   | 2     | 0     | 40    | 112   | 0                              |
| 0     | 255   | 246   | 2     | 0     | 80    | 49    | 0.5                            |
| 0     | 255   | 246   | 2     | 0     | 120   | 14    | 1                              |
| 0     | 255   | 246   | 2     | 0     | 160   | 179   | 1.5                            |
| 0     | 255   | 246   | 2     | 0     | 200   | 177   | 2                              |
| 0     | 255   | 246   | 2     | 0     | 240   | 205   | 2.5                            |
| 0     | 255   | 246   | 2     | 1     | 24    | 65    | 3                              |
| 0     | 255   | 246   | 2     | 1     | 64    | 134   | 3.5                            |
| 0     | 255   | 246   | 2     | 1     | 104   | 185   | 4                              |
| 0     | 255   | 246   | 2     | 1     | 144   | 130   | 4.5                            |
| 0     | 255   | 246   | 2     | 1     | 184   | 189   | 5                              |
| 0     | 255   | 246   | 2     | 1     | 224   | 122   | 5.5                            |
| 0     | 255   | 246   | 2     | 2     | 8     | 47    | 6                              |
| 0     | 255   | 246   | 2     | 2     | 48    | 83    | 6.5                            |
| 0     | 255   | 246   | 2     | 2     | 88    | 81    | 7                              |
| 0     | 255   | 246   | 2     | 2     | 128   | 236   | 7.5                            |
| 0     | 255   | 246   | 2     | 2     | 168   | 211   | 8                              |
| 0     | 255   | 246   | 2     | 2     | 208   | 146   | 8.5                            |
| 0     | 255   | 246   | 2     | 2     | 248   | 173   | 9                              |
| 0     | 255   | 246   | 2     | 3     | 32    | 228   | 9.5                            |
| 0     | 255   | 246   | 2     | 3     | 72    | 230   | 10                             |
| 0     | 255   | 246   | 2     | 3     | 112   | 154   | 10.5                           |
| 0     | 255   | 246   | 2     | 3     | 152   | 226   | 11                             |
| 0     | 255   | 246   | 2     | 3     | 192   | 37    | 11.5                           |
| 0     | 255   | 246   | 2     | 3     | 232   | 26    | 12                             |

# Stop Command
| byte1 | byte2 | byte3 | byte4 | byte5 | byte6 |
|-------|-------|-------|-------|-------|-------|
| 0     | 255   | 245   | 1     | 0     | 182   |

# Unknown Command (Possibly a keep-alive heartbeat)
| byte1 | byte2 | byte3 | byte4 | byte5 |
|-------|-------|-------|-------|-------|
| 0     | 255   | 249   | 0     | 144   |
