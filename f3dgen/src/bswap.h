#ifndef BSWAP_H
#define BSWAP_H

#define bswap32(n) ( \
	(((unsigned int)n & 0xFF000000) >> 24) | \
	(((unsigned int)n & 0x00FF0000) >>  8) | \
	(((unsigned int)n & 0x0000FF00) <<  8) | \
	(((unsigned int)n & 0x000000FF) << 24)   \
)

#define bswap16(n) ( \
	(((unsigned short)n & 0xFF00) >> 8) | \
	(((unsigned short)n & 0x00FF) << 8)   \
)

#endif // BSWAP_H