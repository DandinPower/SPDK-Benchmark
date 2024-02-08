# SPDK-Benchmark

## Installations

### Install SPDK

1. Clone SPDK repository

    ```bash
    git submodule update --init
    ```

2. Install SPDK dependencies

    ```bash
    cd spdk
    git submodule update --init
    sudo ./scripts/pkgdep.sh
    ```

3. Build SPDK to shared library

    ```bash
    sudo ./configure --with-shared
    sudo make -j 32
    ldconfig -v -n ./build/lib ./dpdk/build/lib
    ## Start SPDK unit test
    sudo ./test/unit/unittest.sh
    ```

4. Copy the shared library to /usr/local/lib

    ```bash
    sudo mkdir -p /usr/local/lib/spdk
    sudo mkdir -p /usr/local/lib/dpdk
    sudo cp -r build/lib/* /usr/local/lib/spdk
    sudo cp -r dpdk/build/lib/* /usr/local/lib/dpdk
    ```

5. Add the shared library path to ldconfig

    ```bash 
    sudo nano /etc/ld.so.conf.d/spdk_dpdk.conf
    ```

    - add the following lines to the file

        ```bash
        /usr/local/lib/spdk
        /usr/local/lib/dpdk
        ```

    - run the following command to update the ldconfig

        ```bash
        sudo ldconfig
        ```


6. Configure NVME device for SPDK

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

7. Run Hello World example

    ```bash
    sudo build/examples/hello_world
    ```

### Install SPDK-Benchmark

1. Build SPDK-Benchmark

    ```bash
    cd SPDK-Benchmark
    make
    ```

2. Run SPDK-Benchmark

    ```bash
    sudo ./main
    ```

3. Note

    - if you encounter the problem that can't find shared library, you can use the following command to add the shared library path

        ```bash
        export LD_LIBRARY_PATH=/usr/local/lib/spdk:/usr/local/lib/dpdk
        ```

    - and then run the program with LD_LIBRARY_PATH

        ```bash
        sudo LD_LIBRARY_PATH=$LD_LIBRARY_PATH ./main
        ```