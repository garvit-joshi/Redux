# Redux
A cross-platform Application for storing User-Data.

![Ubuntu](https://github.com/garvit-joshi/Redux/workflows/Ubuntu/badge.svg)
![Windows](https://github.com/garvit-joshi/Redux/workflows/Windows/badge.svg)

## Building The App from Source(Linux): 🔨
### Prerequisites:
    
* CMake >= 3.22 <br>
* g++ >= 8 (CXX standard=17) <br>
* [vcpkg](https://github.com/microsoft/vcpkg)

### Steps:  
1. Download [vcpkg](https://github.com/microsoft/vcpkg) and run 
```
./bootstrap-vcpkg.sh
```
2. Download Redux and run this command assuming vcpkg is installed in ``` home/username/Repos/vcpkg ``` and Redux is located ``` /home/username/Repos/Redux/ ```

```
cmake -B /home/username/Repos/Redux/build -S . -DCMAKE_TOOLCHAIN_FILE=/home/username/Repos/vcpkg/scripts/buildsystems/vcpkg.cmake
```

Redux ships a `vcpkg.json` manifest, so the toolchain file installs Crypto++ itself at configure
time -- there is no separate `vcpkg install` step to run first.

3. To compile the project run:
```
cmake --build /home/username/Repos/Redux/build
```
4. Run The Executable:
``` 
./build/src/Redux
```



## Building The App from Source(Windows): 🔨
### Prerequisites:
    
* CMake >= 3.22 
* Microsoft Visual Studio 2022 (v17) -- matches the compiler CI builds with
* [vcpkg](https://github.com/microsoft/vcpkg)

### Steps:
1. Download [vcpkg](https://github.com/microsoft/vcpkg) and run ```/bootstrap-vcpkg.bat```

2. Opening cmd in root directory of Redux, and run these commands assuming your vcpkg is installed in ```C://vcpkg``` and Redux is located in ```D:\Repos\Redux```:
    ```
    cmake -G "Visual Studio 17 2022" -A Win32 -S . -B "build32" -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
    cmake --build build32 --config Release
    ```

    As on Linux, the `vcpkg.json` manifest is what pulls in Crypto++ -- the toolchain file installs
    it (for whichever triplet CMake is configuring) during the first command above, so there is no
    `vcpkg install cryptopp:...` step to run separately.

3. Binaries will be at ```D:\Repos\Redux\build32\src\Release\Redux.exe```

## Notes (if using Pre-Build Binaries for Windows):

1. Please Install **Microsoft Visual C++ Redistributable for Visual Studio 2022** before running binaries:
    1. For [x64](https://aka.ms/vs/17/release/VC_redist.x64.exe),
    2. For [x86](https://aka.ms/vs/17/release/VC_redist.x86.exe),
    3. For [ARM64](https://aka.ms/vs/17/release/VC_redist.arm64.exe).

## Testing

```
ctest --test-dir build --output-on-failure
```

The regression suite never touches a developer's real config directory: it relocates storage by
setting `REDUX_CONFIG_DIR`, which every build of Redux -- not just the tests -- checks before
falling back to the platform default.

## Vault format

Vaults are encrypted with scrypt-derived AES-256-GCM keys. The on-disk format is versioned, and
this is a deliberate break: vaults written by pre-rework versions of Redux are not readable by
this version, and there is no migration path.
