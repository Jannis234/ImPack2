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

# Options to control features that need additional dependencies
# Change any option to 0 to disable it

# Required to create and unpack an image with encrypted data
WITH_NETTLE = 1

# Image formats, at least one must be enabled
WITH_LIBPNG = 1
WITH_LIBWEBP = 1

# Compression algorithms
WITH_ZLIB = 1
WITH_ZSTD = 1
