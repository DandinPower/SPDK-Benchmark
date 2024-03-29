CXXFLAGS = -std=c++11 -Iinclude -Ispdk/build/include -Ispdk/dpdk/build/include 
LDFLAGS = -L/usr/local/lib/spdk -L/usr/local/lib/dpdk -lspdk_nvme -lspdk_vmd -lspdk_env_dpdk -lspdk_log -lspdk_util \
-lrte_eal -lrte_mempool -lrte_telemetry -lrte_ring -lrte_kvargs -lrte_bus_pci -lrte_pci -luuid -lnuma -lssl -lcrypto

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