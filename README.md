# ImPack2

An application to convert arbitrary files into images and back

## About

ImPack2 allows you to store anything you want inside the RGB pixel data of an image, as long as the image is stored in a lossless format, like PNG for example.

Data can be encrypted and/or compressed before being stored inside the image. Additionally, ImPack2 includes the original filename for convenience and a CRC checksum to verify the data's integrity.

ImPack2 is a complete rewrite of my original ImPack code in C that adds new features and fixes design faults that the original program had. For backwards compatibility, it can extract old images created with the original ImPack.

## Building

Compiling ImPack2 should be as simple as running `make` and `sudo make install` to install the resulting binary.

Check the files `config_build.mak` and `config_system.mak` for any customizable settings.

### Dependencies

ImPack2 can make use of the following dependencies (can be toggled on or off in `config_build.mak`)
* [libpng](http://www.libpng.org/) - To read/write PNG images
* [libwebp](https://chromium.googlesource.com/webm/libwebp/) - To read/write WebP images
* [nettle](http://www.lysator.liu.se/~nisse/nettle/) - For encryption
* [zstd](https://facebook.github.io/zstd/) - For Zstandard compression
* [zlib](https://zlib.net/) - For deflate compression

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
