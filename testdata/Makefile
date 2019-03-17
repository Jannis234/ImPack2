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

# Makefile to generate all supported formats from PNG using ImageMagick
# You don't need to run this

IMGS = $(shell ls *.png)

all: $(IMGS:.png=.webp) $(IMGS:.png=.tiff) $(IMGS:.png=.bmp) $(IMGS:.png=.jp2) $(IMGS:.png=.flif) $(IMGS:.png=.jxr)

%.webp: %.png
	convert $< -define webp:lossless=true $@

%.tiff: %.png
	convert $< $@

%.bmp: %.png
	convert $< out.bmp3
	mv out.bmp3 $@

%.jp2: %.png
	convert $< $@

%.flif: %.png
	flif --overwrite -e $< $@

%.jxr: %.bmp
	JxrEncApp -q 1 -i $< -o $@
