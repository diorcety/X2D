
#ifndef STRUCT_H_
#define STRUCT_H_

#include <stdbool.h>
#include <stdint.h>

#define STRUCT_STATIC_INLINE static inline

#define STRUCT_LE(A) L##A
#define STRUCT_BE(A) B##A

#define STRUCT_TYPE_SIZE_LS8 1
#define STRUCT_TYPE_SIZE_BS8 1
#define STRUCT_TYPE_SIZE_LU8 1
#define STRUCT_TYPE_SIZE_BU8 1
#define STRUCT_TYPE_SIZE_LS16 2
#define STRUCT_TYPE_SIZE_BS16 2
#define STRUCT_TYPE_SIZE_LU16 2
#define STRUCT_TYPE_SIZE_BU16 2
#define STRUCT_TYPE_SIZE_LS24 3
#define STRUCT_TYPE_SIZE_BS24 3
#define STRUCT_TYPE_SIZE_LU24 3
#define STRUCT_TYPE_SIZE_BU24 3
#define STRUCT_TYPE_SIZE_LS32 4
#define STRUCT_TYPE_SIZE_BS32 4
#define STRUCT_TYPE_SIZE_LU32 4
#define STRUCT_TYPE_SIZE_BU32 4
#define STRUCT_TYPE_SIZE_LS64 8
#define STRUCT_TYPE_SIZE_BS64 8
#define STRUCT_TYPE_SIZE_LU64 8
#define STRUCT_TYPE_SIZE_BU64 8
#define STRUCT_TYPE_SIZE_(X) STRUCT_TYPE_SIZE_##X
#define STRUCT_TYPE_SIZE(X) STRUCT_TYPE_SIZE_(X)

#define STRUCT_TYPE_TO_STD_LU64 uint64_t
#define STRUCT_TYPE_TO_STD_BU64 uint64_t
#define STRUCT_TYPE_TO_STD_LS64 int64_t
#define STRUCT_TYPE_TO_STD_BS64 int64_t
#define STRUCT_TYPE_TO_STD_LU32 uint32_t
#define STRUCT_TYPE_TO_STD_BU32 uint32_t
#define STRUCT_TYPE_TO_STD_LS32 int32_t
#define STRUCT_TYPE_TO_STD_BS32 int32_t
#define STRUCT_TYPE_TO_STD_LU24 uint32_t
#define STRUCT_TYPE_TO_STD_BU24 uint32_t
#define STRUCT_TYPE_TO_STD_LS24 int32_t
#define STRUCT_TYPE_TO_STD_BS24 int32_t
#define STRUCT_TYPE_TO_STD_LU16 uint16_t
#define STRUCT_TYPE_TO_STD_BU16 uint16_t
#define STRUCT_TYPE_TO_STD_LS16 int16_t
#define STRUCT_TYPE_TO_STD_BS16 int16_t
#define STRUCT_TYPE_TO_STD_LU8 uint8_t
#define STRUCT_TYPE_TO_STD_BU8 uint8_t
#define STRUCT_TYPE_TO_STD_LS8 int8_t
#define STRUCT_TYPE_TO_STD_BS8 int8_t
#define STRUCT_TYPE_TO_STD_(X) STRUCT_TYPE_TO_STD_##X
#define STRUCT_TYPE_TO_STD(X) STRUCT_TYPE_TO_STD_(X)

