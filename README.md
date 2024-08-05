# netstack

## Introdution

use cpp new features to write a light tcp/ip stack

## 命令

``` shell
sudo ip link add new_eth0 type veth peer name new_eth1
sudo ip link set new_eth0 address f6:34:95:26:90:66
sudo ip link set new_eth0 up
sudo ip link set new_eth1 up
sudo brctl addif docker0 new_eth1
```

## 特性

- [x] arp
- [x] ip frag/defrag
- [x] icmp
- [x] udp
- [x] tcp
- [ ] tcp flow control
- [ ] tcp option
- [ ] tcp retransmission
- [ ] netfilter
- [ ] dhcp
- [ ] dns
- [ ] tls

### arp

``` log
tcpdump: listening on new_eth0, link-type EN10MB (Ethernet), snapshot length 262144 bytes
23:03:10.071296 ARP, Ethernet (len 6), IPv4 (len 4), Request who-has 172.17.0.253 (ff:ff:ff:ff:ff:ff) tell 172.17.0.2, length 28
23:03:10.071504 ARP, Ethernet (len 6), IPv4 (len 4), Reply 172.17.0.253 is-at f6:34:95:26:90:66, length 28
23:03:11.071448 ARP, Ethernet (len 6), IPv4 (len 4), Request who-has 172.17.0.253 (f6:34:95:26:90:66) tell 172.17.0.2, length 28
23:03:11.071525 ARP, Ethernet (len 6), IPv4 (len 4), Reply 172.17.0.253 is-at f6:34:95:26:90:66, length 28
23:03:12.071281 ARP, Ethernet (len 6), IPv4 (len 4), Request who-has 172.17.0.253 (f6:34:95:26:90:66) tell 172.17.0.2, length 28
23:03:12.071483 ARP, Ethernet (len 6), IPv4 (len 4), Reply 172.17.0.253 is-at f6:34:95:26:90:66, length 28
23:03:13.071278 ARP, Ethernet (len 6), IPv4 (len 4), Request who-has 172.17.0.253 (f6:34:95:26:90:66) tell 172.17.0.2, length 28
```

### icmp

``` log
tcpdump: listening on new_eth0, link-type EN10MB (Ethernet), snapshot length 262144 bytes
23:04:43.111651 IP (tos 0x0, ttl 64, id 12504, offset 0, flags [DF], proto ICMP (1), length 84)
    172.17.0.2 > 172.17.0.253: ICMP echo request, id 1, seq 11, length 64
23:04:43.111888 IP (tos 0x0, ttl 64, id 1, offset 0, flags [none], proto ICMP (1), length 84)
    172.17.0.253 > 172.17.0.2: ICMP echo reply, id 1, seq 11, length 64
23:04:44.124978 IP (tos 0x0, ttl 64, id 12766, offset 0, flags [DF], proto ICMP (1), length 84)
    172.17.0.2 > 172.17.0.253: ICMP echo request, id 1, seq 12, length 64
23:04:44.125290 IP (tos 0x0, ttl 64, id 1, offset 0, flags [none], proto ICMP (1), length 84)
    172.17.0.253 > 172.17.0.2: ICMP echo reply, id 1, seq 12, length 64
23:04:45.138320 IP (tos 0x0, ttl 64, id 13008, offset 0, flags [DF], proto ICMP (1), length 84)
    172.17.0.2 > 172.17.0.253: ICMP echo request, id 1, seq 13, length 64
23:04:45.138539 IP (tos 0x0, ttl 64, id 1, offset 0, flags [none], proto ICMP (1), length 84)
    172.17.0.253 > 172.17.0.2: ICMP echo reply, id 1, seq 13, length 64
```

### udp echo server

``` log
23:07:45.995799 IP (tos 0x0, ttl 64, id 30599, offset 0, flags [DF], proto UDP (17), length 34)
    172.17.0.2.60756 > 172.17.0.253.8888: UDP, length 6
23:07:45.995903 IP (tos 0x0, ttl 64, id 1, offset 0, flags [none], proto UDP (17), length 34)
    172.17.0.253.8888 > 172.17.0.2.60756: UDP, length 6
23:07:47.897266 IP (tos 0x0, ttl 64, id 30600, offset 0, flags [DF], proto UDP (17), length 34)
    172.17.0.2.60756 > 172.17.0.253.8888: UDP, length 6
23:07:47.897363 IP (tos 0x0, ttl 64, id 1, offset 0, flags [none], proto UDP (17), length 34)
    172.17.0.253.8888 > 172.17.0.2.60756: UDP, length 6
23:07:49.774089 IP (tos 0x0, ttl 64, id 30601, offset 0, flags [DF], proto UDP (17), length 34)
    172.17.0.2.60756 > 172.17.0.253.8888: UDP, length 6
23:07:49.774194 IP (tos 0x0, ttl 64, id 1, offset 0, flags [none], proto UDP (17), length 34)
    172.17.0.253.8888 > 172.17.0.2.60756: UDP, length 6
23:07:52.577003 IP (tos 0x0, ttl 64, id 30602, offset 0, flags [DF], proto UDP (17), length 34)
    172.17.0.2.60756 > 172.17.0.253.8888: UDP, length 6
23:07:52.577099 IP (tos 0x0, ttl 64, id 1, offset 0, flags [none], proto UDP (17), length 34)
    172.17.0.253.8888 > 172.17.0.2.60756: UDP, length 6
```

### tcp client

