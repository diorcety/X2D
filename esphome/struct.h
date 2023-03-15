
#ifndef STRUCT_H_
#define STRUCT_H_

#include <stdint.h>

#define STRUCT_STATIC_INLINE static inline

#if BYTEORDER_ENDIAN == BYTEORDER_LITTLE_ENDIAN
#define STRUCT_NATIVE(A) L##A
#define STRUCT_EVITAN(A) B##A
#else
#define STRUCT_NATIVE(A) B##A
#define STRUCT_EVITAN(A) L##A
#endif

#define STRUCT_TYPE_SIZE_LS8 1
#define STRUCT_TYPE_SIZE_BS8 1
#define STRUCT_TYPE_SIZE_LU8 1
#define STRUCT_TYPE_SIZE_BU8 1
#define STRUCT_TYPE_SIZE_LS16 2
#define STRUCT_TYPE_SIZE_BS16 2
#define STRUCT_TYPE_SIZE_LU16 2
#define STRUCT_TYPE_SIZE_BU16 2
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

#define STRUCT_TYPE_TO_NATIVE_LU64 uint64_t
#define STRUCT_TYPE_TO_NATIVE_BU64 uint64_t
#define STRUCT_TYPE_TO_NATIVE_LS64 int64_t
#define STRUCT_TYPE_TO_NATIVE_BS64 int64_t
#define STRUCT_TYPE_TO_NATIVE_LU32 uint32_t
#define STRUCT_TYPE_TO_NATIVE_BU32 uint32_t
#define STRUCT_TYPE_TO_NATIVE_LS32 int32_t
#define STRUCT_TYPE_TO_NATIVE_BS32 int32_t
#define STRUCT_TYPE_TO_NATIVE_LU16 uint16_t
#define STRUCT_TYPE_TO_NATIVE_BU16 uint16_t
#define STRUCT_TYPE_TO_NATIVE_LS16 int16_t
#define STRUCT_TYPE_TO_NATIVE_BS16 int16_t
#define STRUCT_TYPE_TO_NATIVE_LU8 uint8_t
#define STRUCT_TYPE_TO_NATIVE_BU8 uint8_t
#define STRUCT_TYPE_TO_NATIVE_LS8 int8_t
#define STRUCT_TYPE_TO_NATIVE_BS8 int8_t
#define STRUCT_TYPE_TO_NATIVE_(X) STRUCT_TYPE_TO_NATIVE_##X
#define STRUCT_TYPE_TO_NATIVE(X) STRUCT_TYPE_TO_NATIVE_(X)

#define STRUCT_TYPE_TO_NATIVE_UPPER_LU64 UNS64
#define STRUCT_TYPE_TO_NATIVE_UPPER_BU64 UNS64
#define STRUCT_TYPE_TO_NATIVE_UPPER_LS64 S64
#define STRUCT_TYPE_TO_NATIVE_UPPER_BS64 S64
#define STRUCT_TYPE_TO_NATIVE_UPPER_LU32 UNS32
#define STRUCT_TYPE_TO_NATIVE_UPPER_BU32 UNS32
#define STRUCT_TYPE_TO_NATIVE_UPPER_LS32 S32
#define STRUCT_TYPE_TO_NATIVE_UPPER_BS32 S32
#define STRUCT_TYPE_TO_NATIVE_UPPER_LU16 UNS16
#define STRUCT_TYPE_TO_NATIVE_UPPER_BU16 UNS16
#define STRUCT_TYPE_TO_NATIVE_UPPER_LS16 S16
#define STRUCT_TYPE_TO_NATIVE_UPPER_BS16 S16
#define STRUCT_TYPE_TO_NATIVE_UPPER_LU8 UNS8
#define STRUCT_TYPE_TO_NATIVE_UPPER_BU8 UNS8
#define STRUCT_TYPE_TO_NATIVE_UPPER_LS8 S8
#define STRUCT_TYPE_TO_NATIVE_UPPER_BS8 S8
#define STRUCT_TYPE_TO_NATIVE_UPPER_(X) STRUCT_TYPE_TO_NATIVE_UPPER_##X
#define STRUCT_TYPE_TO_NATIVE_UPPER(X) STRUCT_TYPE_TO_NATIVE_UPPER_(X)

