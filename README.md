# 3D Forest
3D Forest is software for analysis of Lidar data from forest environment.

Copyright 2020-2022 VUKOZ
Blue Cat team and other authors

## License
3D Forest is released under the GPLv3 license.
See [LICENSE](LICENSE) for more information.

## Documentation
HTML [Documentation](https://vukoz-oel.github.io/3dforest2-documentation/)
with User Manual and Developer Guide.

## Installation Instructions
The code uses C++17 and CMake. Qt5 or Qt6 and OpenGL are required to build desktop application.
The build process generates desktop application with graphical user interface and command line utilities.
See [INSTALL](INSTALL) for more information.

- [Windows](https://vukoz-oel.github.io/3dforest2-documentation/developerguide_build_windows_make.html) build from source code
- [Linux](https://vukoz-oel.github.io/3dforest2-documentation/developerguide_build_linux_make.html) build from source code

## Third-Party Libraries
3D Forest source code includes several third-party libraries which are stored
under 3rdparty directory. This approach allows to use compatible versions of
third-party libraries **without downloading and installation** of each library.

- ctk widgets: ctkRangeSlider (Jul 27, 2018), A slider that has 2 input values.
- delaunator-cpp (Oct 6, 2018), Delaunay triangulation of 2D points.
- eigen (3.4.0), Template library for linear algebra.
- libigl (2.4.0), A simple geometry processing library.
- icons from [Icons8](https://icons8.com/)

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