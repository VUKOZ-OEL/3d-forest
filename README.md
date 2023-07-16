<div align="center">
<img width="100%" src="https://raw.githubusercontent.com/VUKOZ-OEL/3d-forest/main/doc/images/3d-forest-preview-screenshot.png"></a>
</div>

# 3D Forest
3D Forest is software for analysis of Lidar data from forest environment.

Copyright 2020-Present VUKOZ
Blue Cat team and other authors

## License
3D Forest is released under the GPLv3 license.
See [LICENSE](LICENSE) for more information.

## Documentation
HTML [Documentation](https://vukoz-oel.github.io/3d-forest-documentation/3d-forest-user-manual.html)
with User Manual and Developer Guide.

## Build
The code uses C++17, CMake, Qt5 or Qt6 and OpenGL.
```
    > mkdir build
    > cd build
    > cmake -G "MinGW Makefiles" .. -DCMAKE_INSTALL_PREFIX=..
    > mingw32-make
    > mingw32-make install
```

## Build Instructions
The code uses C++17 and CMake. Qt5 or Qt6 and OpenGL are required to build desktop application.
The build process generates desktop application with graphical user interface and command line tools.
See [INSTALL](INSTALL) for more information.

- [Windows MinGW](https://vukoz-oel.github.io/3d-forest-documentation/3d-forest-developer-guide.html#build-windows-make) build from source code
- [Windows Visual Studio](https://vukoz-oel.github.io/3d-forest-documentation/3d-forest-developer-guide.html#build-windows-visual-studio) build from source code
- [Linux](https://vukoz-oel.github.io/3d-forest-documentation/3d-forest-developer-guide.html#build-linux-make) build from source code
- [macOS / Mac OS X](https://vukoz-oel.github.io/3d-forest-documentation/3d-forest-developer-guide.html#build-macos-make) build from source code

## Third-Party Libraries
3D Forest source code includes several third-party libraries which are stored
under 3rdparty directory. This approach allows to use compatible versions of
third-party libraries **without downloading and installation** of each library.

- ctk widgets: ctkRangeSlider (Jul 27, 2018), A slider that has 2 input values.
- delaunator-cpp (Oct 6, 2018), Delaunay triangulation of 2D points.
- eigen (3.4.0), Template library for linear algebra.
- libigl (2.4.0), A simple geometry processing library.
- stb_image_write (v1.16), stb single-file public domain libraries for C++.
- octree (0.1-icra), Index-based Octree implementation.

## Known Issues
This software is currently in development.

# Support
## Links

Some useful links:

- [3D Forest web site](https://www.3dforest.eu/)

## Source Code sitemap
```
CMakeLists.txt     - CMake top-level file.
INSTALL            - Installation and building instructions.
LICENSE            - The GPL license.
README.md          - Project summary.

3rdparty/          - 3rd party libraries.
bin/               - 3D Forest binaries. CMake install destination directory.
build/             - CMake build directory.
cmake/             - CMake settings.
data/              - Example data files.
doc/               - Documentation.
src/               - Source code.
```