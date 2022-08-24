# fill-table-trigger
## clone this project
```
git clone https://github.com/docpenyu/fill-table-trigger.git
```

## build & run this project
this project uses the `c++11` standard, you must compile it with a compiler which supports `c++11`.
you must have `CMake` tool.

In Linux environment
```
cd fill-table-trigger
cmake -B build
cmake --build build
```

In Windows, the MinGW is recommended
```
cmake -B build -G "MinGW Makefiles"
cmake --build build
```

Maybe you will see this Waring about the find_package, "cannot find the benchmarkConfig.cmake"
It doesn't matter. you don't need to install the benchmark lib.

After compiled, then the 'main' executable file will be compiled under the `build` directory.
```
main -h # show the help
main -c channel_number -r hit_rate_per_channel -n trigger_threshold # the type of these three parameter must be integer
```
