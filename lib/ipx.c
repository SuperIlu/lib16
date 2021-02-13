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
#include <stddef.h>
#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <mem.h>

#include "ipx.h"
#include "error.h"

/* ======================================================================
** defines
** ====================================================================== */
#define INT_INSTALLED_STATUS 0x2F      //!< check service INT
#define INT_INSTALLED_STATUS_IPX 0x7A  //!< check for IPX FUNC

#define INT_NOVELL 0x7A                    //!< Novell IPX driver INT
#define INT_NOVELL_SOCKET_OPEN 0x00        //!< open IPX socket FUNC
#define INT_NOVELL_SOCKET_CLOSE 0x01       //!< close IPX socket FUNC
#define INT_NOVELL_SEND_PACKET 0x03        //!< send packet FUNC
#define INT_NOVELL_LISTEN_FOR_PACKET 0x04  //!< listen for packet FUNC
#define INT_NOVELL_GET_ADDRESS 0x09        //!< get local address FUNC
#define INT_NOVELL_IDLE 0x0A               //!< user code idle FUNC

#define IPX_PACKET_EXCHANGE_TYPE 0x00  //!< use packet type

//! swap a uint16_t
#define IPX_SWAP16(x) (((((uint32_t)x) & 0xFF) << 8) | ((((uint32_t)x) & 0xFF00) >> 8))

//! copy a network address
#define IPX_COPY_NET(dst, src) memcpy(dst, src, sizeof(ipx_net_t))

//! copy a node address
#define IPX_COPY_NODE(dst, src) memcpy(dst, src, sizeof(ipx_node_t))

/* ======================================================================
** global variables
** ====================================================================== */

//! broadcast address
const ipx_node_t IPX_BROADCAST_ADDR = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/* ======================================================================
** local variables
** ====================================================================== */
static ipx_net_t ipx_local_net;
static ipx_node_t ipx_local_node;
static ipx_t ipx;
static bool ipx_is_open;
static uint16_t ipx_socket;

/* ======================================================================
** private functions
** ====================================================================== */
static void ipx_receive_callback();

/**
 * This call indicates that the application is idle and permits the IPX driver to do some work
 */
static void ipx_idle() {
    union REGS regs;
    bzero(&regs, sizeof(regs));

    regs.x.bx = INT_NOVELL_IDLE;
    int86(INT_NOVELL, &regs, &regs);
}

/**
 * @brief pass the ECB for receiving packets to the driver.
 *
 * @return true if the driver accepted the ECB, false for error.
 */
static bool ipx_listen_for_packet() {
    union REGS regs;
    struct SREGS sregs;
    bzero(&regs, sizeof(regs));
    bzero(&sregs, sizeof(sregs));

    regs.x.si = FP_OFF((char far *)&ipx.receive.ecb);  // ecb address
    sregs.es = FP_SEG((char far *)&ipx.receive.ecb);

    regs.x.bx = INT_NOVELL_LISTEN_FOR_PACKET;
    int86x(INT_NOVELL, &regs, &regs, &sregs);

    return regs.h.al == 0x00;
}

/**
 * @brief compare node adrresses.
 *
 * @param n1 address 1
 * @param n2 address 2
 * @return true if the addresses are identical, else false.
 */
static bool ipx_compare_nodes(ipx_node_t n1, ipx_node_t n2) {
    int i;
    for (i = 0; i < sizeof(ipx_node_t); i++) {
        if (n1[i] != n2[i]) {
            return false;
        }
    }
    return true;
}

/**
 * @brief callback function for received packets. this is called by the IPX driver.
 */
static void ipx_receive_callback() {
    ipx_listen_for_packet();

    // only copy packet if it is not from outselves
    if (!ipx_compare_nodes(ipx_local_node, ipx.receive.header.source.node)) {
        // copy address and data to the buffer
        IPX_COPY_NODE(ipx.buffer.packets[ipx.buffer.end].source, ipx.receive.header.source.node);
        memcpy(ipx.buffer.packets[ipx.buffer.end].data, ipx.receive.data, sizeof(ipx_data_t));

        // increment ringbuffer
        ipx.buffer.end++;
        if (ipx.buffer.end >= IPX_BUFFER_SIZE) {
            ipx.buffer.end = 0;
        }
    }
}

