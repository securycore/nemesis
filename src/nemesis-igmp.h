/*
 * $Id: nemesis-igmp.h,v 1.1.1.1.4.1 2005/01/27 20:14:53 jnathan Exp $
 *
 * THE NEMESIS PROJECT
 * Copyright (C) 1999, 2000, 2001 Mark Grimes <mark@stateful.net>
 * Copyright (C) 2001 - 2003 Jeff Nathan <jeff@snort.org>
 *
 * nemesis-igmp.h (IGMP Packet Injector)
 *
 */

#ifndef __NEMESIS_IGMP_H__
#define __NEMESIS_IGMP_H__

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>
#if defined(WIN32)
#include <pcap.h>
#endif
#if defined(HAVE_NETINET_IN_H)
#include <netinet/in.h>
#elif defined(WIN32)
#include <winsock2.h>
#endif
#include "nemesis.h"
#include <libnet.h>

int buildigmp(ETHERhdr *, IPhdr *, IGMPhdr *, FileData *, FileData *, libnet_t *);

#endif /* __NEMESIS_IGMP_H__ */
