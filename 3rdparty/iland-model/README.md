# iLand — Forest Landscape Model

iLand is an individual-based forest landscape and disturbance model that simulates how individual trees grow, compete, die, and regenerate under different climate, disturbance, and forest management scenarios.

Operating at scales ranging from watersheds to landscapes of 10,000+ hectares, iLand projects forest dynamics over decades and centuries. It supports the analysis of species composition, stand structure, carbon storage, climate change impacts, natural disturbances, and ecosystem services.

Originally developed as a C++ application using the Qt framework, iLand has been used by research groups worldwide and has contributed to numerous peer-reviewed scientific publications.

## About this port

This repository contains a modified version of iLand adapted to build as a shared library rather than a standalone Qt application.

Key differences from the original application:

- **Shared library:** The model is built as a library for integration into other applications.
- **No Qt:** All Qt dependencies have been removed.
- **Bundled dependencies:** All other required dependencies are included in the package.
- **Single entry point:** The library exposes `runilandmodel` as the entry point for running the model.

The goal of this port is to make the iLand model available for programmatic use and integration without requiring the original Qt application.

## Example build instructions

```cmd
cmake -S . -B build -G "Unix Makefiles"
cmake --build build
```

## Tests

The test suite includes a compatibility benchmark that runs this library and
official iLand 2.1 on the permafrost borehole model, then compares their output
databases. Their results currently match for this dataset. Running the
benchmark requires Qt to build the official iLand 2.1 application; See the
[benchmark instructions](tests/benchmarks/permafrost-borehole-v2.1/README.md).

## Original project and resources

This port is based on the original iLand forest landscape model.

- **Official website:** https://iland-model.org
- **Scientific publications:** https://iland-model.org/iLand+publications
- **Community Discord:** https://discord.gg/e5trneUKav

## License

iLand is distributed under the GNU General Public License (GPL). Please refer to the original project and the license files in this repository for applicable licensing terms.
