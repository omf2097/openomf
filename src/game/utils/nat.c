#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "game/utils/settings.h"
#include "nat.h"
#include "utils/log.h"

bool nat_create_upnp_mapping(nat_ctx *ctx, uint16_t int_port, uint16_t ext_port) {
#ifdef MINIUPNPC_FOUND
    char int_portstr[6];
    snprintf(int_portstr, sizeof(int_portstr), "%d", int_port);

    char ext_portstr[6];
    snprintf(ext_portstr, sizeof(ext_portstr), "%d", ext_port);

    // get the external (WAN) IP address
    // char wan_address[64];
    // UPNP_GetExternalIPAddress(ctx->upnp_urls.controlURL, ctx->upnp_data.first.servicetype, wan_address);

    // add a new UDP port mapping from WAN port 12345 to local host port 24680
    int error =
        UPNP_AddPortMapping(ctx->upnp_urls.controlURL, ctx->upnp_data.first.servicetype,
                            ext_portstr,      // external (WAN) port requested
                            int_portstr,      // internal (LAN) port to which packets will be redirected
                            ctx->lan_address, // internal (LAN) address to which packets will be redirected
                            "OpenOMF", // text description to indicate why or who is responsible for the port mapping
                            "UDP",     // protocol must be either TCP or UDP
                            NULL,      // remote (peer) host address or nullptr for no restriction
                            "86400");  // port map lease duration (in seconds) or zero for "as long as possible"
    if(error == 0) {
        DEBUG("NAT-uPNP Port map successfully created from %d to %d!", int_port, ext_port);

        ctx->int_port = int_port;
        ctx->ext_port = ext_port;
        return true;
    } else {
        DEBUG("NAT-uPNP port %d -> %d mapping failed with %d", int_port, ext_port, error);
        // TODO there are some errors we can work around here
        // like overly short lifetimes
        return false;
    }
#else
    return false;
#endif
}

bool nat_create_pmp_mapping(nat_ctx *ctx, uint16_t int_port, uint16_t ext_port) {
#ifdef NATPMP_FOUND
    int r;
    natpmpresp_t response;
    sendnewportmappingrequest(&ctx->natpmp, NATPMP_PROTOCOL_UDP, int_port, ext_port, 3600);
    do {
        fd_set fds;
        struct timeval timeout;
        FD_ZERO(&fds);
        FD_SET(ctx->natpmp.s, &fds);
        getnatpmprequesttimeout(&ctx->natpmp, &timeout);
        select(FD_SETSIZE, &fds, NULL, NULL, &timeout);
        r = readnatpmpresponseorretry(&ctx->natpmp, &response);
    } while(r == NATPMP_TRYAGAIN);

    if(r == 0) {
        DEBUG("mapped public port %hu to localport %hu liftime %u\n", response.pnu.newportmapping.mappedpublicport,
              response.pnu.newportmapping.privateport, response.pnu.newportmapping.lifetime);
        ctx->int_port = response.pnu.newportmapping.privateport;
        ctx->ext_port = response.pnu.newportmapping.mappedpublicport;
        return true;
    } else {
        DEBUG("NAT-PMP %d -> %d failed with error %d", int_port, ext_port, r);
        // TODO handle some errors here
        return false;
    }
#else
    return false;
#endif
}

bool nat_create_mapping(nat_ctx *ctx, uint16_t int_port, uint16_t ext_port) {
    switch(ctx->type) {
        case NAT_TYPE_UPNP:
            return nat_create_upnp_mapping(ctx, int_port, ext_port);
            break;
        case NAT_TYPE_PMP:
            return nat_create_pmp_mapping(ctx, int_port, ext_port);
            break;
        default:
            return false;
    }
}

