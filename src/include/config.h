/* This file is part of ImPack2.
 *
 * ImPack2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ImPack2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ImPack2. If not, see <http://www.gnu.org/licenses/>. */

#ifndef __IMPACK_CONFIG_H__
#define __IMPACK_CONFIG_H__

#include "config_generated.h"

#if defined(WIN32) || defined(_WIN32)
#define IMPACK_WINDOWS
#endif

#if (IMPACK_CONFIG_NETTLE == 1)
#define IMPACK_WITH_CRYPTO
#endif

#if (IMPACK_CONFIG_PNG != 1) && (IMPACK_CONFIG_WEBP != 1) && (IMPACK_CONFIG_TIFF != 1) && (IMPACK_CONFIG_BMP != 1)
#error "No image formats selected in config_build.mak"
#endif

#if (IMPACK_CONFIG_PNG == 1)
#define IMPACK_WITH_PNG
#endif

#if (IMPACK_CONFIG_WEBP == 1)
#define IMPACK_WITH_WEBP
#endif

#if (IMPACK_CONFIG_TIFF == 1)
#define IMPACK_WITH_TIFF
#endif

#if (IMPACK_CONFIG_BMP == 1)
#define IMPACK_WITH_BMP
#endif

#if (IMPACK_CONFIG_ZLIB == 1) || (IMPACK_CONFIG_ZSTD == 1) || (IMPACK_CONFIG_LZMA == 1) || (IMPACK_CONFIG_BZIP2 == 1) || (IMPACK_CONFIG_BROTLI == 1)
#define IMPACK_WITH_COMPRESSION
#endif

#if (IMPACK_CONFIG_ZLIB == 1)
#define IMPACK_WITH_ZLIB
#endif

#if (IMPACK_CONFIG_ZSTD == 1)
#define IMPACK_WITH_ZSTD
#endif

#if (IMPACK_CONFIG_LZMA == 1)
#define IMPACK_WITH_LZMA
#endif

#if (IMPACK_CONFIG_BZIP2 == 1)
#define IMPACK_WITH_BZIP2
#endif

#if (IMPACK_CONFIG_BROTLI == 1)
#define IMPACK_WITH_BROTLI
#endif

#endif
