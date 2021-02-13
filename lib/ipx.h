/**
 * @file ipx.c
 * @author SuperIlu (superilu@yahoo.com)
 * @brief simple IPX functions
 *
 * @copyright Copyright (C) 2001 Hotwarez LLC, Goldtree Enterprises
 * @copyright SuperIlu
 *
 * This code was developed with the help of:
 * https://github.com/hyperlogic/cylindrix/blob/master/src/legacy/jonipx.c
 */
#ifndef __IPX_H_
#define __IPX_H_

#include <stdbool.h>
#include <stdint.h>

/* ======================================================================
** defines
** ====================================================================== */
#define IPX_MAX_PACKET_LEN 80    //!< max length of a packet
#define IPX_BUFFER_SIZE 50       //!< max number of packets in ringbufer
#define IPX_NETWORK_ADDR_SIZE 4  //!< size of a network address
#define IPX_NODE_ADDR_SIZE 6     //!< size of a node address

#define IPX_DYNAMIC_SOCKET 0x0000  //!< dynamically allocate the socket number

/* ======================================================================
** typedefs
** ====================================================================== */
#pragma pack(__push, 1)  // make sure no padding is used

typedef uint8_t ipx_net_t[IPX_NETWORK_ADDR_SIZE];  //!< network address
typedef uint8_t ipx_node_t[IPX_NODE_ADDR_SIZE];    //!< node address
typedef uint8_t ipx_data_t[IPX_MAX_PACKET_LEN];    //!< IPX packet data

//! a DOS far pointer to data or code
typedef struct __ipx_far_ptr_t {
    uint16_t offset;   //!< data offset
    uint16_t segment;  //!< data segment
} far_ptr_t;

//! IPX Event Control Block (http://www.ctyme.com/intr/rb-7845.htm)
typedef struct __ipx_ecb {
    uint32_t link;
    far_ptr_t service_routine;
    uint8_t in_use;
    uint8_t complete;
    uint16_t socket;
    uint8_t ipx_workspace[4];
    uint8_t driver_workspace[12];
    ipx_node_t immediate_addr;
    uint16_t fragment_count;
    far_ptr_t fragment_data;
    uint16_t fragment_size;
} ipx_ecb_t;

//! complete IPX address
typedef struct __ipx_net_addr {
    ipx_net_t network;  //!< network address
    ipx_node_t node;    //!< node address
    uint16_t socket;    //!< socket number
} ipx_net_addr_t;

//! packet header (http://www.ctyme.com/intr/rb-7845.htm#Table3815)
typedef struct __ipx_header {
    uint16_t checksum;
    uint16_t length;
    uint8_t transport_control;
    uint8_t packet_type;
    ipx_net_addr_t destination;
    ipx_net_addr_t source;
} ipx_header_t;

//! ECB, header and data used either for sending or receiving
typedef struct __ipx_direction {
    ipx_ecb_t ecb;        //!< ECB
    ipx_header_t header;  //!< packet header
    ipx_data_t data;      //!< packet data
} ipx_api_t;

//! a received packet with its source address
typedef struct __ipx_received {
    ipx_node_t source;  //!< source address
    ipx_data_t data;    //!< packet data
} ipx_received_t;

//!< simple ringbuffer for received packets
typedef struct __ipx_ringbuffer {
    uint16_t start;                           //!< start pointer
    uint16_t end;                             //!< end/current pointer
    ipx_received_t packets[IPX_BUFFER_SIZE];  //!< buffer data
} ipx_ringbuffer_t;

//!< per socket ipx struct
typedef struct __ipx {
    ipx_api_t send;           //!< all IPX structs for sending data
    ipx_api_t receive;        //!< all IPX structs for receiving data
    ipx_ringbuffer_t buffer;  //!< ringbuffer for received packets
} ipx_t;

#pragma pack(__pop)  // end pack pragms

/* ======================================================================
** global variables
** ====================================================================== */
//! broadcast address
extern const ipx_node_t IPX_BROADCAST_ADDR;

/* ======================================================================
** prototypes
** ====================================================================== */
extern bool ipx_init();
extern void ipx_get_local_address(ipx_net_t net, ipx_node_t node);
extern void ipx_print_address(ipx_net_t *net, const ipx_node_t *node);
extern bool ipx_open_socket(uint16_t *number);
extern void ipx_close_socket();
extern void ipx_send_packet(const ipx_data_t *data, const ipx_node_t *node);
extern bool ipx_check_packet();
extern bool ipx_get_packet(ipx_received_t *rec);

#endif  // __IPX_H_
