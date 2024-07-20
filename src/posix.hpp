#ifndef __POSIX_H__
#define __POSIX_H__

#include <cstddef>
#include <cstdint>

#include <sys/socket.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief create sock fd
* @param[in] domain sock domain
* @param[in] type sock type
* @param[in] protocol sock protocol
* @return sock fd
*/
int sock_create(int domain, int type, int protocol);

/**
* @brief connect sock fd
* @param[in] fd sock fd,
* @param[in] addr remote addr,
* @param[in] len addr len,
* @return sock fd
*/
int stack_connect(uint32_t fd, struct sockaddr* addr, socklen_t len);

/**
* @brief close sock fd
* @param[in] fd sock fd,
* @return sock fd
*/
int stack_close(uint32_t fd);

/**
* @brief bind sock fd
* @param[in] fd sock fd,
* @param[in] addr remote addr,
* @param[in] len addr len,
* @return sock fd
*/
int stack_bind(uint32_t fd, struct sockaddr* addr, socklen_t len);

/**
* @brief write buf to stack
* @param[in] fd sock fd
* @param[in] buf buffer
* @param[in] size buf len
* @return write size
*/
size_t stack_write(uint32_t fd, char* buf, size_t size);

/**
* @brief read buf from stack
* @param[in] fd sock fd
* @param[in] buf buffer
* @param[in] size buf len
* @return read size
*/
size_t stack_read(uint32_t fd, char* buf, size_t size);

/**
* @brief read buf from stack
* @param[in] fd sock fd
* @param[in] buf buffer
* @param[in] size buf len
* @param[in] addr recv addr
* @param[in] len addr len
* @return read size
*/
size_t stack_readfrom(uint32_t fd, char* buf, size_t size, struct sockaddr* addr, socklen_t* len);

/**
* @brief write buf to stack
* @param[in] fd sock fd
* @param[in] buf buffer
* @param[in] size buf len
* @param[in] addr recv addr
* @param[in] len addr len
* @return read size
*/
size_t stack_writeto(uint32_t fd, char* buf, size_t size, struct sockaddr* addr, socklen_t len);

#ifdef __cplusplus
}
#endif


#endif // __POSIX_H__