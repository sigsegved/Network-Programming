#include "odr_client.h"
#include "../inc/hw_addrs.h"

typedef struct {
	char* ip;
} vm_addr;

int main()
{
  int i, vm_me = -1;
  struct in_addr comp_addr;
  struct hwa_info *hwahead = NULL, *hwa;
  
  vm_addr vm[10];
  vm[0].ip = "192.168.1.101";
  vm[1].ip = "192.168.1.102";
  vm[2].ip = "192.168.1.103";
  vm[3].ip = "192.168.1.104";
  vm[4].ip = "192.168.1.105";
  vm[5].ip = "192.168.1.106";
  vm[6].ip = "192.168.1.107";
  vm[7].ip = "192.168.1.108";
  vm[8].ip = "192.168.1.109";
  vm[9].ip = "192.168.1.110";
  
  hwahead = get_hw_addrs();
  
  if(hwahead == NULL)
    {
      printf("ODR: hwahead = NULL\n");
      exit(1);
    }
  
  for (hwa = hwahead; hwa != NULL; hwa = hwa->hwa_next) 
    {
      if(strlen(hwa->if_name) > 3)
	{
	  if(hwa->if_name[3] == '0')
	    {
	      for(i = 0; i < 10; ++i)
		{
		  inet_pton(AF_INET, vm[i].ip, &comp_addr); 
		  if(((struct sockaddr_in *)hwa->ip_addr)->sin_addr.s_addr == comp_addr.s_addr)
		    {
		      vm_me = i + 1;
		      break;
		    }
		}
	    }
	}
    }
  
  
  int sock = client_init(0);
  
  while (1) {
    
    printf("Available VM's:\n");
    
    for (i = 0; (i < 10); i++) {
      if( i == vm_me - 1 )
	printf("\t%d: %s <- Me\n", (i + 1), vm[i].ip);
      else
	printf("\t%d: %s\n", (i + 1), vm[i].ip);
    }
    printf("\nPick a VM # to get time from or 0 to quit: ");
    
    int target_vm;
    scanf("%d", &target_vm);
    
    if(target_vm == 0)
      {
	break;
      }
    
    if (target_vm >= 1 || target_vm <= 10) {
      msg_send(sock, vm[target_vm - 1].ip, 123, "Got time?", 0);
      printf("Client at VM%i: sending request to server at VM%d\n", vm_me, target_vm);
      
      char sourceip[20];
      int port = 0;
      char buff[1024];
	  int ret = msg_recv(sock, sourceip, &port, buff);
	  if (ret == RECV_OK) {
		printf("Client at VM%i: received from %s: %s\n", vm_me, sourceip, buff);
	  }
	  else if (ret == RECV_TIMEOUT) {
		printf("Timed out.\n");
	  }
	  else {
		printf("Error on Recv\n");
	  }
	  
      
    }
    else {
      printf("Invalid VM #, exiting.");
      break;
    }
  }
  
  client_cleanup(sock);
  return 0;
}