#define STRUCT_FROM_FCT_NAME__(A, B) A##_TO_##B
#define STRUCT_FROM_FCT_NAME_(A, B) STRUCT_FROM_FCT_NAME__(A, B)
#define STRUCT_FROM_FCT_NAME(A) STRUCT_FROM_FCT_NAME_(A, STRUCT_TYPE_TO_NATIVE_UPPER(A))
#define STRUCT_TO_FCT_NAME__(A, B) B##_TO_##A
#define STRUCT_TO_FCT_NAME_(A, B) STRUCT_TO_FCT_NAME__(A, B)
#define STRUCT_TO_FCT_NAME(A) STRUCT_TO_FCT_NAME_(A, STRUCT_TYPE_TO_NATIVE_UPPER(A))

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
#else
#error Compiler not supported
#endif

#define TYPEDEF_PACKED_STRUCT_BEG(N) typedef PACKED_STRUCT_BEG(_##N)
#define TYPEDEF_PACKED_STRUCT_END(N) PACKED_STRUCT_END(N)

#ifdef __cplusplus
#define _TYPEDEF_ENUM_BEG(N, S) \
    typedef enum _##N : S       \
    {
#define _TYPEDEF_ENUM_END(N, S) \
    }                           \
    N;

#define _ENUM_VAR(N, M, S) N M
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
#define _ENUM_VAR(N, M, S) N M : S
#endif
#endif

#define TYPEDEF_ENUM_BEG(N) _TYPEDEF_ENUM_BEG(N, uint8_t)
#define TYPEDEF_ENUM_END(N) _TYPEDEF_ENUM_END(N, uint8_t)
#define ENUM_VAR(N, M) _ENUM_VAR(N, M, 8);

#define VAR(N, M) N M;

#define _TYPEDEF_TYPE(N)                       \
    TYPEDEF_PACKED_STRUCT_BEG(N)               \
    unsigned char __data[STRUCT_TYPE_SIZE(N)]; \
    TYPEDEF_PACKED_STRUCT_END(N)
_TYPEDEF_TYPE(STRUCT_NATIVE(U64))
_TYPEDEF_TYPE(STRUCT_EVITAN(U64))
_TYPEDEF_TYPE(STRUCT_NATIVE(S64))
_TYPEDEF_TYPE(STRUCT_EVITAN(S64))
_TYPEDEF_TYPE(STRUCT_NATIVE(U32))
_TYPEDEF_TYPE(STRUCT_EVITAN(U32))
_TYPEDEF_TYPE(STRUCT_NATIVE(S32))
_TYPEDEF_TYPE(STRUCT_EVITAN(S32))
_TYPEDEF_TYPE(STRUCT_NATIVE(U16))
_TYPEDEF_TYPE(STRUCT_EVITAN(U16))
_TYPEDEF_TYPE(STRUCT_NATIVE(S16))
_TYPEDEF_TYPE(STRUCT_EVITAN(S16))
_TYPEDEF_TYPE(STRUCT_NATIVE(U8))
_TYPEDEF_TYPE(STRUCT_EVITAN(U8))
_TYPEDEF_TYPE(STRUCT_NATIVE(S8))
_TYPEDEF_TYPE(STRUCT_EVITAN(S8))
#undef _TYPEDEF_TYPE