#define STRUCT_TYPE_TO_STD_UPPER_LU64 UNS64
#define STRUCT_TYPE_TO_STD_UPPER_BU64 UNS64
#define STRUCT_TYPE_TO_STD_UPPER_LS64 S64
#define STRUCT_TYPE_TO_STD_UPPER_BS64 S64
#define STRUCT_TYPE_TO_STD_UPPER_LU32 UNS32
#define STRUCT_TYPE_TO_STD_UPPER_BU32 UNS32
#define STRUCT_TYPE_TO_STD_UPPER_LS32 S32
#define STRUCT_TYPE_TO_STD_UPPER_BS32 S32
#define STRUCT_TYPE_TO_STD_UPPER_LU24 UNS24
#define STRUCT_TYPE_TO_STD_UPPER_BU24 UNS24
#define STRUCT_TYPE_TO_STD_UPPER_LS24 S24
#define STRUCT_TYPE_TO_STD_UPPER_BS24 S24
#define STRUCT_TYPE_TO_STD_UPPER_LU16 UNS16
#define STRUCT_TYPE_TO_STD_UPPER_BU16 UNS16
#define STRUCT_TYPE_TO_STD_UPPER_LS16 S16
#define STRUCT_TYPE_TO_STD_UPPER_BS16 S16
#define STRUCT_TYPE_TO_STD_UPPER_LU8 UNS8
#define STRUCT_TYPE_TO_STD_UPPER_BU8 UNS8
#define STRUCT_TYPE_TO_STD_UPPER_LS8 S8
#define STRUCT_TYPE_TO_STD_UPPER_BS8 S8
#define STRUCT_TYPE_TO_STD_UPPER_(X) STRUCT_TYPE_TO_STD_UPPER_##X
#define STRUCT_TYPE_TO_STD_UPPER(X) STRUCT_TYPE_TO_STD_UPPER_(X)

#define STRUCT_FROM_FCT_NAME__(A, B) A##_TO_##B
#define STRUCT_FROM_FCT_NAME_(A, B) STRUCT_FROM_FCT_NAME__(A, B)
#define STRUCT_FROM_FCT_NAME(A) STRUCT_FROM_FCT_NAME_(A, STRUCT_TYPE_TO_STD_UPPER(A))
#define STRUCT_TO_FCT_NAME__(A, B) B##_TO_##A
#define STRUCT_TO_FCT_NAME_(A, B) STRUCT_TO_FCT_NAME__(A, B)
#define STRUCT_TO_FCT_NAME(A) STRUCT_TO_FCT_NAME_(A, STRUCT_TYPE_TO_STD_UPPER(A))

#if defined(__GNUC__) || defined(__XC8__)
#define PACKED_STRUCT_BEG(...)               \
    struct __attribute((packed)) __VA_ARGS__ \
    {
#define PACKED_STRUCT_END(...) \
    }                          \
    __VA_ARGS__;
#elif defined(_MSC_VER)
#define PACKED_STRUCT_BEG(...)                 \
    __pragma(pack(push, 1)) struct __VA_ARGS__ \
    {
#define PACKED_STRUCT_END(...) \
    }                          \
    __VA_ARGS__;               \
    __pragma(pack(pop))
#elif defined(SWIG)
#define PACKED_STRUCT_BEG(...) \
    struct __VA_ARGS__         \
    {
#define PACKED_STRUCT_END(...) \
    }                          \
    __VA_ARGS__;
#else
#error Compiler not supported
#endif

#if defined(__GNUC__) || defined(__XC8__)
#define PACKED_UNION_BEG(...)   \
    union __attribute((packed)) \
    {
#define PACKED_UNION_END(...) \
    }                         \
    __VA_ARGS__;
#elif defined(_MSC_VER)
#define PACKED_UNION_BEG(...)     \
    __pragma(pack(push, 1)) union \
    {
#define PACKED_UNION_END(...) \
    }                         \
    __VA_ARGS__;              \
    __pragma(pack(pop))
#elif defined(SWIG)
#define PACKED_UNION_BEG(...) \
    union                     \
    {
#define PACKED_UNION_END(...) \
    }                         \
    __VA_ARGS__;
#else
#error Compiler not supported
#endif

#define TYPEDEF_PACKED_STRUCT_BEG(N) typedef PACKED_STRUCT_BEG(_##N)
#define TYPEDEF_PACKED_STRUCT_END(N) PACKED_STRUCT_END(N)

#if defined(__cplusplus) || (defined(_MSC_VER) && !defined(_MPC_))
#define _TYPEDEF_ENUM_BEG(N, S) \
    typedef enum _##N : S       \
    {
#define _TYPEDEF_ENUM_END(N, S) \
    }                           \
    N;
