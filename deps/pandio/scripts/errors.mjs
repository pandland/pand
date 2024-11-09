import { writeFileSync } from "fs";

const errcodes = {
  EPERM: { code: 1, message: "Operation not permitted" },
  ENOENT: { code: 2, message: "No such file or directory" },
  ESRCH: { code: 3, message: "No such process" },
  EINTR: { code: 4, message: "Interrupted system call" },
  EIO: { code: 5, message: "I/O error" },
  ENXIO: { code: 6, message: "No such device or address" },
  E2BIG: { code: 7, message: "Argument list too long" },
  ENOEXEC: { code: 8, message: "Exec format error" },
  EBADF: { code: 9, message: "Bad file descriptor" },
  ECHILD: { code: 10, message: "No child processes" },
  EAGAIN: { code: 11, message: "Try again" },
  ENOMEM: { code: 12, message: "Out of memory" },
  EACCES: { code: 13, message: "Permission denied" },
  EFAULT: { code: 14, message: "Bad address" },
  ENOTBLK: { code: 15, message: "Block device required" },
  EBUSY: { code: 16, message: "Device or resource busy" },
  EEXIST: { code: 17, message: "File exists" },
  EXDEV: { code: 18, message: "Cross-device link" },
  ENODEV: { code: 19, message: "No such device" },
  ENOTDIR: { code: 20, message: "Not a directory" },
  EISDIR: { code: 21, message: "Is a directory" },
  EINVAL: { code: 22, message: "Invalid argument" },
  ENFILE: { code: 23, message: "File table overflow" },
  EMFILE: { code: 24, message: "Too many open files" },
  ENOTTY: { code: 25, message: "Not a typewriter" },
  ETXTBSY: { code: 26, message: "Text file busy" },
  EFBIG: { code: 27, message: "File too large" },
  ENOSPC: { code: 28, message: "No space left on device" },
  ESPIPE: { code: 29, message: "Illegal seek" },
  EROFS: { code: 30, message: "Read-only file system" },
  EMLINK: { code: 31, message: "Too many links" },
  EPIPE: { code: 32, message: "Broken pipe" },
  EDOM: { code: 33, message: "Math argument out of domain of func" },
  ERANGE: { code: 34, message: "Math result not representable" },
  EDEADLK: { code: 35, message: "Resource deadlock would occur" },
  ENAMETOOLONG: { code: 36, message: "File name too long" },
  ENOLCK: { code: 37, message: "No record locks available" },
  ENOSYS: { code: 38, message: "Function not implemented" },
  ENOTEMPTY: { code: 39, message: "Directory not empty" },
  ELOOP: { code: 40, message: "Too many symbolic links encountered" },
  EWOULDBLOCK: { code: 11, message: "Operation would block" },
  ENOMSG: { code: 42, message: "No message of desired type" },
  EIDRM: { code: 43, message: "Identifier removed" },
  ECHRNG: { code: 44, message: "Channel number out of range" },
  EL2NSYNC: { code: 45, message: "Level 2 not synchronized" },
  EL3HLT: { code: 46, message: "Level 3 halted" },
  EL3RST: { code: 47, message: "Level 3 reset" },
  ELNRNG: { code: 48, message: "Link number out of range" },
  EUNATCH: { code: 49, message: "Protocol driver not attached" },
  ENOCSI: { code: 50, message: "No CSI structure available" },
  EL2HLT: { code: 51, message: "Level 2 halted" },
  EBADE: { code: 52, message: "Invalid exchange" },
  EBADR: { code: 53, message: "Invalid request descriptor" },
  EXFULL: { code: 54, message: "Exchange full" },
  ENOANO: { code: 55, message: "No anode" },
  EBADRQC: { code: 56, message: "Invalid request code" },
  EBADSLT: { code: 57, message: "Invalid slot" },
  EDEADLOCK: { code: 35, message: "Resource deadlock would occur" },
  EBFONT: { code: 59, message: "Bad font file format" },
  ENOSTR: { code: 60, message: "Device not a stream" },
  ENODATA: { code: 61, message: "No data available" },
  ETIME: { code: 62, message: "Timer expired" },
  ENOSR: { code: 63, message: "Out of streams resources" },
  ENONET: { code: 64, message: "Machine is not on the network" },
  ENOPKG: { code: 65, message: "Package not installed" },
  EREMOTE: { code: 66, message: "Object is remote" },
  ENOLINK: { code: 67, message: "Link has been severed" },
  EADV: { code: 68, message: "Advertise error" },
  ESRMNT: { code: 69, message: "Srmount error" },
  ECOMM: { code: 70, message: "Communication error on send" },
  EPROTO: { code: 71, message: "Protocol error" },
  EMULTIHOP: { code: 72, message: "Multihop attempted" },
  EDOTDOT: { code: 73, message: "RFS specific error" },
  EBADMSG: { code: 74, message: "Not a data message" },
  EOVERFLOW: { code: 75, message: "Value too large for defined data type" },
  ENOTUNIQ: { code: 76, message: "Name not unique on network" },
  EBADFD: { code: 77, message: "File descriptor in bad state" },
  EREMCHG: { code: 78, message: "Remote address changed" },
  ELIBACC: { code: 79, message: "Can not access a needed shared library" },
  ELIBBAD: { code: 80, message: "Accessing a corrupted shared library" },
  ELIBSCN: { code: 81, message: ".lib section in a.out corrupted" },
  ELIBMAX: { code: 82, message: "Attempting to link in too many shared libraries" },
  ELIBEXEC: { code: 83, message: "Cannot exec a shared library directly" },
  EILSEQ: { code: 84, message: "Illegal byte sequence" },
  ERESTART: { code: 85, message: "Interrupted system call should be restarted" },
  ESTRPIPE: { code: 86, message: "Streams pipe error" },
  EUSERS: { code: 87, message: "Too many users" },
  ENOTSOCK: { code: 88, message: "Socket operation on non-socket" },
  EDESTADDRREQ: { code: 89, message: "Destination address required" },
  EMSGSIZE: { code: 90, message: "Message too long" },
  EPROTOTYPE: { code: 91, message: "Protocol wrong type for socket" },
  ENOPROTOOPT: { code: 92, message: "Protocol not available" },
  EPROTONOSUPPORT: { code: 93, message: "Protocol not supported" },
  ESOCKTNOSUPPORT: { code: 94, message: "Socket type not supported" },
  EOPNOTSUPP: { code: 95, message: "Operation not supported on transport endpoint" },
  ENOTSUP: { code: 95, message: "Operation not supported on transport endpoint" },
  EPFNOSUPPORT: { code: 96, message: "Protocol family not supported" },
  EAFNOSUPPORT: { code: 97, message: "Address family not supported by protocol" },
  EADDRINUSE: { code: 98, message: "Address already in use" },
  EADDRNOTAVAIL: { code: 99, message: "Cannot assign requested address" },
  ENETDOWN: { code: 100, message: "Network is down" },
  ENETUNREACH: { code: 101, message: "Network is unreachable" },
  ENETRESET: { code: 102, message: "Network dropped connection because of reset" },
  ECONNABORTED: { code: 103, message: "Software caused connection abort" },
  ECONNRESET: { code: 104, message: "Connection reset by peer" },
  ENOBUFS: { code: 105, message: "No buffer space available" },
  EISCONN: { code: 106, message: "Transport endpoint is already connected" },
  ENOTCONN: { code: 107, message: "Transport endpoint is not connected" },
  ESHUTDOWN: { code: 108, message: "Cannot send after transport endpoint shutdown" },
  ETOOMANYREFS: { code: 109, message: "Too many references: cannot splice" },
  ETIMEDOUT: { code: 110, message: "Connection timed out" },
  ECONNREFUSED: { code: 111, message: "Connection refused" },
  EHOSTDOWN: { code: 112, message: "Host is down" },
  EHOSTUNREACH: { code: 113, message: "No route to host" },
  EALREADY: { code: 114, message: "Operation already in progress" },
  EINPROGRESS: { code: 115, message: "Operation now in progress" },
  ESTALE: { code: 116, message: "Stale file handle" },
  EUCLEAN: { code: 117, message: "Structure needs cleaning" },
  ENOTNAM: { code: 118, message: "Not a XENIX named type file" },
  ENAVAIL: { code: 119, message: "No XENIX semaphores available" },
  EISNAM: { code: 120, message: "Is a named type file" },
  EREMOTEIO: { code: 121, message: "Remote I/O error" },
  EDQUOT: { code: 122, message: "Quota exceeded" },
  ENOMEDIUM: { code: 123, message: "No medium found" },
  EMEDIUMTYPE: { code: 124, message: "Wrong medium type" },
  ECANCELED: { code: 125, message: "Operation Canceled" },
  ENOKEY: { code: 126, message: "Required key not available" },
  EKEYEXPIRED: { code: 127, message: "Key has expired" },
  EKEYREVOKED: { code: 128, message: "Key has been revoked" },
  EKEYREJECTED: { code: 129, message: "Key was rejected by service" },
  EOWNERDEAD: { code: 130, message: "Owner died" },
  ENOTRECOVERABLE: { code: 131, message: "State not recoverable" },
  ERFKILL: { code: 132, message: "Operation not possible due to RF-kill" },
  EHWPOISON: { code: 133, message: "Memory page has hardware error" },
};

