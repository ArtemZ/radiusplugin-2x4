
using namespace std;

struct ip_range cidr_to_range(char *inetnum);
int in_ini_range(char * ipaddr, int test = 0);
char * get_internal_ip(char *external_ip, int test=0);