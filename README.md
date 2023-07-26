
[![linux](https://github.com/tractatus/mmCoreAndDevices/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/tractatus/mmCoreAndDevices/actions/workflows/ubuntu.yml) [![macos](https://github.com/tractatus/mmCoreAndDevices/actions/workflows/crosscompile.yml/badge.svg)](https://github.com/tractatus/mmCoreAndDevices/actions/workflows/crosscompile.yml) [![windows mingw](https://github.com/tractatus/mmCoreAndDevices/actions/workflows/windows.yml/badge.svg)](https://github.com/tractatus/mmCoreAndDevices/actions/workflows/windows.yml) [![windows msvc](https://github.com/tractatus/mmCoreAndDevices/actions/workflows/crosscompile.yml/badge.svg)](https://github.com/tractatus/mmCoreAndDevices/actions/workflows/crosscompile.yml)

# mmCoreAndDevices
The c++ code at the core of the Micro-Manager project.

## API Docs
[Main Page](https://micro-manager.org/apidoc/MMCore/latest/index.html)

If you are using a scripting language to control a microscope through the CMMCore object
then you are likely looking for the [CMMCore API](https://micro-manager.org/apidoc/MMCore/latest/class_c_m_m_core.html)

## Build

I recommend building inside a conda environment:

```
conda create --name mmcore
conda activate mmcore   
```

Then:
```
cmake --build . --clean-first
```
