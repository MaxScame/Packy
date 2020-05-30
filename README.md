# Packy📦

Some info about our project.
3D Bin Packing Algorithm, special for **Hack In Home** Hackathon 2020, KSU MSTU Stankin
Track: for BoschRexroth


## Features

- EB-AFIT Pallet Loading Algorithm
- Simple JSON export
- Documentation `(!)`

## Usage

*Packy* hasn't installation version, just script launch. And supports **only x32** Python 3.

All `.dll` and `.so` already precompiled, but you may do it [yourself](#compile-library).

### Windows

```
python packy.py
```

Linux now is not much tested, but you may try

### Linux

```
python3 packy.py
```

## Compile library

**TL;DR:**

### Windows

Just use *Microsoft Visual Studio 2019* with C/C++ Development Extension, project already in this repository. 

### Linux

Didn't tested, but try compile `main.cpp` file from *VS2019* project with `g++` with flags, maybe something like `-Wl`, but remember about [name mangling](https://en.wikipedia.org/wiki/Name_mangling). Sorry, idk `¯\_(ツ)_/¯`

```bash
 # something like that
g++ -shared -Wl,-soname,3d_packer -o 3d_packer.so -fPIC main.c
```

## Stack of technology

#### Algorithm: `C/C++`

#### Back-end: `Python 3, Flask, Jinja 2`

## Our Team

- Maxim N.
- Maxim S.
- Roman K.

## TODO

- [x] Transferring the computation to a low-level language (`C/C++`)
- [x] x64 support
- [x] Report generation
- [ ] Options window
- [ ] 3D visualization
- [ ] Support of box weight
- [ ] Report history
- [ ] Prettify design
- [ ] Non-metric system support
- [ ] `Chill👌`

Thanks
------

- Ehran Baltacıoğlu

  Who described and documented a heuristic algorithm for finding one of the best way to pack boxes into a pallet of given dimensions

- [Bill Knechtel](https://github.com/wknechtel/3d-bin-pack)

  Acknowledgement for publishing the original [document](doc/AirForceBinPacking.pdf) as well as the program code from it

