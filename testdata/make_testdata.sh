#!/bin/bash

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

# This script can generate the reference files for ImPack2's testsuite
# Since these are used as a ground truth for correct behaviour,
# they should only be regenerated with a build of ImPack2 that already passes all tests
# In addition to ImPack2, this script requires the following programs to be installed:
# - ImageMagick (convert), with support for PNG (always), JPEG2000, TIFF and WebP (if enabled)
# - Legacy ImPack (the executable should be saved as testdata/impack_legacy.exe) and Mono
# and the command line tools from
# - FLIF
# - jxrlib (JxrEncApp)
# - ffmpeg (for JPEG-LS)
# - libheif (heif-enc)
# - libavif (avifenc)
# - libjxl (cjxl)
# Due to the high number of extra requirements, the generated test files are included with ImPack2
# so you will not have to run this script before to run the tests.

IMPACK=../impack
IMPACK_LEGACY="mono impack_legacy.exe"
CONVERT=convert

PASSPHRASE="123456"

echo_and_run(){
	echo "$*"
	"$@"
}

rm -f input.bin
# Bytes from 0x00 to 0xff
for i in $(seq 0 255); do
	printf "\\x$(printf '%x' $i)" >> input.bin
done

# Normal image
echo_and_run $IMPACK -e -i input.bin -o valid.png
# Different combinations of color channels
echo_and_run $IMPACK -e -i input.bin --channel-red -o valid_channel_red.png
echo_and_run $IMPACK -e -i input.bin --channel-green -o valid_channel_green.png
echo_and_run $IMPACK -e -i input.bin --channel-blue -o valid_channel_blue.png
echo_and_run $IMPACK -e -i input.bin --channel-red --channel-green -o valid_channel_red_green.png
echo_and_run $IMPACK -e -i input.bin --channel-red --channel-blue -o valid_channel_red_blue.png
echo_and_run $IMPACK -e -i input.bin --channel-green --channel-blue -o valid_channel_green_blue.png
echo_and_run $IMPACK -e -i input.bin --grayscale -o valid_grayscale.png
# Encrypted
for c in aes camellia serpent twofish; do
	echo_and_run $IMPACK -e -i input.bin -c -p $PASSPHRASE --encryption-type $c --pbkdf2 -o valid_encrypted_${c}_pbkdf2.png
	echo_and_run $IMPACK -e -i input.bin -c -p $PASSPHRASE --encryption-type $c -o valid_encrypted_${c}_argon2.png
done
# Compressed
for z in brotli bzip2 deflate lzma2 zstd; do
	echo_and_run $IMPACK -e -i input.bin -z --compression-type $z -o valid_compressed_${z}.png
done
# Encrypted + compressed
for c in aes camellia serpent twofish; do
	for z in brotli bzip2 deflate lzma2 zstd; do
		# Use PBKDF2 since its always available when encryption is compiled in
		echo_and_run $IMPACK -e -i input.bin -c -p $PASSPHRASE --encryption-type $c -z --compression-type $z --pbkdf2 -o valid_encrypted_${c}_compressed_${z}.png
	done
done
# Legacy images
echo_and_run $IMPACK_LEGACY -q -y -e -i input.bin -o valid_legacy.png
echo_and_run $IMPACK_LEGACY -q -y -e -i input.bin -c -p $PASSPHRASE -o valid_legacy_encrypted.png
echo_and_run $IMPACK_LEGACY -q -y -e -i input.bin -z -o valid_legacy_compressed.png
echo_and_run $IMPACK_LEGACY -q -y -e -i input.bin -c -p $PASSPHRASE -z -o valid_legacy_encrypted_compressed.png

# Truncated image
echo_and_run $CONVERT valid.png -crop -5 invalid_truncated_1.png
echo_and_run $CONVERT valid.png -crop -1 invalid_truncated_2.png
echo_and_run $CONVERT valid.png -crop -0-5 invalid_truncated_3.png
echo_and_run $CONVERT valid.png -crop -0-1 invalid_truncated_4.png
for c in aes camellia serpent twofish; do
	echo_and_run $CONVERT valid_encrypted_${c}_pbkdf2.png -crop -0-5 invalid_encrypted_${c}_truncated.png
done
for z in brotli bzip2 deflate lzma2 zstd; do
	echo_and_run $CONVERT valid_compressed_${z}.png -crop -0-5 invalid_compressed_${z}_truncated.png
done
for c in aes camellia serpent twofish; do
	for z in brotli bzip2 deflate lzma2 zstd; do
		echo_and_run $CONVERT valid_encrypted_${c}_compressed_${z}.png -crop -0-5 invalid_encrypted_${c}_compressed_${z}_truncated.png
	done
done
echo_and_run $CONVERT valid_legacy.png -crop -0-5 invalid_legacy_truncated.png
echo_and_run $CONVERT valid_legacy_encrypted.png -crop -0-5 invalid_legacy_encrypted_truncated.png
echo_and_run $CONVERT valid_legacy_compressed.png -crop -0-5 invalid_legacy_compressed_truncated.png
echo_and_run $CONVERT valid_legacy_encrypted_compressed.png -crop -0-5 invalid_legacy_encrypted_compressed_truncated.png

# CRC
echo_and_run $CONVERT valid.png -fill "rgb(255,255,255)" -draw "color 9,0 point" invalid_crc.png
for c in aes camellia serpent twofish; do
	echo_and_run $CONVERT valid_encrypted_${c}_pbkdf2.png -fill "rgb(255,255,255)" -draw "color 9,0 point" invalid_encrypted_${c}_crc.png