#define _ENUM_VAR(N, M, S) N M;
#else
#define _TYPEDEF_ENUM_BEG(N, S) \
    typedef enum _##N           \
    {
#define _TYPEDEF_ENUM_END(N, S) \
    }                           \
    N;
#ifdef _MPC_
#define _ENUM_VAR(N, M, S) N M;
#else
#define _ENUM_VAR(N, M, S) N M : S;
#endif
#endif

#define TYPEDEF_ENUM_BEG(N) _TYPEDEF_ENUM_BEG(N, uint8_t)
#define TYPEDEF_ENUM_END(N) _TYPEDEF_ENUM_END(N, uint8_t)
#define ENUM_VAR(N, M) _ENUM_VAR(N, M, 8U)

#define VAR(N, M) N M;

#define _TYPEDEF_TYPE(N)                 \
    TYPEDEF_PACKED_STRUCT_BEG(N)         \
    uint8_t __data[STRUCT_TYPE_SIZE(N)]; \
    TYPEDEF_PACKED_STRUCT_END(N)
_TYPEDEF_TYPE(STRUCT_LE(U64))
_TYPEDEF_TYPE(STRUCT_BE(U64))
_TYPEDEF_TYPE(STRUCT_LE(S64))
_TYPEDEF_TYPE(STRUCT_BE(S64))
_TYPEDEF_TYPE(STRUCT_LE(U32))
_TYPEDEF_TYPE(STRUCT_BE(U32))
_TYPEDEF_TYPE(STRUCT_LE(S32))
_TYPEDEF_TYPE(STRUCT_BE(S32))
_TYPEDEF_TYPE(STRUCT_LE(U24))
_TYPEDEF_TYPE(STRUCT_BE(U24))
_TYPEDEF_TYPE(STRUCT_LE(S24))
_TYPEDEF_TYPE(STRUCT_BE(S24))
_TYPEDEF_TYPE(STRUCT_LE(U16))
_TYPEDEF_TYPE(STRUCT_BE(U16))
_TYPEDEF_TYPE(STRUCT_LE(S16))
_TYPEDEF_TYPE(STRUCT_BE(S16))
_TYPEDEF_TYPE(STRUCT_LE(U8))
_TYPEDEF_TYPE(STRUCT_BE(U8))
_TYPEDEF_TYPE(STRUCT_LE(S8))
_TYPEDEF_TYPE(STRUCT_BE(S8))
#undef _TYPEDEF_TYPE

