#ifndef __ENDIANESS_H
#define __ENDIANESS_H

#include <stddef.h>
#include <stdint.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"

/**
 *  @brief Put a 16-bit integer as big-endian to arbitrary location.
 *
 *  Put a 16-bit integer, originally in host endianness, to a
 *  potentially unaligned memory location in big-endian format.
 *
 *  @param val 16-bit integer in host endianness.
 *  @param dst Destination memory address to store the result.
 */
static inline void put_be16(uint16_t val, uint8_t dst[2]) {
    dst[0] = val >> 8;
    dst[1] = val;
}

/**
 *  @brief Put a 24-bit integer as big-endian to arbitrary location.
 *
 *  Put a 24-bit integer, originally in host endianness, to a
 *  potentially unaligned memory location in big-endian format.
 *
 *  @param val 24-bit integer in host endianness.
 *  @param dst Destination memory address to store the result.
 */
static inline void put_be24(uint32_t val, uint8_t dst[3]) {
    dst[0] = val >> 16;
    put_be16(val, &dst[1]);
}

/**
 *  @brief Put a 32-bit integer as big-endian to arbitrary location.
 *
 *  Put a 32-bit integer, originally in host endianness, to a
 *  potentially unaligned memory location in big-endian format.
 *
 *  @param val 32-bit integer in host endianness.
 *  @param dst Destination memory address to store the result.
 */
static inline void put_be32(uint32_t val, uint8_t dst[4]) {
    put_be16(val >> 16, dst);
    put_be16(val, &dst[2]);
}

/**
 *  @brief Put a 48-bit integer as big-endian to arbitrary location.
 *
 *  Put a 48-bit integer, originally in host endianness, to a
 *  potentially unaligned memory location in big-endian format.
 *
 *  @param val 48-bit integer in host endianness.
 *  @param dst Destination memory address to store the result.
 */
static inline void put_be48(uint64_t val, uint8_t dst[6]) {
    put_be16(val >> 32, dst);
    put_be32(val, &dst[2]);
}

/**
 *  @brief Put a 64-bit integer as big-endian to arbitrary location.
 *
 *  Put a 64-bit integer, originally in host endianness, to a
 *  potentially unaligned memory location in big-endian format.
 *
 *  @param val 64-bit integer in host endianness.
 *  @param dst Destination memory address to store the result.
 */
static inline void put_be64(uint64_t val, uint8_t dst[8]) {
    put_be32(val >> 32, dst);
    put_be32(val, &dst[4]);
}

/**
 *  @brief Put a 16-bit integer as little-endian to arbitrary location.
 *
 *  Put a 16-bit integer, originally in host endianness, to a
 *  potentially unaligned memory location in little-endian format.
 *
 *  @param val 16-bit integer in host endianness.
 *  @param dst Destination memory address to store the result.
 */
static inline void put_le16(uint16_t val, uint8_t dst[2]) {
    dst[0] = val;
    dst[1] = val >> 8;
}

/**
 *  @brief Put a 24-bit integer as little-endian to arbitrary location.
 *
 *  Put a 24-bit integer, originally in host endianness, to a
 *  potentially unaligned memory location in littel-endian format.
 *
 *  @param val 24-bit integer in host endianness.
 *  @param dst Destination memory address to store the result.
 */
static inline void put_le24(uint32_t val, uint8_t dst[3]) {
    put_le16(val, dst);
    dst[2] = val >> 16;
}

/**
 *  @brief Put a 32-bit integer as little-endian to arbitrary location.
 *
 *  Put a 32-bit integer, originally in host endianness, to a
 *  potentially unaligned memory location in little-endian format.
 *
 *  @param val 32-bit integer in host endianness.
 *  @param dst Destination memory address to store the result.
 */
static inline void put_le32(uint32_t val, uint8_t dst[4]) {
    put_le16(val, dst);
    put_le16(val >> 16, &dst[2]);
}

/**
 *  @brief Put a 48-bit integer as little-endian to arbitrary location.
 *
 *  Put a 48-bit integer, originally in host endianness, to a
 *  potentially unaligned memory location in little-endian format.
 *
 *  @param val 48-bit integer in host endianness.
 *  @param dst Destination memory address to store the result.
 */
static inline void put_le48(uint64_t val, uint8_t dst[6]) {
    put_le32(val, dst);
    put_le16(val >> 32, &dst[4]);
}

/**
 *  @brief Put a 64-bit integer as little-endian to arbitrary location.
 *
 *  Put a 64-bit integer, originally in host endianness, to a
 *  potentially unaligned memory location in little-endian format.
 *
 *  @param val 64-bit integer in host endianness.
 *  @param dst Destination memory address to store the result.
 */
static inline void put_le64(uint64_t val, uint8_t dst[8]) {
    put_le32(val, dst);
    put_le32(val >> 32, &dst[4]);
}

