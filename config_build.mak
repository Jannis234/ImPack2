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
IMPACK_VERSION = 1.2

# Options to control features that need additional dependencies
# Change any option to 0 to disable it

# Enable or disable building the GUI
WITH_GTK = 1

# Required to create and unpack an image with encrypted data
WITH_NETTLE = 1

# Image formats, at least one must be enabled
WITH_LIBPNG = 1
WITH_LIBWEBP = 1
WITH_LIBTIFF = 1
WITH_LIBNSBMP = 1
WITH_OPENJPEG2 = 1
WITH_FLIF = 1
WITH_JXRLIB = 1
WITH_CHARLS = 1

# Compression algorithms
WITH_ZLIB = 1
WITH_ZSTD = 1
WITH_LZMA = 1
WITH_BZIP2 = 1
WITH_BROTLI = 1
