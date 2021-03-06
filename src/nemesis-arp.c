/*
 * $Id: nemesis-arp.c,v 1.1.1.1.4.1 2005/01/27 20:14:53 jnathan Exp $
 *
 * THE NEMESIS PROJECT
 * Copyright (C) 1999, 2000, 2001 Mark Grimes <mark@stateful.net>
 * Copyright (C) 2001 - 2003 Jeff Nathan <jeff@snort.org>
 *
 * nemesis-arp.c (ARP/RARP Packet Injector)
 *
 */

#include "nemesis-arp.h"
#include "nemesis.h"
#if defined(WIN32)
#include <pcap.h>
#endif

static ETHERhdr etherhdr;
static ARPhdr   arphdr;
static FileData pd;
static int      solarismode;
static int      arp_src, arp_dst; /* modify hardware addresses independantly within arp frame */
static int      rarp;             /* RARP */
static int      reply;            /* ARP/RARP request, 1 == reply */
static char *   device = NULL;    /* Ethernet device */
static char *   file   = NULL;    /* payload file name */

#if defined(WIN32)
static char *ifacetmp = NULL;
#endif

static void arp_cmdline(int, char **);
static int  arp_exit(int);
static void arp_initdata(void);
static void arp_usage(char *);
static void arp_validatedata(void);
static void arp_verbose(void);

void nemesis_arp(int argc, char **argv)
{
	const char *module = "ARP/RARP Packet Injection";
	libnet_t *l;

	nemesis_maketitle(title, module, version);

	if (argc > 1 && !strncmp(argv[1], "help", 4))
		arp_usage(argv[0]);

	arp_initdata();
	arp_cmdline(argc, argv);

	// create by Jacyu, create the libnet handler
	l = libnet_init(LIBNET_LINK_ADV, device, errbuf);
	if (l == NULL) {
		fprintf(stderr, "ERROR: Device not specified and unable to automatically select a device.\n");
		arp_exit(1);
	}

	arp_validatedata();

	/* Determine if there's a source hardware address set */
	if ((nemesis_check_link(&etherhdr, l)) < 0) {
		fprintf(stderr, "ERROR: Cannot retrieve hardware address of %s.\n", device);
		arp_exit(1);
	}

	arp_verbose();

	if (got_payload) {
		if (builddatafromfile(ARPBUFFSIZE, &pd, (const char *)file, (const u_int32_t)PAYLOADMODE) < 0)
			arp_exit(1);
	}

	if (buildarp(&etherhdr, &arphdr, &pd, l) < 0) {
		printf("\n%s Injection Failure\n", (rarp == 0 ? "ARP" : "RARP"));
		arp_exit(1);
	} else {
		printf("\n%s Packet Injected\n", (rarp == 0 ? "ARP" : "RARP"));
		arp_exit(0);
	}
}

static void arp_initdata(void)
{
	/* defaults */
	etherhdr.ether_type = ETHERTYPE_ARP;   /* Ethernet type ARP */
	memset(etherhdr.ether_shost, 0, 6);    /* Ethernet source address */
	memset(etherhdr.ether_dhost, 0xff, 6); /* Ethernet destination address */

	arphdr.ar_op  = ARPOP_REQUEST; /* ARP opcode: request */
	arphdr.ar_hrd = ARPHRD_ETHER;  /* hardware format: Ethernet */
	arphdr.ar_pro = ETHERTYPE_IP;  /* protocol format: IP */
	arphdr.ar_hln = 6;             /* 6 byte hardware addresses */
	arphdr.ar_pln = 4;             /* 4 byte protocol addresses */

	memset(ar_sha, 0, 6); /* ARP frame sender address */
	memset(ar_spa, 0, 4); /* ARP sender protocol (IP) addr */
	memset(ar_tha, 0, 6); /* ARP frame target address */
	memset(ar_tpa, 0, 4); /* ARP target protocol (IP) addr */

	pd.file_mem = NULL; /* payload */
	pd.file_s   = 0;    /* paload size */
	return;
}

static void arp_validatedata()
{
	/* validation tests */
	if ((!memcmp(ar_spa, zero, 4)) || (!memcmp(ar_tpa, zero, 4))) {
		fprintf(stderr, "ERROR: Source and/or Destination IP address missing.\n");
		arp_exit(1);
	}

	if (solarismode && arp_dst) {
		fprintf(stderr, "ERROR: Using -s and -m is redundant, choose one or the other.\n");
		arp_exit(1);
	}

	/* for RARP functionality, set the appropriate opcode in the ARP frame */
	if (rarp) {
		if (reply)
			arphdr.ar_op = ARPOP_REVREPLY;
		else
			arphdr.ar_op = ARPOP_REVREQUEST;
	} else {
		if (reply)
			arphdr.ar_op = ARPOP_REPLY;
		else
			arphdr.ar_op = ARPOP_REQUEST;
	}

	/*
	 * If separate hardware addresses have been specified for ARP
	 * frame use them.  Otherwise, use values from Ethernet frame.
	 */
	if (reply) {
		if (!arp_src)
			memcpy(ar_sha, etherhdr.ether_shost, 6);
		if (!arp_dst)
			memcpy(ar_tha, etherhdr.ether_dhost, 6);
	} else {
		if (!arp_src)
			memcpy(ar_sha, etherhdr.ether_shost, 6);
	}
	return;
}

static void arp_usage(char *arg)
{
	nemesis_printtitle((const char *)title);

	printf("ARP/RARP Usage:\n  %s [-v (verbose)] [options]\n\n", arg);
	printf("ARP/RARP Options: \n"
	       "  -S <Source IP address>\n"
	       "  -D <Destination IP address>\n"
	       "  -h <Sender MAC address within ARP frame>\n"
	       "  -m <Target MAC address within ARP frame>\n"
	       "  -s <Solaris style ARP requests with target hardware addess set to broadcast>\n"
	       "  -r ({ARP,RARP} REPLY enable)\n"
	       "  -R (RARP enable)\n"
	       "  -P <Payload file>\n\n");
	printf("Data Link Options: \n"
#if defined(WIN32)
	       "  -d <Ethernet device number>\n"
#else
	       "  -d <Ethernet device name>\n"
#endif
	       "  -H <Source MAC address>\n"
	       "  -M <Destination MAC address>\n");
#if defined(WIN32)
	printf("  -Z (List available network interfaces by number)\n");
#endif
	putchar('\n');
	printf("You must define a Source and Destination IP address.\n");
	arp_exit(1);
}