#define TYPEDEF_BITFIELD_BEG(N, T) _TYPEDEF_ENUM_BEG(N, uint8_t)
#define TYPEDEF_BITFIELD_END(N, T)                                                 \
    _TYPEDEF_ENUM_END(N, uint8_t)                                                  \
    TYPEDEF_PACKED_STRUCT_BEG(STRUCT_BF_##N)                                       \
    T __value;                                                                     \
    TYPEDEF_PACKED_STRUCT_END(STRUCT_BF_##N)                                       \
    typedef STRUCT_TYPE_TO_STD(T) STRUCT_BF_##N##_NT;                              \
    STRUCT_STATIC_INLINE STRUCT_BF_##N##_NT STRUCT_BF_##N##_TO_LE(STRUCT_BF_##N x) \
    {                                                                              \
        return STRUCT_FROM_FCT_NAME(T)(x.__value);                                 \
    }                                                                              \
    STRUCT_STATIC_INLINE STRUCT_BF_##N LE_TO_##STRUCT_BF_##N(STRUCT_BF_##N##_NT x) \
    {                                                                              \
        STRUCT_BF_##N ret;                                                         \
        ret.__value = STRUCT_TO_FCT_NAME(T)(x);                                    \
        return ret;                                                                \
    }

#define BITFIELD_BIT_SHIFT(N, S) (((STRUCT_BF_##N##_NT)1U) << ((uint8_t)S))
#define BITFIELD_RANGE_LMASK(N, O) ((STRUCT_BF_##N##_NT)(((STRUCT_BF_##N##_NT)(~((STRUCT_BF_##N##_NT)0U))) << (O)))
#define BITFIELD_RANGE_RMASK(N, O) ((STRUCT_BF_##N##_NT)(((STRUCT_BF_##N##_NT)(~((STRUCT_BF_##N##_NT)0U))) >> (O)))
#define BITFIELD_RANGE_MASK(N, H, L) \
    (BITFIELD_RANGE_RMASK(N, ((sizeof(STRUCT_BF_##N##_NT) * 8) - (uint8_t)H - 1U)) & BITFIELD_RANGE_LMASK(N, ((uint8_t)L)))

#define BITFIELD_SET(N, V, X) (V = LE_TO_##STRUCT_BF_##N(X))
#define BITFIELD_GET(N, V) (STRUCT_BF_##N##_TO_LE(V))
#define BITFIELD_GET_RANGE(N, V, H, L) ((BITFIELD_GET(N, V) & BITFIELD_RANGE_MASK(N, H, L)) >> ((uint8_t)L))
#define BITFIELD_SET_RANGE(N, V, H, L, X) \
    (BITFIELD_SET(N, V,                   \
                  ((BITFIELD_GET(N, V) & ~BITFIELD_RANGE_MASK(N, H, L))) | ((((STRUCT_BF_##N##_NT)(X)) << ((uint8_t)L)) & BITFIELD_RANGE_MASK(N, H, L))))

#define BITFIELD_BIT_IS_SET(N, V, B) ((BITFIELD_GET(N, V) & BITFIELD_BIT_SHIFT(N, B)) != 0U ? true : false)
#define BITFIELD_BIT_SET(N, V, B) (BITFIELD_SET(N, V, (BITFIELD_GET(N, V) | BITFIELD_BIT_SHIFT(N, B))))
#define BITFIELD_BIT_CLR(N, V, B) \
    (BITFIELD_SET(N, V, (BITFIELD_GET(N, V) & (STRUCT_BF_##N##_NT)(~BITFIELD_BIT_SHIFT(N, B)))))
#define BITFIELD_BIT_TOGGLE(N, V, B) (BITFIELD_SET(N, V, (BITFIELD_GET(N, V) ^ BITFIELD_BIT_SHIFT(N, B))))
#define BITFIELD_BIT_SET_CLR(C, N, V, B) ((C) ? BITFIELD_BIT_SET(N, V, B) : BITFIELD_BIT_CLR(N, V, B))
#define BITFIELD_TYPE(N) STRUCT_BF_##N
#define BITFIELD_VAR(N, M) BITFIELD_TYPE(N) M;

//
// Define conversion functions
//

#define STRUCT_FROM_FCT(A) STRUCT_TYPE_TO_STD(A) STRUCT_FROM_FCT_NAME(A)(A x)
#define STRUCT_TO_FCT(A) A STRUCT_TO_FCT_NAME(A)(STRUCT_TYPE_TO_STD(A) x)

#define STRUCT_FROM_(X) STRUCT_FROM_FCT(X)
#define STRUCT_TO_(X) STRUCT_TO_FCT(X)

#define STRUCT_FROM_LE(S, A) STRUCT_FROM_(STRUCT_LE(S##A))
#define STRUCT_TO_LE(S, A) STRUCT_TO_(STRUCT_LE(S##A))
#define STRUCT_FROM_BE(S, A) STRUCT_FROM_(STRUCT_BE(S##A))
#define STRUCT_TO_BE(S, A) STRUCT_TO_(STRUCT_BE(S##A))

#define BYTE_MCS_(T, X, S) (((T)((X) & 0xFFU)) << (S))
#define BYTE_SC_(T, X, S) ((uint8_t)(((T)(X)) >> (S)))

STRUCT_STATIC_INLINE STRUCT_FROM_LE(U, 64)
{
    uint64_t a = (uint64_t)(BYTE_MCS_(uint64_t, x.__data[0], 0U) | BYTE_MCS_(uint64_t, x.__data[1], 8U) | BYTE_MCS_(uint64_t, x.__data[2], 16U) | BYTE_MCS_(uint64_t, x.__data[3], 24U) | BYTE_MCS_(uint64_t, x.__data[4], 32U) | BYTE_MCS_(uint64_t, x.__data[5], 40U) | BYTE_MCS_(uint64_t, x.__data[6], 48U) | BYTE_MCS_(uint64_t, x.__data[7], 56U));
    return a;
}
STRUCT_STATIC_INLINE STRUCT_TO_LE(U, 64)
{
    STRUCT_LE(U64)
    a;
    a.__data[0] = BYTE_SC_(uint64_t, x, 0U);
    a.__data[1] = BYTE_SC_(uint64_t, x, 8U);
    a.__data[2] = BYTE_SC_(uint64_t, x, 16U);
    a.__data[3] = BYTE_SC_(uint64_t, x, 24U);
    a.__data[4] = BYTE_SC_(uint64_t, x, 32U);
    a.__data[5] = BYTE_SC_(uint64_t, x, 40U);
    a.__data[6] = BYTE_SC_(uint64_t, x, 48U);
    a.__data[7] = BYTE_SC_(uint64_t, x, 56U);
    return a;
}

STRUCT_STATIC_INLINE STRUCT_FROM_LE(S, 64)
{
    int64_t a = (int64_t)(BYTE_MCS_(uint64_t, x.__data[0], 0U) | BYTE_MCS_(uint64_t, x.__data[1], 8U) | BYTE_MCS_(uint64_t, x.__data[2], 16U) | BYTE_MCS_(uint64_t, x.__data[3], 24U) | BYTE_MCS_(uint64_t, x.__data[4], 32U) | BYTE_MCS_(uint64_t, x.__data[5], 40U) | BYTE_MCS_(uint64_t, x.__data[6], 48U) | BYTE_MCS_(uint64_t, x.__data[7], 56U));
    return a;
}
STRUCT_STATIC_INLINE STRUCT_TO_LE(S, 64)
{
    STRUCT_LE(S64)
    a;
    a.__data[0] = BYTE_SC_(uint64_t, x, 0U);
    a.__data[1] = BYTE_SC_(uint64_t, x, 8U);
    a.__data[2] = BYTE_SC_(uint64_t, x, 16U);
    a.__data[3] = BYTE_SC_(uint64_t, x, 24U);
    a.__data[4] = BYTE_SC_(uint64_t, x, 32U);
    a.__data[5] = BYTE_SC_(uint64_t, x, 40U);
    a.__data[6] = BYTE_SC_(uint64_t, x, 48U);
    a.__data[7] = BYTE_SC_(uint64_t, x, 56U);
    return a;
}

STRUCT_STATIC_INLINE STRUCT_FROM_LE(U, 32)
{
    uint32_t a = (uint32_t)(BYTE_MCS_(uint32_t, x.__data[0], 0U) | BYTE_MCS_(uint32_t, x.__data[1], 8U) | BYTE_MCS_(uint32_t, x.__data[2], 16U) | BYTE_MCS_(uint32_t, x.__data[3], 24U));
    return a;
}
STRUCT_STATIC_INLINE STRUCT_TO_LE(U, 32)
{
    STRUCT_LE(U32)
    a;
    a.__data[0] = BYTE_SC_(uint32_t, x, 0U);
    a.__data[1] = BYTE_SC_(uint32_t, x, 8U);
    a.__data[2] = BYTE_SC_(uint32_t, x, 16U);
    a.__data[3] = BYTE_SC_(uint32_t, x, 24U);
    return a;
}

STRUCT_STATIC_INLINE STRUCT_FROM_LE(S, 32)
{
    int32_t a = (int32_t)(BYTE_MCS_(uint32_t, x.__data[0], 0U) | BYTE_MCS_(uint32_t, x.__data[1], 8U) | BYTE_MCS_(uint32_t, x.__data[2], 16U) | BYTE_MCS_(uint32_t, x.__data[3], 24U));
    return a;
}
STRUCT_STATIC_INLINE STRUCT_TO_LE(S, 32)
{
    STRUCT_LE(S32)
    a;
    a.__data[0] = BYTE_SC_(uint32_t, x, 0U);
    a.__data[1] = BYTE_SC_(uint32_t, x, 8U);
    a.__data[2] = BYTE_SC_(uint32_t, x, 16U);
    a.__data[3] = BYTE_SC_(uint32_t, x, 24U);
    return a;
}

STRUCT_STATIC_INLINE STRUCT_FROM_LE(U, 24)
{
    uint32_t a = (uint32_t)(BYTE_MCS_(uint32_t, x.__data[0], 0U) | BYTE_MCS_(uint32_t, x.__data[1], 8U) | BYTE_MCS_(uint32_t, x.__data[2], 16U));
    return a;
}
STRUCT_STATIC_INLINE STRUCT_TO_LE(U, 24)
{
    STRUCT_LE(U24)
    a;
    a.__data[0] = BYTE_SC_(uint32_t, x, 0U);
    a.__data[1] = BYTE_SC_(uint32_t, x, 8U);
    a.__data[2] = BYTE_SC_(uint32_t, x, 16U);
    return a;
}

STRUCT_STATIC_INLINE STRUCT_FROM_LE(S, 24)
{
    uint32_t a = (uint32_t)(BYTE_MCS_(uint32_t, x.__data[0], 0U) | BYTE_MCS_(uint32_t, x.__data[1], 8U) | BYTE_MCS_(uint32_t, x.__data[2], 16U));
    return ((int32_t)(a << 8)) >> 8;
}
STRUCT_STATIC_INLINE STRUCT_TO_LE(S, 24)
{
    STRUCT_LE(S24)
    a;
    a.__data[0] = BYTE_SC_(uint32_t, x, 0U);
    a.__data[1] = BYTE_SC_(uint32_t, x, 8U);
    a.__data[2] = BYTE_SC_(uint32_t, x, 16U);
    return a;
}

STRUCT_STATIC_INLINE STRUCT_FROM_LE(U, 16)
{
    uint16_t a = (uint16_t)(BYTE_MCS_(uint16_t, x.__data[0], 0U) | BYTE_MCS_(uint16_t, x.__data[1], 8U));
    return a;
}
STRUCT_STATIC_INLINE STRUCT_TO_LE(U, 16)
{
    STRUCT_LE(U16)
    a;
    a.__data[0] = BYTE_SC_(uint16_t, x, 0U);
    a.__data[1] = BYTE_SC_(uint16_t, x, 8U);
    return a;
}

STRUCT_STATIC_INLINE STRUCT_FROM_LE(S, 16)
{
    int16_t a = (int16_t)(BYTE_MCS_(uint16_t, x.__data[0], 0U) | BYTE_MCS_(uint16_t, x.__data[1], 8U));
    return a;
}
STRUCT_STATIC_INLINE STRUCT_TO_LE(S, 16)
{
    STRUCT_LE(S16)
    a;
    a.__data[0] = BYTE_SC_(uint16_t, x, 0U);
    a.__data[1] = BYTE_SC_(uint16_t, x, 8U);
    return a;
}

STRUCT_STATIC_INLINE STRUCT_FROM_LE(U, 8)
{
    uint8_t a = (uint8_t)(BYTE_MCS_(uint8_t, x.__data[0], 0U));
    return a;
}
STRUCT_STATIC_INLINE STRUCT_TO_LE(U, 8)
{
    STRUCT_LE(U8)
    a;
    a.__data[0] = BYTE_SC_(uint8_t, x, 0U);
    return a;
}

STRUCT_STATIC_INLINE STRUCT_FROM_LE(S, 8)
{
    int8_t a = (int8_t)(BYTE_MCS_(uint8_t, x.__data[0], 0U));
    return a;
}
STRUCT_STATIC_INLINE STRUCT_TO_LE(S, 8)
{
    STRUCT_LE(S8)
    a;
    a.__data[0] = BYTE_SC_(uint8_t, x, 0U);
    return a;
}

STRUCT_STATIC_INLINE STRUCT_FROM_BE(U, 64)
{
    uint64_t a = (uint64_t)(BYTE_MCS_(uint64_t, x.__data[0], 56U) | BYTE_MCS_(uint64_t, x.__data[1], 48U) | BYTE_MCS_(uint64_t, x.__data[2], 40U) | BYTE_MCS_(uint64_t, x.__data[3], 32U) | BYTE_MCS_(uint64_t, x.__data[4], 24U) | BYTE_MCS_(uint64_t, x.__data[5], 16U) | BYTE_MCS_(uint64_t, x.__data[6], 8U) | BYTE_MCS_(uint64_t, x.__data[7], 0U));
    return a;
}
STRUCT_STATIC_INLINE STRUCT_TO_BE(U, 64)
{
    STRUCT_BE(U64)
    a;
    a.__data[0] = BYTE_SC_(uint64_t, x, 56U);
    a.__data[1] = BYTE_SC_(uint64_t, x, 48U);
    a.__data[2] = BYTE_SC_(uint64_t, x, 40U);
    a.__data[3] = BYTE_SC_(uint64_t, x, 32U);
    a.__data[4] = BYTE_SC_(uint64_t, x, 24U);
    a.__data[5] = BYTE_SC_(uint64_t, x, 16U);
    a.__data[6] = BYTE_SC_(uint64_t, x, 8U);
    a.__data[7] = BYTE_SC_(uint64_t, x, 0U);
    return a;
}

STRUCT_STATIC_INLINE STRUCT_FROM_BE(S, 64)
{
    int64_t a = (int64_t)(BYTE_MCS_(uint64_t, x.__data[0], 56U) | BYTE_MCS_(uint64_t, x.__data[1], 48U) | BYTE_MCS_(uint64_t, x.__data[2], 40U) | BYTE_MCS_(uint64_t, x.__data[3], 32U) | BYTE_MCS_(uint64_t, x.__data[4], 24U) | BYTE_MCS_(uint64_t, x.__data[5], 16U) | BYTE_MCS_(uint64_t, x.__data[6], 8U) | BYTE_MCS_(uint64_t, x.__data[7], 0U));
    return a;
}
STRUCT_STATIC_INLINE STRUCT_TO_BE(S, 64)
{
    STRUCT_BE(S64)
    a;
    a.__data[0] = BYTE_SC_(uint64_t, x, 56U);
    a.__data[1] = BYTE_SC_(uint64_t, x, 48U);
    a.__data[2] = BYTE_SC_(uint64_t, x, 40U);
    a.__data[3] = BYTE_SC_(uint64_t, x, 32U);
    a.__data[4] = BYTE_SC_(uint64_t, x, 24U);
    a.__data[5] = BYTE_SC_(uint64_t, x, 16U);
    a.__data[6] = BYTE_SC_(uint64_t, x, 8U);
    a.__data[7] = BYTE_SC_(uint64_t, x, 0U);
    return a;
}

STRUCT_STATIC_INLINE STRUCT_FROM_BE(U, 32)
{
    uint32_t a = (uint32_t)(BYTE_MCS_(uint32_t, x.__data[0], 24U) | BYTE_MCS_(uint32_t, x.__data[1], 16U) | BYTE_MCS_(uint32_t, x.__data[2], 8U) | BYTE_MCS_(uint32_t, x.__data[3], 0U));
    return a;
}
STRUCT_STATIC_INLINE STRUCT_TO_BE(U, 32)
{
    STRUCT_BE(U32)
    a;
    a.__data[0] = BYTE_SC_(uint32_t, x, 24U);
    a.__data[1] = BYTE_SC_(uint32_t, x, 16U);
    a.__data[2] = BYTE_SC_(uint32_t, x, 8U);
    a.__data[3] = BYTE_SC_(uint32_t, x, 0U);
    return a;
}

STRUCT_STATIC_INLINE STRUCT_FROM_BE(S, 32)
{
    int32_t a = (int32_t)(BYTE_MCS_(uint32_t, x.__data[0], 24U) | BYTE_MCS_(uint32_t, x.__data[1], 16U) | BYTE_MCS_(uint32_t, x.__data[2], 8U) | BYTE_MCS_(uint32_t, x.__data[3], 0U));
    return a;
}
STRUCT_STATIC_INLINE STRUCT_TO_BE(S, 32)
{
    STRUCT_BE(S32)
    a;
    a.__data[0] = BYTE_SC_(uint32_t, x, 24U);
    a.__data[1] = BYTE_SC_(uint32_t, x, 16U);
    a.__data[2] = BYTE_SC_(uint32_t, x, 8U);
    a.__data[3] = BYTE_SC_(uint32_t, x, 0U);
    return a;
}

STRUCT_STATIC_INLINE STRUCT_FROM_BE(U, 24)
{
    uint32_t a = (uint32_t)(BYTE_MCS_(uint32_t, x.__data[0], 16U) | BYTE_MCS_(uint32_t, x.__data[1], 8U) | BYTE_MCS_(uint32_t, x.__data[2], 0U));
    return a;
}
STRUCT_STATIC_INLINE STRUCT_TO_BE(U, 24)
{
    STRUCT_BE(U24)
    a;
    a.__data[0] = BYTE_SC_(uint32_t, x, 16U);
    a.__data[1] = BYTE_SC_(uint32_t, x, 8U);
    a.__data[2] = BYTE_SC_(uint32_t, x, 0U);
    return a;
}

STRUCT_STATIC_INLINE STRUCT_FROM_BE(S, 24)
{
    uint32_t a = (uint32_t)(BYTE_MCS_(uint32_t, x.__data[0], 16U) | BYTE_MCS_(uint32_t, x.__data[1], 8U) | BYTE_MCS_(uint32_t, x.__data[2], 0U));
    return ((int32_t)(a << 8)) >> 8;
}
STRUCT_STATIC_INLINE STRUCT_TO_BE(S, 24)
{
    STRUCT_BE(S24)
    a;
    a.__data[0] = BYTE_SC_(uint32_t, x, 16U);
    a.__data[1] = BYTE_SC_(uint32_t, x, 8U);
    a.__data[2] = BYTE_SC_(uint32_t, x, 0U);
    return a;
}

STRUCT_STATIC_INLINE STRUCT_FROM_BE(U, 16)
{
    uint16_t a = (uint16_t)(BYTE_MCS_(uint16_t, x.__data[0], 8U) | BYTE_MCS_(uint16_t, x.__data[1], 0U));
    return a;
}

STRUCT_STATIC_INLINE STRUCT_TO_BE(U, 16)
{
    STRUCT_BE(U16)
    a;
    a.__data[0] = BYTE_SC_(uint16_t, x, 8U);
    a.__data[1] = BYTE_SC_(uint16_t, x, 0U);
    return a;
}

STRUCT_STATIC_INLINE STRUCT_FROM_BE(S, 16)
{
    int16_t a = (int16_t)(BYTE_MCS_(uint16_t, x.__data[0], 8U) | BYTE_MCS_(uint16_t, x.__data[1], 0U));
    return a;
}
STRUCT_STATIC_INLINE STRUCT_TO_BE(S, 16)
{
    STRUCT_BE(S16)
    a;
    a.__data[0] = BYTE_SC_(uint16_t, x, 8U);
    a.__data[1] = BYTE_SC_(uint16_t, x, 0U);
    return a;
}

STRUCT_STATIC_INLINE STRUCT_FROM_BE(U, 8)
{
    uint8_t a = (uint8_t)(BYTE_MCS_(uint8_t, x.__data[0], 0U));
    return a;
}
STRUCT_STATIC_INLINE STRUCT_TO_BE(U, 8)
{
    STRUCT_BE(U8)
    a;
    a.__data[0] = BYTE_SC_(uint8_t, x, 0U);
    return a;
}

STRUCT_STATIC_INLINE STRUCT_FROM_BE(S, 8)
{
    int8_t a = (int8_t)(BYTE_MCS_(uint8_t, x.__data[0], 0U));
    return a;
}
STRUCT_STATIC_INLINE STRUCT_TO_BE(S, 8)
{
    STRUCT_BE(S8)
    a;
    a.__data[0] = BYTE_SC_(uint8_t, x, 0U);
    return a;
}

#undef BYTE_MCS_
#undef BYTE_SC_

#endif