/* ======================================================================
** public functions
** ====================================================================== */
/**
 * @brief detect/initialize IPX subsystem.
 *
 * @return true if an IPX driver was found, else false.
 */
bool ipx_init() {
    union REGS regs;
    bzero(&regs, sizeof(regs));

    regs.h.ah = INT_INSTALLED_STATUS_IPX;
    regs.h.al = 0x00;
    int86(INT_INSTALLED_STATUS, &regs, &regs);

    ipx_is_open = false;

    // check if driver is installed
    if (regs.h.al != 0xFF) {
        ERR_DRIVR();
        return false;
    }

    ipx_get_local_address(&ipx_local_net, &ipx_local_node);
    ERR_OK();
    return true;
}

/**
 * @brief get the network/address of this node.
 *
 * @param net pointer to an ipx_net_t to fill.
 * @param node pointer to an ipx_node_t to fill.
 */
void ipx_get_local_address(ipx_net_t net, ipx_node_t node) {
    uint8_t buff[IPX_NETWORK_ADDR_SIZE + IPX_NODE_ADDR_SIZE];
    union REGS regs;
    struct SREGS sregs;
    bzero(&regs, sizeof(regs));
    bzero(&sregs, sizeof(sregs));

    bzero(buff, sizeof(buff));

    regs.x.si = FP_OFF((char far *)buff);  // buffer for result
    sregs.es = FP_SEG((char far *)buff);

    regs.x.bx = INT_NOVELL_GET_ADDRESS;
    int86x(INT_NOVELL, &regs, &regs, &sregs);

    memcpy(net, buff, sizeof(ipx_net_t));
    memcpy(node, &buff[IPX_NETWORK_ADDR_SIZE], sizeof(ipx_node_t));
}

/**
 * @brief print network/node address to stdout.
 *
 * @param net ipx_net_t or NULL.
 * @param node ipx_node_t or NULL.
 */
void ipx_print_address(ipx_net_t net, const ipx_node_t node) {
    if (net) {
        printf("net  %02X:%02X:%02X:%02X\n", net[0], net[1], net[2], net[3]);
    }
    if (node) {
        printf("node %02X:%02X:%02X:%02X:%02X:%02X\n", node[0], node[1], node[2], node[3], node[4], node[5]);
    }
}

/**
 * @brief open an IPX socket for sending/receiving.
 * NOTE: This code supports only opening a single socket!
 *
 * @param number pointer to socket number or 0x0000 for dynamic allocation. The actually used socket number will be returned here as well.
 * @return true if the socket could be opened, else false.
 */
