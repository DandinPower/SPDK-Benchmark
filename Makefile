CXXFLAGS = -std=c++11 -Iinclude -Ispdk/build/include -Ispdk/dpdk/build/include -Wl,--whole-archive
LDFLAGS = -Lspdk/build/lib -Lspdk/dpdk/build/lib -lspdk_nvme -lspdk_vmd -lspdk_env_dpdk -lspdk_log -lspdk_util \
-lrte_eal -lrte_mempool -lrte_telemetry -lrte_ring -lrte_kvargs -lrte_bus_pci -lrte_pci -luuid -lnuma -lssl -lcrypto -Wl,--no-whole-archive

SRCS = src/logger.cpp src/utils.cpp src/spdk_utils.cpp main.cpp 
OBJS = $(SRCS:.cpp=.o)

TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)
	rm -f $(OBJS)