// clang-format off
void nat_try_upnp(nat_ctx *ctx) {
#ifdef MINIUPNPC_FOUND
    int error = 0;
    ctx->upnp_dev = upnpDiscover(2000,    // time to wait (milliseconds)
                                 NULL,    // multicast interface (or null defaults to 239.255.255.250)
                                 NULL,    // path to minissdpd socket (or null defaults to /var/run/minissdpd.sock)
                                 0,       // source port to use (or zero defaults to port 1900)
                                 0,       // 0==IPv4, 1==IPv6
                                 2,       // TTL
                                 &error); // error condition
    // TODO check error here?
    // try to look up our lan address, to test it
    #if(MINIUPNPC_API_VERSION >= 18)
        int status = UPNP_GetValidIGD(ctx->upnp_dev, &ctx->upnp_urls, &ctx->upnp_data, ctx->lan_address,
                                      sizeof(ctx->lan_address), NULL, 0);
    #else
        int status =
            UPNP_GetValidIGD(ctx->upnp_dev, &ctx->upnp_urls, &ctx->upnp_data, ctx->lan_address, sizeof(ctx->lan_address));
    #endif

    // look up possible "status" values, the number "1" indicates a valid IGD was found
    if(status == 1) {
        DEBUG("discovered uPNP server");
        ctx->type = NAT_TYPE_UPNP;
    }
#else
    DEBUG("NAT-uPNP support not available");
#endif
}
// clang-format on

void nat_try_pmp(nat_ctx *ctx) {
#ifdef NATPMP_FOUND
    // try nat-pmp
    in_addr_t forcedgw = {0};
    if(initnatpmp(&ctx->natpmp, 0, forcedgw) == 0) {
        DEBUG("discovered NAT-PMP server");
        ctx->type = NAT_TYPE_PMP;
    }
#else
    DEBUG("NAT-PMP support not available");
#endif
}

void nat_release_upnp(nat_ctx *ctx) {
#ifdef MINIUPNPC_FOUND
    char ext_portstr[6];
    snprintf(ext_portstr, sizeof(ext_portstr), "%d", ctx->ext_port);

    int error =
        UPNP_DeletePortMapping(ctx->upnp_urls.controlURL, ctx->upnp_data.first.servicetype, ext_portstr, "UDP", NULL);
    if(error == 0) {
        DEBUG("successfully removed NAT-uPNP port mapping for %d -> %d", ctx->int_port, ctx->ext_port);
    } else {
        DEBUG("failed to remove port mapping with %d", error);
    }
    FreeUPNPUrls(&ctx->upnp_urls);
    freeUPNPDevlist(ctx->upnp_dev);
#endif
}

void nat_release_pmp(nat_ctx *ctx) {
#ifdef NATPMP_FOUND
    natpmpresp_t response;
    int r;
    sendnewportmappingrequest(&ctx->natpmp, NATPMP_PROTOCOL_UDP, ctx->int_port, ctx->ext_port, 0);
    do {
        fd_set fds;
        struct timeval timeout;
        FD_ZERO(&fds);
        FD_SET(ctx->natpmp.s, &fds);
        getnatpmprequesttimeout(&ctx->natpmp, &timeout);
        select(FD_SETSIZE, &fds, NULL, NULL, &timeout);
        r = readnatpmpresponseorretry(&ctx->natpmp, &response);
    } while(r == NATPMP_TRYAGAIN);
    if(r == 0) {
        DEBUG("released public port %hu to localport %huu\n", response.pnu.newportmapping.mappedpublicport,
              response.pnu.newportmapping.privateport);
    } else {
        DEBUG("NAT-PMP release failed with error %d", r);
    }
    closenatpmp(&ctx->natpmp);
#endif
}

void nat_create(nat_ctx *ctx) {

    ctx->type = NAT_TYPE_NONE;
    ctx->int_port = 0;
    ctx->ext_port = 0;

    if(settings_get()->net.net_use_upnp) {
        nat_try_upnp(ctx);
    }

    if(ctx->type != NAT_TYPE_NONE) {
        return;
    }

    if(settings_get()->net.net_use_pmp) {
        nat_try_pmp(ctx);
    }
}

void nat_free(nat_ctx *ctx) {
    switch(ctx->type) {
        case NAT_TYPE_UPNP:
            nat_release_upnp(ctx);
            break;
        case NAT_TYPE_PMP:
            nat_release_pmp(ctx);
            break;
        default:
            break;
    }
    ctx->type = NAT_TYPE_NONE;
}