#define TYPEDEF_BITFIELD_BEG(N, T) _TYPEDEF_ENUM_BEG(N, uint8_t)
#define TYPEDEF_BITFIELD_END(N, T)                                                     \
    _TYPEDEF_ENUM_END(N, uint8_t)                                                         \
    TYPEDEF_PACKED_STRUCT_BEG(STRUCT_BF_##N)                                           \
    T __value;                                                                         \
    TYPEDEF_PACKED_STRUCT_END(STRUCT_BF_##N)                                           \
    typedef STRUCT_TYPE_TO_NATIVE(T) STRUCT_BF_##N##_NT;                               \
    STRUCT_STATIC_INLINE STRUCT_BF_##N##_NT STRUCT_BF_##N##_TO_NATIVE(STRUCT_BF_##N x) \
    {                                                                                  \
        return STRUCT_FROM_FCT_NAME(T)(x.__value);                                     \
    }                                                                                  \
    STRUCT_STATIC_INLINE STRUCT_BF_##N NATIVE_TO_##STRUCT_BF_##N(STRUCT_BF_##N##_NT x) \
    {                                                                                  \
        STRUCT_BF_##N ret;                                                             \
        ret.__value = STRUCT_TO_FCT_NAME(T)(x);                                        \
        return ret;                                                                    \
    }

#define BITFIELD_BIT_TO_INT(N, B) (((STRUCT_BF_##N##_NT)1) << ((uint8_t)B))
#define BITFIELD_BIT_TO_SMASK(N, O) ((STRUCT_BF_##N##_NT)(((STRUCT_BF_##N##_NT)(~((STRUCT_BF_##N##_NT)0))) << (O)))

#define BITFIELD_SET(N, V, X) (V = NATIVE_TO_##STRUCT_BF_##N(X))
#define BITFIELD_GET(N, V) (STRUCT_BF_##N##_TO_NATIVE(V))
#define BITFIELD_GET_RANGE(N, V, H, L) \
    ((BITFIELD_GET(N, V) & ((STRUCT_BF_##N##_NT)(~BITFIELD_BIT_TO_SMASK(N, ((uint8_t)H) + 1)))) >> ((uint8_t)L))
#define BITFIELD_SET_RANGE(N, V, H, L, X) \
    (BITFIELD_SET(                        \
        N, V,                             \
        ((BITFIELD_GET(N, V) & ~(BITFIELD_BIT_TO_SMASK(N, ((uint8_t)H + 1)) ^ BITFIELD_BIT_TO_SMASK(N, ((uint8_t)L))))) | (((STRUCT_BF_##N##_NT)(X & (~(BITFIELD_BIT_TO_SMASK(N, ((uint8_t)H) - ((uint8_t)L) + 1))))) << ((uint8_t)L))))

#define BITFIELD_BIT_IS_SET(N, V, B) ((STRUCT_BF_##N##_TO_NATIVE(V) & BITFIELD_BIT_TO_INT(N, B)) != 0)
#define BITFIELD_BIT_SET(N, V, B) (BITFIELD_SET(N, V, (BITFIELD_GET(N, V) | BITFIELD_BIT_TO_INT(N, B))))
#define BITFIELD_BIT_CLR(N, V, B) (BITFIELD_SET(N, V, (BITFIELD_GET(N, V) & ~BITFIELD_BIT_TO_INT(N, B))))
#define BITFIELD_BIT_TOGGLE(N, V, B) (BITFIELD_SET(N, V, (BITFIELD_GET(N, V) ^ BITFIELD_BIT_TO_INT(N, B))))
#define BITFIELD_BIT_SET_CLR(C, N, V, B) ((C) ? BITFIELD_BIT_SET(N, V, B) : BITFIELD_BIT_CLR(N, V, B))
#define BITFIELD_VAR(N, M) STRUCT_BF_##N M;

//
// Define conversion functions
//

#define STRUCT_FROM_FCT(A) STRUCT_TYPE_TO_NATIVE(A) STRUCT_FROM_FCT_NAME(A)(A x)
#define STRUCT_TO_FCT(A) A STRUCT_TO_FCT_NAME(A)(STRUCT_TYPE_TO_NATIVE(A) x)

#define STRUCT_FROM_(X) STRUCT_STATIC_INLINE STRUCT_FROM_FCT(X)
#define STRUCT_TO_(X) STRUCT_STATIC_INLINE STRUCT_TO_FCT(X)

#define STRUCT_FROM_NATIVE(S, A) STRUCT_FROM_(STRUCT_NATIVE(S##A))
#define STRUCT_TO_NATIVE(S, A) STRUCT_TO_(STRUCT_NATIVE(S##A))
#define STRUCT_FROM_EVITAN(S, A) STRUCT_FROM_(STRUCT_EVITAN(S##A))
#define STRUCT_TO_EVITAN(S, A) STRUCT_TO_(STRUCT_EVITAN(S##A))
#define _BYTE_MASK(x) ((x)&0xFF)

STRUCT_FROM_NATIVE(U, 64)
{
    return *((uint64_t *)&x);
}
STRUCT_TO_NATIVE(U, 64)
{
    STRUCT_NATIVE(U64) *a = (STRUCT_NATIVE(U64) *)&x;
    return *a;
}

STRUCT_FROM_NATIVE(S, 64)
{
    return *((int64_t *)&x);
}
STRUCT_TO_NATIVE(S, 64)
{
    STRUCT_NATIVE(S64) *a = (STRUCT_NATIVE(S64) *)&x;
    return *a;
}

STRUCT_FROM_NATIVE(U, 32)
{
    return *((uint32_t *)&x);
}
STRUCT_TO_NATIVE(U, 32)
{
    STRUCT_NATIVE(U32) *a = (STRUCT_NATIVE(U32) *)&x;
    return *a;
}

STRUCT_FROM_NATIVE(S, 32)
{
    return *((int32_t *)&x);
}
STRUCT_TO_NATIVE(S, 32)
{
    STRUCT_NATIVE(S32) *a = (STRUCT_NATIVE(S32) *)&x;
    return *a;
}

STRUCT_FROM_NATIVE(U, 16)
{
    return *((uint16_t *)&x);
}
STRUCT_TO_NATIVE(U, 16)
{
    STRUCT_NATIVE(U16) *a = (STRUCT_NATIVE(U16) *)&x;
    return *a;
}

STRUCT_FROM_NATIVE(S, 16)
{
    return *((int16_t *)&x);
}
STRUCT_TO_NATIVE(S, 16)
{
    STRUCT_NATIVE(S16) *a = (STRUCT_NATIVE(S16) *)&x;
    return *a;
}

STRUCT_FROM_NATIVE(U, 8)
{
    return *((uint8_t *)&x);
}
STRUCT_TO_NATIVE(U, 8)
{
    STRUCT_NATIVE(U8) *a = (STRUCT_NATIVE(U8) *)&x;
    return *a;
}

STRUCT_FROM_NATIVE(S, 8)
{
    return *(int8_t *)&x;
}
STRUCT_TO_NATIVE(S, 8)
{
    STRUCT_NATIVE(S8) *a = (STRUCT_NATIVE(S8) *)&x;
    return *a;
}

STRUCT_FROM_EVITAN(U, 64)
{
    uint64_t a = *((uint64_t *)&x);
    a = (_BYTE_MASK(a >> 0) << 56) | (_BYTE_MASK(a >> 8) << 48) | (_BYTE_MASK(a >> 16) << 40) | (_BYTE_MASK(a >> 24) << 32) | (_BYTE_MASK(a >> 32) << 24) | (_BYTE_MASK(a >> 40) << 16) | (_BYTE_MASK(a >> 48) << 8) | (_BYTE_MASK(a >> 56) << 0);
    return a;
}
STRUCT_TO_EVITAN(U, 64)
{
    STRUCT_EVITAN(U64) * a;
    x = (_BYTE_MASK(x >> 0) << 56) | (_BYTE_MASK(x >> 8) << 48) | (_BYTE_MASK(x >> 16) << 40) | (_BYTE_MASK(x >> 24) << 32) | (_BYTE_MASK(x >> 32) << 24) | (_BYTE_MASK(x >> 40) << 16) | (_BYTE_MASK(x >> 48) << 8) | (_BYTE_MASK(x >> 56) << 0);
    a = (STRUCT_EVITAN(U64) *)&x;
    return *a;
}

STRUCT_FROM_EVITAN(S, 64)
{
    int64_t a = *((int64_t *)&x);
    a = (_BYTE_MASK(a >> 0) << 56) | (_BYTE_MASK(a >> 8) << 48) | (_BYTE_MASK(a >> 16) << 40) | (_BYTE_MASK(a >> 24) << 32) | (_BYTE_MASK(a >> 32) << 24) | (_BYTE_MASK(a >> 40) << 16) | (_BYTE_MASK(a >> 48) << 8) | (_BYTE_MASK(a >> 56) << 0);
    return a;
}
STRUCT_TO_EVITAN(S, 64)
{
    STRUCT_EVITAN(S64) * a;
    x = (_BYTE_MASK(x >> 0) << 56) | (_BYTE_MASK(x >> 8) << 48) | (_BYTE_MASK(x >> 16) << 40) | (_BYTE_MASK(x >> 24) << 32) | (_BYTE_MASK(x >> 32) << 24) | (_BYTE_MASK(x >> 40) << 16) | (_BYTE_MASK(x >> 48) << 8) | (_BYTE_MASK(x >> 56) << 0);
    a = (STRUCT_EVITAN(S64) *)&x;
    return *a;
}

STRUCT_FROM_EVITAN(U, 32)
{
    uint32_t a = *((uint32_t *)&x);
    a = (_BYTE_MASK(a >> 0) << 24) | (_BYTE_MASK(a >> 8) << 16) | (_BYTE_MASK(a >> 16) << 8) | (_BYTE_MASK(a >> 24) << 0);
    return a;
}
STRUCT_TO_EVITAN(U, 32)
{
    STRUCT_EVITAN(U32) * a;
    x = (_BYTE_MASK(x >> 0) << 24) | (_BYTE_MASK(x >> 8) << 16) | (_BYTE_MASK(x >> 16) << 8) | (_BYTE_MASK(x >> 24) << 0);
    a = (STRUCT_EVITAN(U32) *)&x;
    return *a;
}

STRUCT_FROM_EVITAN(S, 32)
{
    int32_t a = *((int32_t *)&x);
    a = (_BYTE_MASK(a >> 0) << 24) | (_BYTE_MASK(a >> 8) << 16) | (_BYTE_MASK(a >> 16) << 8) | (_BYTE_MASK(a >> 24) << 0);
    return a;
}
STRUCT_TO_EVITAN(S, 32)
{
    STRUCT_EVITAN(S32) * a;
    x = (_BYTE_MASK(x >> 0) << 24) | (_BYTE_MASK(x >> 8) << 16) | (_BYTE_MASK(x >> 16) << 8) | (_BYTE_MASK(x >> 24) << 0);
    a = (STRUCT_EVITAN(S32) *)&x;
    return *a;
}

STRUCT_FROM_EVITAN(U, 16)
{
    uint16_t a = *((uint16_t *)&x);
    a = (_BYTE_MASK(a >> 0) << 8) | (_BYTE_MASK(a >> 8) << 0);
    return a;
}
STRUCT_TO_EVITAN(U, 16)
{
    STRUCT_EVITAN(U16) * a;
    x = (_BYTE_MASK(x >> 0) << 8) | (_BYTE_MASK(x >> 8) << 0);
    a = (STRUCT_EVITAN(U16) *)&x;
    return *a;
}

STRUCT_FROM_EVITAN(S, 16)
{
    int16_t a = *((int16_t *)&x);
    a = (_BYTE_MASK(a >> 0) << 8) | (_BYTE_MASK(a >> 8) << 0);
    return a;
}
STRUCT_TO_EVITAN(S, 16)
{
    STRUCT_EVITAN(S16) * a;
    x = (_BYTE_MASK(x >> 0) << 8) | (_BYTE_MASK(x >> 8) << 0);
    a = (STRUCT_EVITAN(S16) *)&x;
    return *a;
}

STRUCT_FROM_EVITAN(U, 8)
{
    return *((uint8_t *)&x);
}
STRUCT_TO_EVITAN(U, 8)
{
    STRUCT_EVITAN(U8) *a = (STRUCT_EVITAN(U8) *)&x;
    return *a;
}

STRUCT_FROM_EVITAN(S, 8)
{
    return *(int8_t *)&x;
}
STRUCT_TO_EVITAN(S, 8)
{
    STRUCT_EVITAN(S8) *a = (STRUCT_EVITAN(S8) *)&x;
    return *a;
}

#undef _BYTE_MASK
#undef STRUCT_FROM_NATIVE
#undef STRUCT_TO_NATIVE
#undef STRUCT_FROM_EVITAN
#undef STRUCT_TO_EVITAN
#undef STRUCT_FROM_
#undef STRUCT_TO_
#undef STRUCT_FROM_FCT
#undef STRUCT_TO_FCT

#endif
