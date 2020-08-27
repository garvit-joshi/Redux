# Redux
A cross-platform Application for storing User-Data.

![Ubuntu](https://github.com/garvit-joshi/Redux/workflows/Ubuntu/badge.svg)
![Windows](https://github.com/garvit-joshi/Redux/workflows/Windows/badge.svg)

## Building The App from Source(Linux): 🔨
### Prerequisites:
    
 1. CMake >= 3.1 <br>
 2. g++ >= 8 (CXX standard=17) <br>
 3. [vcpkg](https://github.com/microsoft/vcpkg)

### Steps:  
1. Download [vcpkg](https://github.com/microsoft/vcpkg) and run 
```
/bootstrap-vcpkg.sh
```
2. To install the libraries for your project, run: 
```
./vcpkg install cryptopp
```
3. Download Redux and run this command assuming vcpkg is installed in ``` home/username/Repos/vcpkg ``` and Redux is located ``` /home/username/Repos/Redux/ ```

```
cmake -B /home/username/Repos/Redux/build -S . -DCMAKE_TOOLCHAIN_FILE=/home/username/Repos/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_CXX_COMPILER=g++-10
```

4. To compile the project run:
```
cmake --build /home/username/Repos/Redux/build
```
5. Run The Executable:
``` 
./build/src/app 
```



## Building The App from Source(Windows): 🔨
### Prerequisites:
    
1. CMake >= 3.1 
2. Microsoft Visual Studio C++ Compiler
3. [vcpkg](https://github.com/microsoft/vcpkg)

### Steps:

