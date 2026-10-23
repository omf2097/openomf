/* ENet stand-in. Calls succeed or no-op; no sockets are opened. */
#ifndef MIN_LIB_BUILD
#error "enet_stub.c is part of MIN_LIB_BUILD"
#endif

#include <enet/enet.h>

int enet_initialize(void) {
    return 0;
}

void enet_deinitialize(void) {
}

ENetHost *enet_host_create(const ENetAddress *address, size_t peerCount, size_t channelLimit,
                           enet_uint32 incomingBandwidth, enet_uint32 outgoingBandwidth) {
    (void)address;
    (void)peerCount;
    (void)channelLimit;
    (void)incomingBandwidth;
    (void)outgoingBandwidth;
    return NULL;
}

void enet_host_destroy(ENetHost *host) {
    (void)host;
}

int enet_host_service(ENetHost *host, ENetEvent *event, enet_uint32 timeout) {
    (void)host;
    (void)event;
    (void)timeout;
    return 0;
}

void enet_host_flush(ENetHost *host) {
    (void)host;
}

ENetPeer *enet_host_connect(ENetHost *host, const ENetAddress *address, size_t channelCount, enet_uint32 data) {
    (void)host;
    (void)address;
    (void)channelCount;
    (void)data;
    return NULL;
}

int enet_address_set_host(ENetAddress *address, const char *hostName) {
    (void)address;
    (void)hostName;
    return -1;
}

ENetPacket *enet_packet_create(const void *data, size_t dataLength, enet_uint32 flags) {
    (void)data;
    (void)dataLength;
    (void)flags;
    return NULL;
}

void enet_packet_destroy(ENetPacket *packet) {
    (void)packet;
}

int enet_peer_send(ENetPeer *peer, enet_uint8 channelID, ENetPacket *packet) {
    (void)peer;
    (void)channelID;
    (void)packet;
    return -1;
}

void enet_peer_reset(ENetPeer *peer) {
    (void)peer;
}

void enet_peer_disconnect_later(ENetPeer *peer, enet_uint32 data) {
    (void)peer;
    (void)data;
}

void enet_peer_timeout(ENetPeer *peer, enet_uint32 timeoutLimit, enet_uint32 timeoutMinimum,
                       enet_uint32 timeoutMaximum) {
    (void)peer;
    (void)timeoutLimit;
    (void)timeoutMinimum;
    (void)timeoutMaximum;
}

void enet_peer_ping_interval(ENetPeer *peer, enet_uint32 pingInterval) {
    (void)peer;
    (void)pingInterval;
}

int enet_socket_set_option(ENetSocket socket, ENetSocketOption option, int value) {
    (void)socket;
    (void)option;
    (void)value;
    return -1;
}
