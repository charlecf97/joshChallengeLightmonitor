# joshChallenge

A C++ project targeting C++14. This README provides an overview, prerequisites, build and run instructions for Visual Studio 2022 and command-line builds, and a license placeholder.

This project is designed as a solution to the coding challenge [here](https://github.com/jstarllc/JoshCodingChallenge).

## Prerequisites

- Windows 10 or later
- Visual Studio 2022 with the "Desktop development with C++" workload
- C++14-compatible toolset (MSVC toolset included with VS2022)
- (Optional) CMake and a command-line build environment if using CMake

## Build (Visual Studio)

1. Open `joshChallenge.sln` (or the solution file in the repository) in Visual Studio 2022.
2. Select the desired configuration (Debug/Release) and platform (x86/x64).
3. Build the solution: `Build > Build Solution` or press `Ctrl+Shift+B`.

## Build (Command Line)

- Using MSBuild:

  msbuild joshChallenge.sln /p:Configuration=Release

- If the project uses CMake, from the repository root:

  mkdir build
  cd build
  cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
  cmake --build . --config Release

## Run

- From Visual Studio: Run the project with `F5` (Debug) or `Ctrl+F5` (Run without debugging).
- From the command line: execute the built binary found under `\x64`,`\bin`, `\build`, or `\Release`/`\Debug` folders depending on your configuration.

The program communicate with localhost on port 8080. Running the simulator is recommended for testing. The simulator can be found [here](https://github.com/jstarllc/JoshCodingChallenge/releases).
