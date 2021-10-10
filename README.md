# Redux
A cross-platform Application for storing User-Data.

![Ubuntu](https://github.com/garvit-joshi/Redux/workflows/Ubuntu/badge.svg)
![Windows](https://github.com/garvit-joshi/Redux/workflows/Windows/badge.svg)
[![Open in Visual Studio Code](https://open.vscode.dev/badges/open-in-vscode.svg)](https://open.vscode.dev/garvit-joshi/Redux)

## Building The App from Source(Linux): 🔨
### Prerequisites:
    
* CMake >= 3.1 <br>
* g++ >= 8 (CXX standard=17) <br>
* [vcpkg](https://github.com/microsoft/vcpkg)

### Steps:  
1. Download [vcpkg](https://github.com/microsoft/vcpkg) and run 
```
./bootstrap-vcpkg.sh
```
2. To install the libraries for your project, run the below command in the root directory of vcpkg: 
```bash
./vcpkg install cryptopp
```
3. Download Redux and run this command assuming vcpkg is installed in ``` home/username/Repos/vcpkg ``` and Redux is located ``` /home/username/Repos/Redux/ ```

```
cmake -B /home/username/Repos/Redux/build -S . -DCMAKE_TOOLCHAIN_FILE=/home/username/Repos/vcpkg/scripts/buildsystems/vcpkg.cmake
```

4. To compile the project run:
```
cmake --build /home/username/Repos/Redux/build
```
5. Run The Executable:
``` 
./build/src/Redux
```



## Building The App from Source(Windows): 🔨
### Prerequisites:
    
* CMake >= 3.1 
* Microsoft Visual Studio 2019
* [vcpkg](https://github.com/microsoft/vcpkg)

### Steps:
1. Download [vcpkg](https://github.com/microsoft/vcpkg) and run ```/bootstrap-vcpkg.bat```

2. For installing required modules, run one of these commands in the root directory of vcpkg:

    * ```vcpkg install cryptopp:x64-windows``` (For 64-bit PC)
    * ```vcpkg install cryptopp:x86-windows``` (For 32-bit PC)

3. Opening cmd in root directory of Redux, and run these commands assuming your vcpkg is installed in ```C://vcpkg``` and Redux is located in ```D:\Repos\Redux```:
    ```
    cmake -G "Visual Studio 16 2019" -A Win32 -S . -B "build32" -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
    cmake --build build32 --config Release
    ```

4. Binaries will be at ```D:\Repos\Redux\build32\src\Release\Redux.exe```

## Notes (if using Pre-Build Binaries for Windows):

1. Please Install **Microsoft Visual C++ Redistributable for Visual Studio 2019** before running binaries:
    1. For [x64](https://aka.ms/vs/16/release/VC_redist.x64.exe),
    2. For [x86](https://aka.ms/vs/16/release/VC_redist.x86.exe),
    3. For [ARM64](https://aka.ms/vs/16/release/VC_redist.arm64.exe).

2. The last thing is simply a matter of ***perception***. If you are running any sort of anti-virus, like ***ZoneAlarm, Norton, McAfee, etc***. then they will get a very unpleasant message about your program trying to do something considered ***dangerous***. It may be due to ***system(); function*** used in program. Read more about it [here](http://www.cplusplus.com/reference/cstdlib/system/) and [here](http://www.cplusplus.com/articles/j3wTURfi/)