static void arp_cmdline(int argc, char **argv)
{
	int          opt, i;
	u_int32_t    addr_tmp[6];
	char *       arp_options;
	extern char *optarg;
	extern int   optind;

#if defined(ENABLE_PCAPOUTPUT)
#if defined(WIN32)
	arp_options = "d:D:h:H:L:m:M:P:S:rRsvWZ?";
#else
	arp_options = "d:D:h:H:L:m:M:P:S:rRsvW?";
#endif
#else
#if defined(WIN32)
	arp_options = "d:D:h:H:L:m:M:P:S:rRsvZ?";
#else
	arp_options = "d:D:h:H:L:m:M:P:S:rRsv?";
#endif
#endif

	while ((opt = getopt(argc, argv, arp_options)) != -1) {
		switch (opt) {
		case 'd': /* Ethernet device */
#if defined(WIN32)
			if (nemesis_getdev(atoi(optarg), &device) < 0) {
				fprintf(stderr, "ERROR: Unable to lookup device: '%d'.\n", atoi(optarg));
				arp_exit(1);
			}
#else
			if (strlen(optarg) < 256)
				device = strdup(optarg);
			else {
				fprintf(stderr, "ERROR: device %s > 256 characters.\n", optarg);
				arp_exit(1);
			}
#endif
			break;
		case 'D': /* ARP target IP address */
			if (nemesis_name_resolve(optarg, (uint32_t *)ar_tpa) < 0) {
				fprintf(stderr, "ERROR: Invalid destination IP address: \"%s\".\n", optarg);
				arp_exit(1);
			}
			break;
		case 'h': /* ARP sender hardware address */
			memset(addr_tmp, 0, sizeof(addr_tmp));
			arp_src = 1;
			sscanf(optarg, "%02X:%02X:%02X:%02X:%02X:%02X", &addr_tmp[0],
			       &addr_tmp[1], &addr_tmp[2], &addr_tmp[3], &addr_tmp[4], &addr_tmp[5]);
			for (i = 0; i < 6; i++)
				ar_sha[i] = (u_int8_t)addr_tmp[i];
			break;
		case 'H': /* Ethernet source address */
			memset(addr_tmp, 0, sizeof(addr_tmp));
			sscanf(optarg, "%02X:%02X:%02X:%02X:%02X:%02X", &addr_tmp[0],
			       &addr_tmp[1], &addr_tmp[2], &addr_tmp[3], &addr_tmp[4], &addr_tmp[5]);
			for (i = 0; i < 6; i++)
				etherhdr.ether_shost[i] = (u_int8_t)addr_tmp[i];
			break;
		case 'm': /* ARP target hardware address */
			memset(addr_tmp, 0, sizeof(addr_tmp));
			arp_dst = 1;
			sscanf(optarg, "%02X:%02X:%02X:%02X:%02X:%02X", &addr_tmp[0],
			       &addr_tmp[1], &addr_tmp[2], &addr_tmp[3], &addr_tmp[4], &addr_tmp[5]);
			for (i = 0; i < 6; i++)
				ar_tha[i] = (u_int8_t)addr_tmp[i];
			break;
		case 'M': /* Ethernet destination address */
			memset(addr_tmp, 0, sizeof(addr_tmp));
			sscanf(optarg, "%02X:%02X:%02X:%02X:%02X:%02X", &addr_tmp[0],
			       &addr_tmp[1], &addr_tmp[2], &addr_tmp[3], &addr_tmp[4], &addr_tmp[5]);
			for (i = 0; i < 6; i++)
				etherhdr.ether_dhost[i] = (u_int8_t)addr_tmp[i];
			break;
		case 'P': /* payload file */
			if (strlen(optarg) < 256) {
				file        = strdup(optarg);
				got_payload = 1;
			} else {
				fprintf(stderr, "ERROR: payload file %s > 256 characters.\n", optarg);
				arp_exit(1);
			}
			break;
		case 'r': /* ARP/RARP reply */
			reply = 1;
			break;
		case 'R': /* RARP */
			etherhdr.ether_type = ETHERTYPE_REVARP;
			rarp                = 1;
			break;
		case 's':
			solarismode = 1;
			memset(ar_tha, 0xff, 6);
			break;
		case 'S': /* ARP sender IP address */
			if (nemesis_name_resolve(optarg, (uint32_t *)ar_spa) < 0) {
				fprintf(stderr, "ERROR: Invalid source IP address: \"%s\".\n", optarg);
				arp_exit(1);
			}
			break;
		case 'v':
			verbose++;
			if (verbose == 1)
				nemesis_printtitle((const char *)title);
			break;
#if defined(ENABLE_PCAPOUTPUT)
		case 'W':
			pcap_output = 1;
			break;
#endif /* ENABLE_PCAPOUTPUT */
#if defined(WIN32)
		case 'Z':
			if ((ifacetmp = pcap_lookupdev(errbuf)) == NULL)
				perror(errbuf);

			PrintDeviceList(ifacetmp);
			arp_exit(1);
#endif
		case '?': /* FALLTHROUGH */
		default:
			arp_usage(argv[0]);
			break;
		}
	}
	argc -= optind;
	argv += optind;
	return;
}

static int arp_exit(int code)
{
	if (got_payload)
		free(pd.file_mem);

	if (file != NULL)
		free(file);

	if (device != NULL)
		free(device);

#if defined(WIN32)
	if (ifacetmp != NULL)
		free(ifacetmp);
#endif

	exit(code);
}

static void arp_verbose(void)
{
	if (verbose) {
		nemesis_printeth(&etherhdr);
		nemesis_printarp(&arphdr);
	}
	return;
}
