#ifndef _STDINT_H
#define _STDINT_H

typedef __int8 int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
typedef unsigned __int64 uint64_t;

//The fix
typedef __int8  int_least8_t;
typedef __int16 int_least16_t;
typedef __int32 int_least32_t;
typedef __int64 int_least64_t;
typedef unsigned __int8 uint_least8_t;
typedef unsigned __int16 uint_least16_t;
typedef unsigned __int32 uint_least32_t;
typedef unsigned __int64 uint_least64_t;

typedef __int8 int_fast8_t;
typedef __int16 int_fast16_t;
typedef __int32 int_fast32_t;
typedef __int64 int_fast64_t;
typedef unsigned __int8 uint_fast8_t;
typedef unsigned __int16 uint_fast16_t;
typedef unsigned __int32 uint_fast32_t;
typedef unsigned __int64 uint_fast64_t;

typedef long long intmax_t;
typedef unsigned long long uintmax_t;

#endif // _STDINT_H


// Copyright (C) 2004-2006 Pixelgene Ltd. All rights reserved. Consult your license regarding permissions and restrictions.
