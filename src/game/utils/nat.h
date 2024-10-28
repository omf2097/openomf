
#ifdef NATPMP_FOUND
#include <natpmp.h>
#endif

#ifdef MINIUPNPC_FOUND
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
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
#endif
} nat_ctx;

void nat_create(nat_ctx *ctx, uint16_t port);

void nat_free(nat_ctx *ctx);
