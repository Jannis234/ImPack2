# ImPack2

An application to convert arbitrary files into images and back

## About

ImPack2 allows you to store anything you want inside the RGB pixel data of an image, as long as the image is stored in a lossless format, like PNG for example.

Data can be encrypted and/or compressed before being stored inside the image. Additionally, ImPack2 includes the original filename for convenience and a CRC checksum to verify the data's integrity.

ImPack2 is a complete rewrite of my original ImPack code in C that adds new features and fixes design faults that the original program had. For backwards compatibility, it can extract old images created with the original ImPack.

## Building

Compiling ImPack2 should be as simple as running `make` and `sudo make install` to install the result.

Check the files `config_build.mak` and `config_system.mak` for any customizable settings. The GUI will automatically be built if GTK is enabled in `config_build.mak`.

### Dependencies

ImPack2 can make use of the following dependencies (can be toggled on or off in `config_build.mak`)
* [gtk3](https://www.gtk.org/) - For the ImPack2 GUI
* [nettle](http://www.lysator.liu.se/~nisse/nettle/) - For encryption
* [argon2](https://github.com/P-H-C/phc-winner-argon2) - For password hashing during encryption
* [charls](https://github.com/team-charls/charls) - To read/write JPEG-LS images
* [jxrlib](https://github.com/4creators/jxrlib) - To read/write JPEG XR images
* [libavif](https://github.com/AOMediaCodec/libavif) - To read/write AVIF images
* [libflif](https://flif.info/) - To read/write FLIF images
* [libheif](https://github.com/strukturag/libheif) - To read/write HEIF images
* [libjxl](https://github.com/libjxl/libjxl) - To read/write JPEG-XL images
* [libnsbmp](http://www.netsurf-browser.org/projects/libnsbmp/) - To read BMP images
* [libpng](http://www.libpng.org/) - To read/write PNG images
* [libtiff](http://libtiff.org/) - To read/write TIFF images
* [libwebp](https://chromium.googlesource.com/webm/libwebp/) - To read/write WebP images
* [openjpeg2](http://www.openjpeg.org/) - To read/write JPEG2000 images
* [brotli](https://github.com/google/brotli) - For Brotli compression
* [libbzip2](https://sourceware.org/bzip2/) - For Bzip2 compression
* [liblzma](https://tukaani.org/xz/) - For LZMA2 compression
* [zlib](https://zlib.net/) - For deflate compression
* [zstd](https://facebook.github.io/zstd/) - For Zstandard compression

If unsure, it is recommended to enable at least libpng to support a commonly used image format.
For compression, at least zstd and liblzma are recommended as zstd is the default choice when enabling compression and LZMA2 is suggested by the GUI for maximum compression.
For encryption, nettle is required and argon2 is highly recommended for higher security.

## Usage examples

Store a file:
```
impack -e -i ./myfile -o ./myimage.png
```
Extract a file:
```
impack -d -i ./myimage.png
```
For all available options, see:
```
impack --help
```
