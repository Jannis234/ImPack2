# This file is part of ImPack2.
#
# ImPack2 is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ImPack2 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with ImPack2. If not, see <http://www.gnu.org/licenses/>.

#IMPACK_VERSION = git-$(shell git rev-parse --short HEAD)
IMPACK_VERSION = 1.5

# Options to control features that need additional dependencies
# Change any option to 0 to disable it

# Build the GUI
WITH_GTK = 1

# Encryption support
WITH_NETTLE = 1
# Argon2 passphrase hashing support, highly recommended
WITH_ARGON2 = 1

# Image formats, at least one must be enabled
# Default output format, you likely want to enable this
WITH_LIBPNG = 1
# Other, less common formats
WITH_CHARLS = 1
WITH_LIBAVIF = 1
WITH_LIBHEIF = 1
WITH_LIBJXL = 1
WITH_LIBNSBMP = 1
WITH_LIBTIFF = 1
WITH_LIBWEBP = 1
WITH_OPENJPEG2 = 1
# Deprecated, unmaintained formats and libraries; recommended to leave these off
WITH_FLIF = 0
WITH_JXRLIB = 0

# Compression support
# Default algorithm, recommended to enable this
WITH_ZSTD = 1
# When enabled, appears in the GUI as "strong" compression
WITH_LZMA = 1
# Other alternatives
WITH_BROTLI = 1
WITH_BZIP2 = 1
WITH_ZLIB = 1
