#ifndef VERBOSE_HEADER
#define VERBOSE_HEADER

#ifdef VERBOSE
#define verbose(mess, ...) do { \
		printf("[ Verbose ]: "); \
		printf(mess, ##__VA_ARGS__); \
		putc('\n', stdout); \
	} while (0)
#else
#define verbose(...) do { } while (0)
#endif

#ifdef VERY_VERBOSE
#define very_verbose(mess, ...) do { \
		printf("[ Very Verbose ]: "); \
		printf(mess, ##__VA_ARGS__); \
		putc('\n', stdout); \
	} while (0)
#else
#define very_verbose(...) do { } while (0)
#endif

#endif