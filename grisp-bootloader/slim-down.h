/**
 * @file
 *
 * @brief Stage-2 slim down.
 *
 * This file replaces some (heavy) functions from libbsd with dummy
 * implementations. This reduces the memory footprint.
 */

#include <netdb.h>

/* Remove some features of ifconfig. */

void _bsd_ifconfig_bridge_ctor(void)
{
}

void _bsd_ifconfig_gif_ctor(void)
{
}

void _bsd_ifconfig_gre_ctor(void)
{
}

void _bsd_ifconfig_group_ctor(void)
{
}

void _bsd_ifconfig_ieee80211_ctor(void)
{
}

void _bsd_ifconfig_lagg_ctor(void)
{
}

void _bsd_ifconfig_pfsync_ctor(void)
{
}

void _bsd_ifconfig_vlan_ctor(void)
{
}


/* Remove name service. */

struct hostent * gethostbyname(const char *name)
{
  (void) name;
  return NULL;
}

struct hostent * gethostbyname2(const char *name, int af)
{
  (void) name; (void) af;
  return NULL;
}

struct hostent * gethostbyaddr(const void *addr, socklen_t len, int af)
{
  (void) addr; (void) len; (void) af;
  return NULL;
}

struct netent * getnetbyname(const char *name)
{
  (void) name;
  return NULL;
}

struct netent *getnetbyaddr(uint32_t net, int type)
{
  (void) net; (void) type;
  return NULL;
}

int getaddrinfo(const char *hostname, const char *servname,
    const struct addrinfo *hints, struct addrinfo **res)
{
  (void) hostname; (void) servname; (void) hints; (void) res;
  return EAI_FAIL;
}

int getnameinfo(const struct sockaddr *sa, socklen_t salen,
    char *host, size_t hostlen, char *serv, size_t servlen,
    int flags)
{
  (void) sa; (void) salen; (void) host; (void) hostlen; (void) serv;
  (void) servlen; (void) flags;
  return EAI_FAIL;
}

const char * gai_strerror(int ecode)
{
  static const char *ret = "name resolution not implemented";
  (void) ecode;
  return ret;
}

void freeaddrinfo(struct addrinfo *ai)
{
  (void) ai;
}
