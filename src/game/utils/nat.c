#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "game/utils/settings.h"
#include "nat.h"
#include "utils/log.h"

#ifdef MINIUPNPC_FOUND
uint16_t upnp_find_wildcard_port(nat_ctx *ctx, uint16_t int_port) {
    // Retrieve port mappings list
    struct PortMappingParserData portMappings;
    memset(&portMappings, 0, sizeof(portMappings));

    int result = UPNP_GetListOfPortMappings(ctx->upnp_urls.controlURL, ctx->upnp_data.first.servicetype,
                                            "0",     // startPort
                                            "65535", // endPort
                                            "UDP",   // protocol
                                            "0",     // numberOfPorts (0 = all)
                                            &portMappings);

    if(result != UPNPCOMMAND_SUCCESS) {
        log_debug("GetListOfPortMappings failed: %d, cannot obtain wildcard port", result);
        return 0;
    }

    uint16_t ext_port = 0;

    // Search for our mapping
    struct PortMapping *pm;
    for(pm = portMappings.l_head; pm != NULL; pm = pm->l_next) {
        if(strcmp(pm->internalClient, ctx->lan_address) == 0 && pm->internalPort == int_port &&
           strcmp(pm->description, "OpenOMF") == 0 && strcmp(pm->protocol, "UDP") == 0) {
            ext_port = pm->externalPort;
            break;
        }
    }

    // Cleanup
    FreePortListing(&portMappings);
    return ext_port;
}
#endif

bool nat_create_upnp_mapping(nat_ctx *ctx, uint16_t int_port, uint16_t ext_port) {
#ifdef MINIUPNPC_FOUND
    char int_portstr[6];
    snprintf(int_portstr, sizeof(int_portstr), "%d", int_port);

    char ext_portstr[6];
    snprintf(ext_portstr, sizeof(ext_portstr), "%d", ctx->wildcard_ext_port ? 0 : ext_port);

    char const *lease_duration = ctx->use_permanent_lease ? "0" : "86400";

    int error =
        UPNP_AddPortMapping(ctx->upnp_urls.controlURL, ctx->upnp_data.first.servicetype,
                            ext_portstr,      // external (WAN) port requested
                            int_portstr,      // internal (LAN) port to which packets will be redirected
                            ctx->lan_address, // internal (LAN) address to which packets will be redirected
                            "OpenOMF", // text description to indicate why or who is responsible for the port mapping
                            "UDP",     // protocol must be either TCP or UDP
                            NULL,      // remote (peer) host address or nullptr for no restriction
                            lease_duration); // port map lease duration (in seconds) or zero for "as long as possible"

    if(error == 0) {
        log_debug("NAT-uPNP Port map successfully created from %d to %d!", int_port, ext_port);

        ctx->int_port = int_port;
        if(ctx->wildcard_ext_port) {
            // we are not given our wildcard port in the response, so we have to wallow in uPNP some more to get it
            ctx->ext_port = upnp_find_wildcard_port(ctx, int_port);
        } else {
            ctx->ext_port = ext_port;
        }
        return true;
    } else {
        log_debug("NAT-uPNP port %d -> %d mapping failed with %d", int_port, ext_port, error);
        if(error == 725 /* OnlyPermanentLeasesSupported */ && !ctx->use_permanent_lease) {
            log_debug("NAT-uPNP error 725 is 'OnlyPermanentLeasesSupported', so retrying for a permanent lease.");
            ctx->use_permanent_lease = true;
            return false; // try again
        } else if(error == 718) {
            log_debug("NAT-uPNP error 718 is 'ConflictInMappingEntry', so retrying with a different external port.");
            return false; // try again
        } else if(error == 727 && !ctx->wildcard_ext_port) {
            log_debug(
                "NAT-uPNP error 718 is 'ExternalPortOnlySupportsWildcard', so retrying with a wildcard external port.");
            ctx->wildcard_ext_port = true;
            return false; // try again
        } else if(error == 724) {
            log_debug("NAT-uPNP error 724 is 'SamePortValuesRequired', but handling this is not yet implemented");
        } else {
            log_debug("NAT-uPNP error %d is unhandled", error);
        }
        // give up, these errors are not recoverable
        FreeUPNPUrls(&ctx->upnp_urls);
        freeUPNPDevlist(ctx->upnp_dev);
        ctx->type = NAT_TYPE_NONE;
        return false;
    }
#else
    return false;
#endif
}

#ifdef NATPMP_FOUND
// helper function
int readpmpresponse(nat_ctx *ctx, natpmpresp_t *response) {
    int r;
    do {
        fd_set fds;
        struct timeval timeout;
        FD_ZERO(&fds);
#ifdef _WIN32
        log_debug("praying that winapi socket '%d' hasn't been truncated", ctx->natpmp.s);
        // XXX : libnatpmp stores winapi SOCKETs as an `int`, which is terrible because SOCKET is pointer-sized.
        // WinAPI seems to be kind to us, and is returning small values like 0x00000114.. but this is far from
        // guaranteed! (using ptrdiff for sign-extension in case `s` is INVALID_SOCKET -1)
        FD_SET((SOCKET)(ptrdiff_t)ctx->natpmp.s, &fds);
#else
        FD_SET(ctx->natpmp.s, &fds);
#endif
        getnatpmprequesttimeout(&ctx->natpmp, &timeout);
        select(FD_SETSIZE, &fds, NULL, NULL, &timeout);
        r = readnatpmpresponseorretry(&ctx->natpmp, response);
    } while(r == NATPMP_TRYAGAIN);
    return r;
}
#endif

