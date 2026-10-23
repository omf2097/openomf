#ifndef ENET_H
#define ENET_H

#include <stddef.h>
#include <stdint.h>

/* Enough of ENet for MIN_LIB_BUILD to compile. Nothing here sends or receives. */

typedef uint8_t enet_uint8;
typedef uint16_t enet_uint16;
typedef uint32_t enet_uint32;

typedef struct ENetAddress {
    enet_uint32 host;
    enet_uint16 port;
} ENetAddress;

#define ENET_HOST_ANY 0

typedef struct ENetPacket {
    enet_uint8 *data;
    size_t dataLength;
} ENetPacket;

typedef int ENetSocket;

typedef struct ENetHost {
    ENetSocket socket;
    ENetAddress address;
} ENetHost;

typedef struct ENetPeer {
    ENetAddress address;
    void *data;
    enet_uint32 connectID;
    enet_uint32 roundTripTime;
} ENetPeer;

typedef enum ENetEventType
{
    ENET_EVENT_TYPE_NONE = 0,
    ENET_EVENT_TYPE_CONNECT = 1,
    ENET_EVENT_TYPE_DISCONNECT = 2,
    ENET_EVENT_TYPE_RECEIVE = 3
} ENetEventType;

typedef struct ENetEvent {
    ENetEventType type;
    ENetPeer *peer;
    ENetPacket *packet;
} ENetEvent;

#define ENET_PACKET_FLAG_RELIABLE (1u << 0)
#define ENET_PACKET_FLAG_UNSEQUENCED (1u << 1)

typedef enum ENetSocketOption
{
    ENET_SOCKOPT_REUSEADDR = 5
} ENetSocketOption;

int enet_initialize(void);
void enet_deinitialize(void);
ENetHost *enet_host_create(const ENetAddress *address, size_t peerCount, size_t channelLimit,
                           enet_uint32 incomingBandwidth, enet_uint32 outgoingBandwidth);
void enet_host_destroy(ENetHost *host);
int enet_host_service(ENetHost *host, ENetEvent *event, enet_uint32 timeout);
void enet_host_flush(ENetHost *host);
ENetPeer *enet_host_connect(ENetHost *host, const ENetAddress *address, size_t channelCount, enet_uint32 data);
int enet_address_set_host(ENetAddress *address, const char *hostName);
ENetPacket *enet_packet_create(const void *data, size_t dataLength, enet_uint32 flags);
void enet_packet_destroy(ENetPacket *packet);
int enet_peer_send(ENetPeer *peer, enet_uint8 channelID, ENetPacket *packet);
void enet_peer_reset(ENetPeer *peer);
void enet_peer_disconnect_later(ENetPeer *peer, enet_uint32 data);
void enet_peer_timeout(ENetPeer *peer, enet_uint32 timeoutLimit, enet_uint32 timeoutMinimum,
                       enet_uint32 timeoutMaximum);
void enet_peer_ping_interval(ENetPeer *peer, enet_uint32 pingInterval);
int enet_socket_set_option(ENetSocket socket, ENetSocketOption option, int value);

#endif /* ENET_H */