done
for z in brotli bzip2 deflate lzma2 zstd; do
	echo_and_run $CONVERT valid_compressed_${z}.png -fill "rgb(255,255,255)" -draw "color 9,0 point" invalid_compressed_${z}_crc.png
done
for c in aes camellia serpent twofish; do
	for z in brotli bzip2 deflate lzma2 zstd; do
		echo_and_run $CONVERT valid_encrypted_${c}_compressed_${z}.png -fill "rgb(255,255,255)" -draw "color 9,0 point" invalid_encrypted_${c}_compressed_${z}_crc.png
	done
done
echo_and_run $CONVERT valid_legacy.png -fill "rgb(255,255,255)" -draw "color 5,1 point" invalid_legacy_crc.png

# Length
echo_and_run $CONVERT valid.png -fill "rgb(255,255,255)" -draw "color 5,0 point" invalid_length.png
for c in aes camellia serpent twofish; do
	echo_and_run $CONVERT valid_encrypted_${c}_pbkdf2.png -fill "rgb(255,255,255)" -draw "color 5,0 point" invalid_encrypted_${c}_length.png
done
for z in brotli bzip2 deflate lzma2 zstd; do
	echo_and_run $CONVERT valid_compressed_${z}.png -fill "rgb(255,255,255)" -draw "color 5,0 point" invalid_compressed_${z}_length.png
done
for c in aes camellia serpent twofish; do
	for z in brotli bzip2 deflate lzma2 zstd; do
		echo_and_run $CONVERT valid_encrypted_${c}_compressed_${z}.png -fill "rgb(255,255,255)" -draw "color 5,0 point" invalid_encrypted_${c}_compressed_${z}_length.png
	done
done
echo_and_run $CONVERT valid_legacy.png -fill "rgb(255,255,255)" -draw "color 4,0 point" invalid_legacy_length.png

# Data
echo_and_run $CONVERT valid.png -fill "rgb(255,255,255)" -draw "color 5,5 point" invalid_data.png
for c in aes camellia serpent twofish; do
	echo_and_run $CONVERT valid_encrypted_${c}_pbkdf2.png -fill "rgb(255,255,255)" -draw "color 5,5 point" invalid_encrypted_${c}_data.png
done
for z in brotli bzip2 deflate lzma2 zstd; do
	echo_and_run $CONVERT valid_compressed_${z}.png -fill "rgb(255,255,255)" -draw "color 5,5 point" invalid_compressed_${z}_data.png
done
for c in aes camellia serpent twofish; do
	for z in brotli bzip2 deflate lzma2 zstd; do
		echo_and_run $CONVERT valid_encrypted_${c}_compressed_${z}.png -fill "rgb(255,255,255)" -draw "color 5,5 point" invalid_encrypted_${c}_compressed_${z}_data.png
	done
done
echo_and_run $CONVERT valid_legacy.png -fill "rgb(255,255,255)" -draw "color 5,5 point" invalid_legacy_data.png
echo_and_run $CONVERT valid_legacy_encrypted.png -fill "rgb(255,255,255)" -draw "color 5,5 point" invalid_legacy_encrypted_data.png
echo_and_run $CONVERT valid_legacy_compressed.png -fill "rgb(255,255,255)" -draw "color 5,5 point" invalid_legacy_compressed_data.png
echo_and_run $CONVERT valid_legacy_encrypted_compressed.png -fill "rgb(255,255,255)" -draw "color 5,5 point" invalid_legacy_encrypted_compressed_data.png

for c in aes camellia serpent twofish; do
	echo_and_run $CONVERT valid_encrypted_${c}_pbkdf2.png -fill "rgb(255,255,255)" -draw "color 1,1 point" invalid_encrypted_${c}_iv.png
done

# Invalid magic number
echo_and_run $CONVERT valid.png -fill "rgb(255,255,255)" -draw "color 1,0 point" invalid_magic.png
echo_and_run $CONVERT valid_legacy.png -fill "rgb(255,255,255)" -draw "color 1,0 point" invalid_legacy_magic.png
# Channel selection that doesn't match the content
echo_and_run $CONVERT valid.png -fill "rgb(255,0,0)" -draw "color 0,0 point" invalid_channel_1.png
echo_and_run $CONVERT valid_channel_red.png -fill "rgb(255,255,255)" -draw "color 0,0 point" invalid_channel_2.png

for i in *.png; do
	echo_and_run $CONVERT $i -define webp:lossless=true ${i%png}webp
	echo_and_run $CONVERT $i ${i%png}tiff
	echo_and_run $CONVERT $i ${i%png}jp2
	echo_and_run $CONVERT $i -type TrueColor ${i%png}bmp3 # Don't write indexed BMPs
	echo_and_run mv ${i%png}bmp3 ${i%png}bmp
	echo_and_run flif --overwrite -e $i ${i%png}flif
	echo_and_run JxrEncApp -q 1 -i ${i%png}bmp -o ${i%png}jxr
	echo_and_run ffmpeg -loglevel -8 -y -i $i -c:v jpegls ${i%png}jls
	echo_and_run heif-enc $i -o ${i%png}heic -L -p chroma=444 --matrix_coefficients=0
	echo_and_run avifenc -l $i -o ${i%png}avif
	echo_and_run cjxl --quiet -d 0 $i ${i%png}jxl
done