bool nat_create_pmp_mapping(nat_ctx *ctx, uint16_t int_port, uint16_t ext_port) {
#ifdef NATPMP_FOUND
    natpmpresp_t response;
    sendnewportmappingrequest(&ctx->natpmp, NATPMP_PROTOCOL_UDP, int_port, ext_port, 3600);
    int r = readpmpresponse(ctx, &response);

    if(r == 0) {
        log_debug("mapped public port %hu to localport %hu lifetime %u", response.pnu.newportmapping.mappedpublicport,
                  response.pnu.newportmapping.privateport, response.pnu.newportmapping.lifetime);
        ctx->int_port = response.pnu.newportmapping.privateport;
        ctx->ext_port = response.pnu.newportmapping.mappedpublicport;
        return true;
    } else if(r == NATPMP_ERR_NOGATEWAYSUPPORT) {
        closenatpmp(&ctx->natpmp);
        ctx->type = NAT_TYPE_NONE;
        return false;
    } else {
        log_debug("NAT-PMP %d -> %d failed with error %d", int_port, ext_port, r);
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
    ctx->upnp_dev = upnpDiscover(500,    // time to wait (milliseconds)
                                 NULL,    // multicast interface (or null defaults to 239.255.255.250)
                                 NULL,    // path to minissdpd socket (or null defaults to /var/run/minissdpd.sock)
                                 0,       // source port to use (or zero defaults to port 1900)
                                 0,       // 0==IPv4, 1==IPv6
                                 2,       // TTL
                                 &error); // error condition
    if(ctx->upnp_dev) {
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
        if(ctx->upnp_dev && status == 1) {
            log_debug("discovered uPNP server");
            // get the external (WAN) IP address
            if(UPNP_GetExternalIPAddress(ctx->upnp_urls.controlURL, ctx->upnp_data.first.servicetype, ctx->wan_address)) {
                // if this fails, zero the field out
                ctx->wan_address[0] = 0;
            }
            ctx->use_permanent_lease = false;
            ctx->wildcard_ext_port = false;
            ctx->type = NAT_TYPE_UPNP;
        } else {
            FreeUPNPUrls(&ctx->upnp_urls);
            freeUPNPDevlist(ctx->upnp_dev);
        }
    }
#else
    log_debug("NAT-uPNP support not available");
#endif
}
// clang-format on

void nat_try_pmp(nat_ctx *ctx) {
#ifdef NATPMP_FOUND
    // try nat-pmp
    in_addr_t forcedgw = {0};
    if(initnatpmp(&ctx->natpmp, 0, forcedgw) == 0) {
        log_debug("discovered NAT-PMP server");
        natpmpresp_t response;
        sendpublicaddressrequest(&ctx->natpmp);
        int r = readpmpresponse(ctx, &response);
        if(r == 0) {
            inet_ntop(AF_INET, (void *)&response.pnu.publicaddress.addr, ctx->wan_address, sizeof(ctx->wan_address));
        }
        ctx->type = NAT_TYPE_PMP;
    }
#else
    log_debug("NAT-PMP support not available");
#endif
}

void nat_release_upnp(nat_ctx *ctx) {
#ifdef MINIUPNPC_FOUND
    char ext_portstr[6];
    snprintf(ext_portstr, sizeof(ext_portstr), "%d", ctx->ext_port);

    int error =
        UPNP_DeletePortMapping(ctx->upnp_urls.controlURL, ctx->upnp_data.first.servicetype, ext_portstr, "UDP", NULL);
    if(error == 0) {
        log_debug("successfully removed NAT-uPNP port mapping for %d -> %d", ctx->int_port, ctx->ext_port);
    } else {
        log_debug("failed to remove port mapping with %d", error);
    }
    FreeUPNPUrls(&ctx->upnp_urls);
    freeUPNPDevlist(ctx->upnp_dev);
#endif
}

void nat_release_pmp(nat_ctx *ctx) {
#ifdef NATPMP_FOUND
    natpmpresp_t response;
    sendnewportmappingrequest(&ctx->natpmp, NATPMP_PROTOCOL_UDP, ctx->int_port, ctx->ext_port, 0);
    int r = readpmpresponse(ctx, &response);
    if(r == 0) {
        log_debug("released public port %hu to localport %hu", ctx->int_port, response.pnu.newportmapping.privateport);
    } else {
        log_debug("NAT-PMP release failed with error %d", r);
    }
    closenatpmp(&ctx->natpmp);
#endif
}

void nat_create(nat_ctx *ctx) {

    if(ctx->type != NAT_TYPE_NONE) {
        nat_free(ctx);
    }
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
