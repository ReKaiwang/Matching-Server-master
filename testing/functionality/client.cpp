#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>

/*   the host name and port number, for debugging only   */
/*   may change if server executing in another machine   */
#define SERVER_ADDR "vcm-2971.vm.duke.edu"
#define SERVER_PORT "12345"

#define MAX_THREAD  1
#define BUFF_SIZE   10240


// function which each thread will execute
void* handler (void* arg) {
  char buffer[BUFF_SIZE];
  int server_sfd;
  int server_port_num;
  int stat;
  int len;
  struct hostent* server_info;
  struct addrinfo host_info;
  struct addrinfo* host_info_list;
  
  server_info = gethostbyname(SERVER_ADDR);
  if (server_info == NULL) {
    std::cerr << "host not found\n";
    exit(1);
  }
  server_port_num = 12345; //atoi(SERVER_PORT);
  
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  stat = getaddrinfo(SERVER_ADDR, SERVER_PORT, &host_info, &host_info_list);
  
  // create socket
  server_sfd = socket(host_info_list->ai_family,
                      host_info_list->ai_socktype,
                      host_info_list->ai_protocol);
  int yes = 1;
  stat = setsockopt(server_sfd, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes));
  if (server_sfd < 0) {
    perror("socket");
    exit(server_sfd);
  }
  // connect to the proxy server
  stat = connect(server_sfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (stat < 0) {
    perror("server connect");
    exit(stat);
  }
  
  // XML request to be sent
  char* temp = (char*)arg;
  std::string file_path(temp);
  std::ifstream fs(file_path);
  std::string fcontent;
  std::stringstream ss;
  std::string req;
  
  if (!fs.fail()) {
    ss << fs.rdbuf();
    try {
    req = ss.str();
    }
    catch (std::exception& e) {
      std::cerr << "here... " << e.what() << std::endl;
    }
  }
  
  long long xml_len = req.length();
  std::string prefix = std::to_string(xml_len);
  prefix += "\n";
  req = prefix + req;
  clock_t t = clock();
  len = send(server_sfd, req.c_str(), req.length(), 0);
  stat = recv(server_sfd, buffer, BUFF_SIZE, 0);
  t = clock() - t;
  std::cout << "time: " << t << std::endl;
  std::cout << buffer << std::endl;
}



int main (int argc, char** argv) {
  clock_t t = clock();
  int threads[MAX_THREAD];
  pthread_attr_t thread_attr[MAX_THREAD];
  pthread_t thread_ids[MAX_THREAD];
  
  for (int i = 0; i < MAX_THREAD; ++i) {
    threads[i] = pthread_create(&thread_ids[i], NULL, handler, argv[1]);
    usleep(1000);
  }
  for (int i = 0; i < MAX_THREAD; ++i) {
    pthread_join(thread_ids[i], NULL);
  }
  t = clock() - t;
  std::cout << "total: " << t << std::endl;
  return 0;
}
