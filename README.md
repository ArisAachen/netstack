# netstack

## Introdution

use cpp new features to write a light tcp/ip stack

## 命令

``` shell
sudo ip link add link wlp2s0f0u5 name new_eth0 type macvlan mode bridge
sudo ip link set new_eth0 address f6:34:95:26:90:66
sudo ip link set new_eth0 up
sudo ip link add link wlp2s0f0u5 name new_eth1 type macvlan mode bridge
sudo ip addr add dev new_eth1 192.168.121.252/24
sudo ip link set new_eth1 up
```

## 问题归总

1. 为什么采用macvlan, 在路由器端接收到的arp请求的mac, 不是macvlan的mac地址,  
而是宿主的mac地址?  

2. 为什么macvlan自定义实现的arp请求, 只能在本机接收到arp相应, 而在外部路由器收不到?  

3. 在raw socket发送链路层的数据流时, 是否需要计算crc?  
