#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>

#define ESP_LOGD(type,msg,args...) printf(msg,args)
const uint8_t MAC_ADDR_STR_LEN = 6*2+5;

// Sample input "00:AA:BB:11:22:33"
int wol(std::string mac_str) {
    uint32_t mac_in[6];

    int res;
    res = sscanf(mac_str.c_str(), "%x:%x:%x:%x:%x:%x", &mac_in[0],&mac_in[1],&mac_in[2],&mac_in[3],&mac_in[4],&mac_in[5]);
    if (res < 6){
        ESP_LOGD("lambda","ERROR SSCANF - INVALID MAC FORMAT %s",mac_str.c_str());
        return -1;
    }
    uint8_t mac[6];
    int i;
    for (i=0; i<6; i++)
        mac[i] = static_cast<uint8_t>(mac_in[i]);

    int sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in destination, source;
    int broadcast = 1 ;
    
    res = setsockopt(sock, SOL_SOCKET, SO_BROADCAST,&broadcast, sizeof broadcast);
    if (res == -1){
        ESP_LOGD("lambda","ERROR SO_BROADCAST: %d", errno);
        ::close(sock);
        return -1;
    }

    uint8_t toSend[102];

    for (i=0; i<6; i++)
        toSend[i] = 0xFF;

    for (i=1; i<=16; i++)
        memcpy(&toSend[i*6], &mac, 6*sizeof(uint8_t));
    

    destination.sin_family = AF_INET;
    destination.sin_addr.s_addr = -1;
    destination.sin_port = htons(40000);

    int n_bytes = ::sendto(sock, &toSend, sizeof(toSend), 0, reinterpret_cast<sockaddr*>(&destination), sizeof(destination));
    ESP_LOGD("lambda", "Sent WOL to %s\n", mac_str.c_str());
    ::close(sock);
    return 0;
}

int main (int argc, char** argv){
    if (argc!=2) {
        printf("Wrong number of arguments\n");
        exit(EXIT_FAILURE);
    };
    if (strlen(argv[1]) != MAC_ADDR_STR_LEN){
        printf("Wrong size of arguments\n");
        exit(EXIT_FAILURE);
    }
    uint32_t mac_in[MAC_ADDR_STR_LEN];
    int res = sscanf(argv[1],"%x:%x:%x:%x:%x:%x",&mac_in[0],&mac_in[1],&mac_in[2],&mac_in[3],&mac_in[4],&mac_in[5]);
    if (res!=6) {
        printf("Invalid input format\n");
        exit(EXIT_FAILURE);
    };
    char buf[17];
    sprintf(buf,"%x:%x:%x:%x:%x:%x",mac_in[0],mac_in[1],mac_in[2],mac_in[3],mac_in[4],mac_in[5]);
    wol(buf);
}