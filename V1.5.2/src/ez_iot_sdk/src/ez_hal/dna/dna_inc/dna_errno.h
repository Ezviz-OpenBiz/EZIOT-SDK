/*
 *  dna_errno.h -- provide dna-system error code management.
 *  
 *  ORIGINAL AUTHOR: Xu Chun (chun.xu@broadlink.com.cn)
 *
 *  Copyright (c) 2016 Broadlink Corporation
 */

#ifndef __DNA_ERRNO_H
#define __DNA_ERRNO_H

/* Globally unique success and fail code */
#define DNA_SUCCESS 			0
#define DNA_FAIL                1

#define DNA_E_WOULDBLOCK        DNA_E_AGAIN
#define DNA_E_DEADLOCK          DNA_E_DEADLK

enum dna_errno_e {
	DNA_E_PERM          = 1,    /* Operation not permitted */
	DNA_E_NOENT,                /* No such file or directory */
	DNA_E_SRCH,                 /* No such process */
	DNA_E_INTR,                 /* Interrupted system call */
	DNA_E_IO,                   /* I/O error */
	DNA_E_NXIO,                 /* No such device or address */
	DNA_E_2BIG,                 /* Argument list too long */
	DNA_E_NOEXEC,               /* Exec format error */
	DNA_E_BADF,                 /* Bad file number */
	DNA_E_CHILD,                /* No child processes */
	DNA_E_AGAIN,                /* Try again */
	DNA_E_NOMEM,                /* Out of memory */
	DNA_E_ACCES,                /* Permission denied */
	DNA_E_FAULT,                /* Bad address */
	DNA_E_NOTBLK,               /* Block device required */
	DNA_E_BUSY,                 /* Device or resource busy */
	DNA_E_EXIST,                /* File exists */
	DNA_E_XDEV,                 /* Cross-device link */
	DNA_E_NODEV,                /* No such device */
	DNA_E_NOTDIR,               /* Not a directory */
	DNA_E_ISDIR,                /* Is a directory */
	DNA_E_INVAL,                /* Invalid argument */
	DNA_E_NFILE,                /* File table overflow */
	DNA_E_MFILE,                /* Too many open files */
	DNA_E_NOTTY,                /* Not a typewriter */
	DNA_E_TXTBSY,               /* Text file busy */
	DNA_E_FBIG,                 /* File too large */
	DNA_E_NOSPC,                /* No space left on device */
	DNA_E_SPIPE,                /* Illegal seek */
	DNA_E_ROFS,                 /* Read-only file system */
	DNA_E_MLINK,                /* Too many links */
	DNA_E_PIPE,                 /* Broken pipe */
	DNA_E_DOM,                  /* Math argument out of domain of func */
	DNA_E_RANGE,                /* Math result not representable */
    DNA_E_DEADLK,               /* Resource deadlock would occur */
    DNA_E_NAMETOOLONG,          /* File name too long */
    DNA_E_NOLCK,                /* No record locks available */
    DNA_E_NOSYS,                /* Function not implemented */
    DNA_E_NOTEMPTY,             /* Directory not empty */
    DNA_E_LOOP,                 /* Too many symbolic links encountered */
    DNA_E_NOMSG          = 42,  /* No message of desired type */
    DNA_E_IDRM,                 /* Identifier removed */
    DNA_E_CHRNG,                /* Channel number out of range */
    DNA_E_L2NSYNC,              /* Level 2 not synchronized */
    DNA_E_L3HLT,                /* Level 3 halted */
    DNA_E_L3RST,                /* Level 3 reset */
    DNA_E_LNRNG,                /* Link number out of range */
    DNA_E_UNATCH,               /* Protocol driver not attached */
    DNA_E_NOCSI,                /* No CSI structure available */
    DNA_E_L2HLT,                /* Level 2 halted */
    DNA_E_BADE,                 /* Invalid exchange */
    DNA_E_BADR,                 /* Invalid request descriptor */
    DNA_E_XFULL,                /* Exchange full */
    DNA_E_NOANO,                /* No anode */
    DNA_E_BADRQC,               /* Invalid request code */
    DNA_E_BADSLT,               /* Invalid slot */
    DNA_E_BFONT          = 59,  /* Bad font file format */
    DNA_E_NOSTR,                /* Device not a stream */
    DNA_E_NODATA,               /* No data available */
    DNA_E_TIME,                 /* Timer expired */
    DNA_E_NOSR,                 /* Out of streams resources */
    DNA_E_NONET,                /* Machine is not on the network */
    DNA_E_NOPKG,                /* Package not installed */
    DNA_E_REMOTE,               /* Object is remote */
    DNA_E_NOLINK,               /* Link has been severed */
    DNA_E_ADV,                  /* Advertise error */
    DNA_E_SRMNT,                /* Srmount error */
    DNA_E_COMM,                 /* Communication error on send */
    DNA_E_PROTO,                /* Protocol error */
    DNA_E_MULTIHOP,             /* Multihop attempted */
    DNA_E_DOTDOT,               /* RFS specific error */
    DNA_E_BADMSG,               /* Not a data message */
    DNA_E_OVERFLOW,             /* Value too large for defined data type */
    DNA_E_NOTUNIQ,              /* Name not unique on network */
    DNA_E_BADFD,                /* File descriptor in bad state */
    DNA_E_REMCHG,               /* Remote address changed */
    DNA_E_LIBACC,               /* Can not access a needed shared library */
    DNA_E_LIBBAD,               /* Accessing a corrupted shared library */
    DNA_E_LIBSCN,               /* .lib section in a.out corrupted */
    DNA_E_LIBMAX,               /* Attempting to link in too many shared libraries */
    DNA_E_LIBEXEC,              /* Cannot exec a shared library directly */
    DNA_E_ILSEQ,                /* Illegal byte sequence */
    DNA_E_RESTART,              /* Interrupted system call should be restarted */
    DNA_E_STRPIPE,              /* Streams pipe error */
    DNA_E_USERS,                /* Too many users */
    DNA_E_NOTSOCK,              /* Socket operation on non-socket */
    DNA_E_DESTADDRREQ,          /* Destination address required */
    DNA_E_MSGSIZE,              /* Message too long */
    DNA_E_PROTOTYPE,            /* Protocol wrong type for socket */
    DNA_E_NOPROTOOPT,           /* Protocol not available */
    DNA_E_PROTONOSUPPORT,       /* Protocol not supported */
    DNA_E_SOCKTNOSUPPORT,       /* Socket type not supported */
    DNA_E_OPNOTSUPP,            /* Operation not supported on transport endpoint */
    DNA_E_PFNOSUPPORT,          /* Protocol family not supported */
    DNA_E_AFNOSUPPORT,          /* Address family not supported by protocol */
    DNA_E_ADDRINUSE,            /* Address already in use */
    DNA_E_ADDRNOTAVAIL,         /* Cannot assign requested address */
    DNA_E_NETDOWN,              /* Network is down */
    DNA_E_NETUNREACH,           /* Network is unreachable */
    DNA_E_NETRESET,             /* Network dropped connection because of reset */
    DNA_E_CONNABORTED,          /* Software caused connection abort */
    DNA_E_CONNRESET,            /* Connection reset by peer */
    DNA_E_NOBUFS,               /* No buffer space available */
    DNA_E_ISCONN,               /* Transport endpoint is already connected */
    DNA_E_NOTCONN,              /* Transport endpoint is not connected */
    DNA_E_SHUTDOWN,             /* Cannot send after transport endpoint shutdown */
    DNA_E_TOOMANYREFS,          /* Too many references: cannot splice */
    DNA_E_TIMEDOUT,             /* Connection timed out */
    DNA_E_CONNREFUSED,          /* Connection refused */
    DNA_E_HOSTDOWN,             /* Host is down */
    DNA_E_HOSTUNREACH,          /* No route to host */
    DNA_E_ALREADY,              /* Operation already in progress */
    DNA_E_INPROGRESS,           /* Operation now in progress */
    DNA_E_STALE,                /* Stale NFS file handle */
    DNA_E_UCLEAN,               /* Structure needs cleaning */
    DNA_E_NOTNAM,               /* Not a XENIX named type file */
    DNA_E_NAVAIL,               /* No XENIX semaphores available */
    DNA_E_ISNAM,                /* Is a named type file */
    DNA_E_REMOTEIO,             /* Remote I/O error */
    DNA_E_DQUOT,                /* Quota exceeded */
    DNA_E_NOMEDIUM,             /* No medium found */
    DNA_E_MEDIUMTYPE,           /* Wrong medium type */
    DNA_E_CRC,                  /* Error in CRC check */
	DNA_E_UNINIT,               /* Module is not yet initialized */
	DNA_E_SMTCFG_TIMEOUT,       /* Smart config timeout */
	DNA_E_SMTCFG_STOP,          /* Smart config stopped */
#ifdef CONFIG_FASTCON_CLOUD
	DNA_E_SMTCFG_FASTCON,
#endif
#ifdef CONFIG_ANDLINK_CLOUD
	DNA_E_APCFG_STOP,
#endif
};

#endif

