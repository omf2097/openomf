
#ifdef NATPMP_FOUND
#include <natpmp.h>
#endif

#ifdef MINIUPNPC_FOUND
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#endif

// socklen_t nonsense
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

typedef enum
{
    NAT_TYPE_NONE,
    NAT_TYPE_UPNP,
    NAT_TYPE_PMP
} nat_type_t;

typedef struct nat_ctx_t {
    nat_type_t type;
    uint16_t int_port;
    uint16_t ext_port;
#ifdef MINIUPNPC_FOUND
    struct UPNPUrls upnp_urls;
    struct IGDdatas upnp_data;
    struct UPNPDev *upnp_dev;
#endif
#ifdef NATPMP_FOUND
    natpmp_t natpmp;
#endif
} nat_ctx;

void nat_create(nat_ctx *ctx);

bool nat_create_mapping(nat_ctx *ctx, uint16_t int_port, uint16_t ext_port);

void nat_free(nat_ctx *ctx);
