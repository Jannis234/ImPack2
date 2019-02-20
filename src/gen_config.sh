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

echo "// Auto-generated, edit config_build.mak instead"
echo
echo "#ifndef __IMPACK_CONFIG_GENERATED_H__"
echo "#define __IMPACK_CONFIG_GENERATED_H__"
echo
echo "#define IMPACK_VERSION_STRING \"$1\""
echo "#define IMPACK_CONFIG_NETTLE $2"
echo "#define IMPACK_CONFIG_PNG $3"
echo "#define IMPACK_CONFIG_WEBP $4"
echo "#define IMPACK_CONFIG_ZLIB $5"
echo "#define IMPACK_CONFIG_ZSTD $6"
echo "#define IMPACK_CONFIG_LZMA $7"
echo
echo "#endif"
