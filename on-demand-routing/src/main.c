#include "tableDDT.h"

int main(int argc, char *argv[])
{

	int ch;
	char dest_ip[100];
	char nexth_ip[100];
	int hc,rc;
	

	do
	{
		printf("\n1.ADD \n2.DELETE \n3.RETREIVE\n\t ENTER : ");
		scanf("%d",&ch);

		getchar();

		memset(dest_ip,0,sizeof(dest_ip));
		memset(nexth_ip,0,sizeof(nexth_ip));

		switch(ch)
		{
			case 1:
				printf("\nDEST IP : ");
				scanf("%s",dest_ip);
				printf("\nNHOP IP : ");
				scanf("%s",nexth_ip);
				printf("\nHOP COUNT : ");
				scanf("%d",&hc);
				
				routeInfo rtinfo;
				memset(&rtinfo,0,sizeof(rtinfo));
			
				strcpy(rtinfo.dest_ipaddr,dest_ip);
				strcpy(rtinfo.nexthop_ip,nexth_ip);
				rtinfo.hopCount = hc;
				
				rc = addrouteInfo(&rtinfo);
				if(rc == 1)
					printf("\nROUTE INFO ADDED TO THE LIST\n");
				else if(rc == 2)
					printf("\nROUTE INFO UPDATED \n");
				else
					printf("\nERROR : FAILED TO INSERT ROUTE INFO\n");
		
				break;
			
			case 2:
				printf("\nDEST IP : ");
				scanf("%s",dest_ip);
				destroyRouteInformation(dest_ip);
				break;
			case 3:
				printf("\nDEST IP : ");
				scanf("%s",dest_ip);
				routeInfo *prtinfo;
				prtinfo = getrouteInfo(dest_ip);
				if(prtinfo)
				{
				printf("DEST IP : %s\n",prtinfo->dest_ipaddr);
				printf("NEXT HOP IP : %s\n",prtinfo->nexthop_ip);
				printf("HOP COUNT : %d\n",prtinfo->hopCount);
				free(prtinfo);
				}
				else
					printf("NO ROUTE FOUND FOR IP : %s",dest_ip);
				break;

			default:
				break;
				
		
		}
		
		
	}while(ch<=3);

	destroyList();
	return 0;
}
