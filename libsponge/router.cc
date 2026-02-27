#include "router.hh"
 
#include <iostream>
 
using namespace std;
 
// IP路由器的简单实现
 
// 给定一个传入的互联网数据报，路由器决定
// (1) 从哪个接口发送出去
// (2) 发送到哪个下一跳地址
 
// 对于实验6，请替换为能通过`make check_lab6`自动化检查的真实实现
 
// 你需要在`router.hh`的类声明中添加私有成员
 
template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}
 
//! \param[in] route_prefix 要与数据报目的地址匹配的"最长32位"IPv4地址前缀
//! \param[in] prefix_length 要使此路由生效，route_prefix的高位（最高有效位）中需要有多少位与数据报目的地址的相应位匹配？
//! \param[in] next_hop 下一跳的IP地址。如果网络直接连接到路由器（此时下一跳地址应为数据报的最终目的地），则该参数为空
//! \param[in] interface_num 发送数据报的接口索引
void Router::add_route(const uint32_t route_prefix,
                       const uint8_t prefix_length,
                       const optional<Address> next_hop,
                       const size_t interface_num) {
    cerr << "DEBUG: 添加路由 " << Address::from_ipv4_numeric(route_prefix).ip() << "/" << int(prefix_length)
         << " => " << (next_hop.has_value() ? next_hop->ip() : "(直接连接)") << " 在接口 " << interface_num << "\n";
 
    // 在此处编写你的代码
    _route_table.emplace_back(route_prefix, prefix_length, next_hop, interface_num);
}
 
//! \param[in] dgram 要路由的数据报
void Router::route_one_datagram(InternetDatagram &dgram) {
    // 在此处编写你的代码
    // 取出IP字段，在路由表中进行最长前缀匹配
    auto ip = dgram.header().dst;
    auto best_match_it = _route_table.end();
    for (auto it = _route_table.begin(); it != _route_table.end(); it = std::next(it)) {
        // 前缀匹配成功
        //! 注意：根据CSAPP，在32位整数下，右移量只取低5位（即mod 32），因此右移32位等价于右移0位，因此这里需要特判右移32位的情况 
        // cerr << "DEBUG 路由表: " << ((it->route_prefix ^ ip) >> (32 - it->prefix_length)) << '\n';
        if (it->prefix_length == 0 ||  (it->route_prefix ^ ip) >> (32 - it->prefix_length) == 0) {
            if (best_match_it == _route_table.end() || it->prefix_length > best_match_it->prefix_length) {
                best_match_it = it;
            }
        }
    }
    // 匹配到路由规则并且TTL大于1
    if (best_match_it != _route_table.end() && dgram.header().ttl > 1) {
        --dgram.header().ttl;
        auto &next_interface = interface(best_match_it->interface_num);
        // 如果路由器直接连接到相关网络，则下一跳就是目的IP地址，否则为下一跳路由器的IP地址
        if (best_match_it->next_hop.has_value()) {
            next_interface.send_datagram(dgram, best_match_it->next_hop.value());
        } else {
            next_interface.send_datagram(dgram, Address::from_ipv4_numeric(ip));
        }
    }
    // 其余情况数据报则直接丢弃，也不作ICMP回复
}
 
void Router::route() {
    // 遍历所有接口，将每个传入的数据报路由到其正确的传出接口
    for (auto &interface : _interfaces) {
        auto &queue = interface.datagrams_out();
        while (not queue.empty()) {
            route_one_datagram(queue.front());
            queue.pop();
        }
    }
}