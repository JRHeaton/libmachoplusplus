#define ESWAP16(x) (((x) >> 0x08) | ((x) << 0x08))
#define ESWAP32(x) (((x) >> 0x18) | \
                    (((x) >> 0x08) & 0x0000FF00) | \
                    (((x) << 0x08) & 0x00FF0000) | \
                    ((x) << 0x18))
#define ESWAP64(x) (((x) >> 0x38) | \
                    (((x) >> 0x28) & 0x000000000000FF00) |\
                    (((x) >> 0x18) & 0x0000000000FF0000) |\
                    (((x) >> 0x08) & 0x00000000FF000000) |\
                    (((x) << 0x08) & 0x000000FF00000000) |\
                    (((x) << 0x18) & 0x0000FF0000000000) |\
                    (((x) << 0x28) & 0x00FF000000000000) |\
                    ((x) << 0x38))

#define ENABLE_FLAG(field, pIndex) (field |= (pIndex))
#define DISABLE_FLAG(field, pIndex) (field &= ~(pIndex))
#define CHECK_FLAG(field, pIndex) (field & (pIndex))

#define ENABLE_FLAG_I(field, index) ENABLE_FLAG(field, (1 << index))
#define DISABLE_FLAG_I(field, index) DISABLE_FLAG(field, (1 << index))
#define CHECK_FLAG_I(field, index) CHECK_FLAG(field, (1 << index))

