# netstack

## Introdution

use cpp new features to write a light tcp/ip stack

## 命令

``` shell
sudo ip link add new_eth0 type veth peer name new_eth1
sudo ip link set new_eth0 address f6:34:95:26:90:66
sudo ip link set new_eth0 up
sudo brctl addif docker0 new_eth1
```

## 问题归总

1. 为什么采用macvlan, 在路由器端接收到的arp请求的mac, 不是macvlan的mac地址,  
而是宿主的mac地址?  

2. 为什么macvlan自定义实现的arp请求, 只能在本机接收到arp相应, 而在外部路由器收不到?  

3. 在raw socket发送链路层的数据流时, 是否需要计算crc?  

4. cpp 可变长结构体, 需要重载new方法

5. 在raw socket中读取的字节数, 无论如何都是60个字节, 如果接收的是arp请求, 那么实际可能是42字节,  
其原因在于以太网的最小帧是64字节, 使用raw socket接收时, 移除了CRC的4个字节
