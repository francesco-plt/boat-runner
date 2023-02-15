# Boat Runner

This repository contains the final project for the "Computer Graphics" course at Politecnico di Milano. The project is a simulation of a boat floating in water which must avoid hitting rocks on its path.

<img width="912" alt="Screenshot 2023-02-15 alle 22 22 29" src="https://user-images.githubusercontent.com/26849744/219170801-24e0c3c7-cc8b-4df3-bd16-3a31c4a2db6e.png">

## Disclaimer

* Please note that this project was developed as a student project and is not intended for commercial use. The project may contain bugs or unfinished features and is provided as is.
* Additionally, the project is for educational purposes only and should not be used for any other purposes.
* Use it at your own risk.
* Feel free to use the code and assets as a reference for your own projects, but please do not copy or redistribute them without giving proper credit.

## How to compile

In order to compile the project, you need to follow this tutorial: [Development environment - Vulkan Tutorial (vulkan-tutorial.com)](https://vulkan-tutorial.com/Development_environment).

Once you're done, you can `cd` into any folder and run the following if you're on Linux:

```shell
$ make -j$(nproc) clean build
```

Or, if you're on macOS:

```shell
$ make -j(sysctl -a | grep machdep.cpu | awk '/machdep.cpu.core_count/ {print $2}')
```

If you're on Windows, you may need to reevaluate your priorities.

## Running the project

Once you have compiled it, you can just fire up a shell, `cd` into the project directory and run:

``` shell
$ cd BoatRunner
$ ./build/BoatRunner
```
