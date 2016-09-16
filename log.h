#ifndef  _LOG_H
#define  _LOG_H
#include <syslog.h>

// Uncomment to log debug entries
#define DEBUG

#ifdef DEBUG
#define PLOG(cat, args...) g_print(##args)
#define PLOGLINE(cat, fmt, args...) g_print("%s:%d:" fmt"\n", __FILE__, __LINE__, ##args)
#define PDEBUG(fmt, args...) PLOGLINE(LOG_DEBUG, fmt, ##args)
#else
#define PLOG(cat, args...)  do { syslog(cat, ##args); } while(0)
#define PLOGLINE(cat, fmt, args...)  do { syslog(cat, "%s:%d: " fmt, __FILE__, __LINE__, ##args); } while(0)
#define PDEBUG(Args...)
#endif

#define PCRIT(fmt, args...) PLOGLINE(LOG_CRIT, fmt, ##args)
#define PERROR(fmt, args...) PLOGLINE(LOG_ERR, fmt, ##args)
#define PWARN(fmt, args...) PLOGLINE(LOG_WARNING, fmt, ##args)
#define PINFO(fmt, args...) PLOG(LOG_INFO, fmt, ##args)

#endif /* _LOG_H */