bool ipx_open_socket(uint16_t *number) {
    union REGS regs;
    bzero(&regs, sizeof(regs));

    if (!ipx_is_open) {
        regs.h.dl = (*number >> 8) & 0xFF;  // socket number, high in DL
        regs.h.dh = *number & 0xFF;

        regs.x.bx = INT_NOVELL_SOCKET_OPEN;
        int86(INT_NOVELL, &regs, &regs);

        if (regs.h.al == 0x00) {
            ipx_socket = (regs.h.dl << 8) | regs.h.dh;

            // copy socket number if dynamic allocation was selected
            if (*number == 0x00) {
                *number = ipx_socket;
            }

            // clear struct
            bzero(&ipx, sizeof(ipx_t));

            //////
            // init send ecb
            ipx.send.ecb.socket = IPX_SWAP16(ipx_socket);
            IPX_COPY_NODE(ipx.send.ecb.immediate_addr, IPX_BROADCAST_ADDR);
            ipx.send.ecb.fragment_count = 1;
            ipx.send.ecb.fragment_size = sizeof(ipx_header_t) + sizeof(ipx_data_t);
            ipx.send.ecb.fragment_data.offset = FP_OFF((char far *)&ipx.send.header);
            ipx.send.ecb.fragment_data.segment = FP_SEG((char far *)&ipx.send.header);

            ipx.send.header.checksum = 0xFFFF;
            ipx.send.header.packet_type = IPX_PACKET_EXCHANGE_TYPE;

            // set header source
            IPX_COPY_NET(ipx.send.header.source.network, ipx_local_net);
            IPX_COPY_NODE(ipx.send.header.source.node, ipx_local_node);
            ipx.send.header.source.socket = IPX_SWAP16(ipx_socket);

            // set header dest
            IPX_COPY_NET(ipx.send.header.destination.network, ipx_local_net);
            IPX_COPY_NODE(ipx.send.header.destination.node, IPX_BROADCAST_ADDR);
            ipx.send.header.destination.socket = IPX_SWAP16(ipx_socket);

            //////
            // init receive ecb
            ipx.receive.ecb.in_use = 0x1D;
            ipx.receive.ecb.socket = IPX_SWAP16(ipx_socket);
            ipx.receive.ecb.fragment_count = 1;
            ipx.receive.ecb.fragment_size = sizeof(ipx_header_t) + sizeof(ipx_data_t);
            ipx.receive.ecb.fragment_data.segment = FP_SEG((char far *)&ipx.receive.header);
            ipx.receive.ecb.fragment_data.offset = FP_OFF((char far *)&ipx.receive.header);

            ipx.receive.ecb.service_routine.segment = FP_SEG((char far *)ipx_receive_callback);
            ipx.receive.ecb.service_routine.offset = FP_OFF((char far *)ipx_receive_callback);

            if (ipx_listen_for_packet()) {
                ipx_is_open = true;
                ERR_OK();
                return true;
            } else {
                ipx_close_socket();
            }
        }
    }
    ERR_AVAIL();
    return false;
}

/**
 * @brief close the socket.
 */
void ipx_close_socket() {
    union REGS regs;
    bzero(&regs, sizeof(regs));

    if (ipx_is_open) {
        regs.h.dl = (ipx_socket >> 8) & 0xFF;  // socket number, high in DL
        regs.h.dh = ipx_socket & 0xFF;

        regs.x.bx = INT_NOVELL_SOCKET_CLOSE;
        int86(INT_NOVELL, &regs, &regs);

        ipx_is_open = false;
    }
}

/**
 * @brief send a packet through an IPX socket.
 *
 * @param data the data to send.
 * @param node destination node address or IPX_BROADCAST_ADDR to send to all nodes on network.
 */
void ipx_send_packet(const ipx_data_t *data, const ipx_node_t *node) {
    union REGS regs;
    struct SREGS sregs;
    bzero(&regs, sizeof(regs));
    bzero(&sregs, sizeof(sregs));

    ipx_idle();

    // copy data and destination address
    memcpy(ipx.send.data, data, sizeof(ipx_data_t));
    IPX_COPY_NODE(ipx.send.ecb.immediate_addr, node);
    IPX_COPY_NODE(ipx.send.header.destination.node, node);

    regs.x.si = FP_OFF((char far *)&ipx.send);  // ecb address
    sregs.es = FP_SEG((char far *)&ipx.send);

    regs.x.bx = INT_NOVELL_SEND_PACKET;
    int86x(INT_NOVELL, &regs, &regs, &sregs);

    ipx_idle();
    delay(3);
}

/**
 * @brief check if a packet is available in the ringbuffer.
 *
 * @return true if there are packets waiting, else false.
 */
bool ipx_check_packet() { return ipx.buffer.start != ipx.buffer.end; }

/**
 * @brief get a packet from the ringbuffer.
 *
 * @param rec pointer to memory for packet data and source address.
 *
 * @return true if a packet could be retrieved, else false.
 */
bool ipx_get_packet(ipx_received_t *rec) {
    if (!ipx_check_packet()) {
        return false;
    }

    memcpy(rec, &ipx.buffer.packets[ipx.buffer.start], sizeof(ipx_received_t));

    ipx.buffer.start++;
    if (ipx.buffer.start > IPX_BUFFER_SIZE) {
        ipx.buffer.start = 0;
    }

    return true;
}