/**
 *  @brief Get a 16-bit integer stored in big-endian format.
 *
 *  Get a 16-bit integer, stored in big-endian format in a potentially
 *  unaligned memory location, and convert it to the host endianness.
 *
 *  @param src Location of the big-endian 16-bit integer to get.
 *
 *  @return 16-bit integer in host endianness.
 */
static inline uint16_t get_be16(const uint8_t src[2]) {
    return ((uint16_t)src[0] << 8) | src[1];
}

/**
 *  @brief Get a 24-bit integer stored in big-endian format.
 *
 *  Get a 24-bit integer, stored in big-endian format in a potentially
 *  unaligned memory location, and convert it to the host endianness.
 *
 *  @param src Location of the big-endian 24-bit integer to get.
 *
 *  @return 24-bit integer in host endianness.
 */
static inline uint32_t get_be24(const uint8_t src[3]) {
    return ((uint32_t)src[0] << 16) | get_be16(&src[1]);
}

/**
 *  @brief Get a 32-bit integer stored in big-endian format.
 *
 *  Get a 32-bit integer, stored in big-endian format in a potentially
 *  unaligned memory location, and convert it to the host endianness.
 *
 *  @param src Location of the big-endian 32-bit integer to get.
 *
 *  @return 32-bit integer in host endianness.
 */
static inline uint32_t get_be32(const uint8_t src[4]) {
    return ((uint32_t)get_be16(&src[0]) << 16) | get_be16(&src[2]);
}

/**
 *  @brief Get a 48-bit integer stored in big-endian format.
 *
 *  Get a 48-bit integer, stored in big-endian format in a potentially
 *  unaligned memory location, and convert it to the host endianness.
 *
 *  @param src Location of the big-endian 48-bit integer to get.
 *
 *  @return 48-bit integer in host endianness.
 */
static inline uint64_t get_be48(const uint8_t src[6]) {
    return ((uint64_t)get_be32(&src[0]) << 16) | get_be16(&src[4]);
}

/**
 *  @brief Get a 64-bit integer stored in big-endian format.
 *
 *  Get a 64-bit integer, stored in big-endian format in a potentially
 *  unaligned memory location, and convert it to the host endianness.
 *
 *  @param src Location of the big-endian 64-bit integer to get.
 *
 *  @return 64-bit integer in host endianness.
 */
static inline uint64_t get_be64(const uint8_t src[8]) {
    return ((uint64_t)get_be32(&src[0]) << 32) | get_be32(&src[4]);
}

/**
 *  @brief Get a 16-bit integer stored in little-endian format.
 *
 *  Get a 16-bit integer, stored in little-endian format in a potentially
 *  unaligned memory location, and convert it to the host endianness.
 *
 *  @param src Location of the little-endian 16-bit integer to get.
 *
 *  @return 16-bit integer in host endianness.
 */
static inline uint16_t get_le16(const uint8_t src[2]) {
    return ((uint16_t)src[1] << 8) | src[0];
}

/**
 *  @brief Get a 24-bit integer stored in big-endian format.
 *
 *  Get a 24-bit integer, stored in big-endian format in a potentially
 *  unaligned memory location, and convert it to the host endianness.
 *
 *  @param src Location of the big-endian 24-bit integer to get.
 *
 *  @return 24-bit integer in host endianness.
 */
static inline uint32_t get_le24(const uint8_t src[3]) {
    return ((uint32_t)src[2] << 16) | get_le16(&src[0]);
}

/**
 *  @brief Get a 32-bit integer stored in little-endian format.
 *
 *  Get a 32-bit integer, stored in little-endian format in a potentially
 *  unaligned memory location, and convert it to the host endianness.
 *
 *  @param src Location of the little-endian 32-bit integer to get.
 *
 *  @return 32-bit integer in host endianness.
 */
static inline uint32_t get_le32(const uint8_t src[4]) {
    return ((uint32_t)get_le16(&src[2]) << 16) | get_le16(&src[0]);
}

/**
 *  @brief Get a 48-bit integer stored in little-endian format.
 *
 *  Get a 48-bit integer, stored in little-endian format in a potentially
 *  unaligned memory location, and convert it to the host endianness.
 *
 *  @param src Location of the little-endian 48-bit integer to get.
 *
 *  @return 48-bit integer in host endianness.
 */
static inline uint64_t get_le48(const uint8_t src[6]) {
    return ((uint64_t)get_le32(&src[2]) << 16) | get_le16(&src[0]);
}

/**
 *  @brief Get a 64-bit integer stored in little-endian format.
 *
 *  Get a 64-bit integer, stored in little-endian format in a potentially
 *  unaligned memory location, and convert it to the host endianness.
 *
 *  @param src Location of the little-endian 64-bit integer to get.
 *
 *  @return 64-bit integer in host endianness.
 */
static inline uint64_t get_le64(const uint8_t src[8]) {
    return ((uint64_t)get_le32(&src[4]) << 32) | get_le32(&src[0]);
}

#pragma GCC diagnostic pop

#endif /* __ENDIANESS_H */
