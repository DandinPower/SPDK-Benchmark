# SPDK-Benchmark

## Installations

### Install SPDK

1. clone spdk repo

    ```bash
    git submodule update --init
    ```

2. Install SPDK dependencies

    ```bash
    cd spdk
    git submodule update --init
    sudo ./scripts/pkgdep.sh
    ```

3. Build SPDK

    ```bash
    sudo ./configure --with-shared
    sudo make -j 16
    ldconfig -v -n ./build/lib ./dpdk/build/lib
    ## Start SPDK unit test
    sudo ./test/unit/unittest.sh
    ```

4. Configure NVME device for SPDK

    ```bash
    sudo scripts/setup.sh
    ```

    - if you encounter the following error
        
        `0000:03:00.0 (c0a9 5412): Active devices: data@nvme1n1, so not binding PCI dev`

    - please ensure that the device is not mounted or used by other applications.

    - after the experiment, you can use the following command to unbind the device

        ```bash
        sudo scripts/setup.sh reset
        ```

5. Run Hello World example

    ```bash
    sudo LD_LIBRARY_PATH=build/lib:dpdk/build/lib build/examples/hello_world
    ```

### Install SPDK-Benchmark

1. Build SPDK-Benchmark

    ```bash
    cd SPDK-Benchmark
    export LD_LIBRARY_PATH=spdk/build/lib:spdk/dpdk/build/lib
    make
    ```

2. Run SPDK-Benchmark

    ```bash
    sudo LD_LIBRARY_PATH=spdk/build/lib:spdk/dpdk/build/lib ./main
    ```