const pandioExtension = {
  OK: { code: 0, message: "Success" },
  UNKNOWN: { code: -7000, message: "Unknown error" },
  EOF: { code: -7002, message: "End of the stream" },
};

let file = "/* Auto generated file */\n#pragma once\n#include <errno.h>\n";

file += "\n/* PANDIO custom errors: */\n";
for (const [name, { code }] of Object.entries(pandioExtension)) {
  file += `#define PD_${name} (${code})\n`;
}

file += "\n/* Unix system error codes: */";

const FALLBACK_BASE = 3000;

for (const [name, { code }] of Object.entries(errcodes)) {
  file += `\n#ifdef ${name}\n#define PD_${name} (-${name})\n#else\n#define PD_${name} (-${FALLBACK_BASE + code})\n#endif\n`;
}

file += "\n\n#define PD_ERR_STR_MAPPING(X)       \\\n";

let nameMapping = "\n\n#define PD_ERR_NAME_MAPPING(X)   \\\n";

let lastValue;
for (const [name, { message, code }] of Object.entries(errcodes)) {
  // deduplication
  if (lastValue && code <= lastValue)
    continue;

  file += `   X(PD_${name}, "${message}")       \\\n`;
  nameMapping += `   X(PD_${name}, "${name}")       \\\n`;
  lastValue = code;
}

const maxIndex = Object.entries(pandioExtension).length - 1;
Object.entries(pandioExtension).forEach(([name, { message }], index) => {
  file += `   X(PD_${name}, "${message}")${ index === maxIndex ? "" : "       \\" }\n`;
  nameMapping += `   X(PD_${name}, "${name}")${ index === maxIndex ? "" : "       \\" }\n`;
});

file += nameMapping;

writeFileSync("./include/pandio/err.h", file, { encoding: "utf8" });
