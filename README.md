# fill-table-trigger
## clone this project
```
git clone https://github.com/docpenyu/fill-table-trigger.git
```

## build & run this project
```
cd fill-table-trigger
cmake -B build
cmake --build build
```

then the 'main' executable file will be compiled under the build directory.
```
main -h # show the help
main -c channel_number -r hit_rate_per_channel -n trigger_threshold # the type of these three parameter must be integer
```