``` log
# netcat服务器
root@efede262645e:/# nc -l 8888
hello!hello!hello!

# 抓包
23:11:10.411813 IP (tos 0x0, ttl 64, id 1, offset 0, flags [none], proto TCP (6), length 40)
    172.17.0.253.52168 > 172.17.0.2.8888: Flags [S], cksum 0x6798 (correct), seq 1483843638, win 65535, length 0
23:11:10.411843 IP (tos 0x0, ttl 64, id 0, offset 0, flags [DF], proto TCP (6), length 44)
    172.17.0.2.8888 > 172.17.0.253.52168: Flags [S.], cksum 0x5940 (incorrect -> 0xaff6), seq 1377587911, ack 1483843639, win 64240, options [mss 1460], length 0
23:11:10.412080 IP (tos 0x0, ttl 64, id 1, offset 0, flags [none], proto TCP (6), length 40)
    172.17.0.253.52168 > 172.17.0.2.8888: Flags [.], cksum 0xc2a4 (correct), ack 1, win 65535, length 0
23:11:10.412102 IP (tos 0x0, ttl 64, id 1, offset 0, flags [none], proto TCP (6), length 46)
    172.17.0.253.52168 > 172.17.0.2.8888: Flags [.], cksum 0x7eab (correct), seq 1:7, ack 1, win 65535, length 6
23:11:10.412114 IP (tos 0x0, ttl 64, id 35455, offset 0, flags [DF], proto TCP (6), length 40)
    172.17.0.2.8888 > 172.17.0.253.52168: Flags [.], cksum 0x593c (incorrect -> 0xc7b3), ack 7, win 64234, length 0
23:11:15.412128 IP (tos 0x0, ttl 64, id 1, offset 0, flags [none], proto TCP (6), length 46)
    172.17.0.253.52168 > 172.17.0.2.8888: Flags [.], cksum 0x7ea5 (correct), seq 7:13, ack 1, win 65535, length 6
23:11:15.412147 IP (tos 0x0, ttl 64, id 35456, offset 0, flags [DF], proto TCP (6), length 40)
    172.17.0.2.8888 > 172.17.0.253.52168: Flags [.], cksum 0x593c (incorrect -> 0xc7b3), ack 13, win 64228, length 0
```

### tcp echo server

``` log
# 发送echo, 接收echo
root@efede262645e:/# nc 172.17.0.253 8888
echo!
echo!

# 抓包
tcpdump: listening on new_eth0, link-type EN10MB (Ethernet), snapshot length 262144 bytes
23:17:16.640054 IP (tos 0x0, ttl 64, id 77, offset 0, flags [DF], proto TCP (6), length 60)
    172.17.0.2.36644 > 172.17.0.253.8888: Flags [S], cksum 0x5950 (incorrect -> 0x9ffb), seq 794448035, win 64240, options [mss 1460,sackOK,TS val 750056803 ecr 0,nop,wscale 7], length 0
23:17:16.640138 IP (tos 0x0, ttl 64, id 1, offset 0, flags [none], proto TCP (6), length 40)
    172.17.0.253.8888 > 172.17.0.2.36644: Flags [S.], cksum 0x24d6 (correct), seq 0, ack 794448036, win 65535, length 0
23:17:16.640159 IP (tos 0x0, ttl 64, id 78, offset 0, flags [DF], proto TCP (6), length 40)
    172.17.0.2.36644 > 172.17.0.253.8888: Flags [.], cksum 0x593c (incorrect -> 0x29e6), ack 1, win 64240, length 0
23:17:19.365916 IP (tos 0x0, ttl 64, id 79, offset 0, flags [DF], proto TCP (6), length 46)
    172.17.0.2.36644 > 172.17.0.253.8888: Flags [P.], cksum 0x5942 (incorrect -> 0x3afb), seq 1:7, ack 1, win 64240, length 6
23:17:19.365980 IP (tos 0x0, ttl 64, id 1, offset 0, flags [none], proto TCP (6), length 40)
    172.17.0.253.8888 > 172.17.0.2.36644: Flags [.], cksum 0x24d1 (correct), ack 7, win 65535, length 0
23:17:19.366010 IP (tos 0x0, ttl 64, id 1, offset 0, flags [none], proto TCP (6), length 46)
    172.17.0.253.8888 > 172.17.0.2.36644: Flags [.], cksum 0x35ee (correct), seq 1:7, ack 7, win 65535, length 6
23:17:19.366024 IP (tos 0x0, ttl 64, id 80, offset 0, flags [DF], proto TCP (6), length 40)
    172.17.0.2.36644 > 172.17.0.253.8888: Flags [.], cksum 0x593c (incorrect -> 0x29e0), ack 7, win 64234, length 0
```

## 问题归总

1. 为什么采用macvlan, 在路由器端接收到的arp请求的mac, 不是macvlan的mac地址,  
而是宿主的mac地址?  

2. 为什么macvlan自定义实现的arp请求, 只能在本机接收到arp相应, 而在外部路由器收不到?  

3. 在raw socket发送链路层的数据流时, 是否需要计算crc?  

4. cpp 可变长结构体, 需要重载new方法

5. 在raw socket中读取的字节数, 无论如何都是60个字节, 如果接收的是arp请求, 那么实际可能是42字节,  
其原因在于以太网的最小帧是64字节, 使用raw socket接收时, 移除了CRC的4个字节

6. tcp作为客户端请求时, 应该注意避免sequence number重复, 或者为0, 否则会被作为重放攻击丢弃请求