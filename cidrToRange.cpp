#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>

#include "simpleini/SimpleIni.h"

/*
    Struct ip_range contains first and last IP in range in network byte order
    Convert to host byte order using ntohl()
*/
struct ip_range
{

    unsigned long first;
    unsigned long last;
    
};
struct ip_range cidr_to_range(char *inetnum)
{
    struct ip_range range;

    unsigned int ip[4],ip_min[4],ip_max[4],netmask,tmp1,tmp2;
    sscanf(inetnum,"%d.%u.%u.%u/%u",&ip[0],&ip[1],&ip[2],&ip[3],&netmask);
    

    /*Set the bytes which won't be changed*/
    for (tmp1=tmp2=netmask/8;tmp1>0;tmp1--){
	ip_min[tmp1-1] = ip[tmp1-1];
	ip_max[tmp1-1] = ip[tmp1-1];
    }

    /*Set the bytes which should be 0ed or 255ed.*/
   for(tmp1 = tmp2,++tmp2;tmp2< 4;tmp2++){
	ip_min[tmp2]=0;
	ip_max[tmp2]=255;
    }

    /* Finally set the one which has to be shifted.*/
    if( tmp1 < 4){
	tmp2 = 8-netmask%8;
	ip_min[tmp1]=ip[tmp1]>>tmp2;
	ip_min[tmp1]<<=tmp2;
	ip_max[tmp1]=ip_min[tmp1]+pow(2,tmp2)-1;
    }
    char *firstBuf = new char[29];
    char *lastBuf = new char[29];
    
    sprintf(firstBuf,"%u.%u.%u.%u",ip_min[0],ip_min[1],ip_min[2],ip_min[3]);
    sprintf(lastBuf,"%u.%u.%u.%u",ip_max[0],ip_max[1],ip_max[2],ip_max[3]);

    inet_pton(AF_INET,firstBuf,&range.first);
    inet_pton(AF_INET,lastBuf,&range.last);
    //range.first = firstBuf;
    //range.last = lastBuf;
    return range;
}
char *get_ip_from_second_range(char *ipaddr, struct ip_range firstRange, struct ip_range secondRange){
    char ipbuf[INET_ADDRSTRLEN];
    char *second_range_ip = (char*)malloc(29);
    int first_ip_index;
    int count = 0;
    unsigned int currentIp;
    for(int a = ntohl(firstRange.first); a <= ntohl(firstRange.last); a = a + 1){
	currentIp = htonl((unsigned int )a);
	if(strcmp(ipaddr,inet_ntop(AF_INET, &currentIp, ipbuf, sizeof(ipbuf) )) == 0 ){
		first_ip_index = count;
	}
	count++;
    }
    count = 0;
    char ipbuf2[INET_ADDRSTRLEN];
//    printf("first ip index: %u",first_ip_index);
    for(int a = ntohl(secondRange.first); a <= ntohl(secondRange.last); a = a + 1){
	currentIp = htonl((unsigned int )a);
	if(count == first_ip_index){
		strcpy(second_range_ip, inet_ntop(AF_INET, &currentIp, ipbuf2, sizeof(ipbuf)));
	}
	count++;
    }
    //printf("%s", second_range_ip);
    return second_range_ip;
}
int check_ip_in_range(char *ipaddr, struct ip_range range){
    long ip_in_nbo;
    inet_pton(AF_INET, ipaddr, &ip_in_nbo);
    if(ntohl(ip_in_nbo) > ntohl(range.first) && ntohl(ip_in_nbo) < ntohl(range.last)){
	return 1;
    }
    return 0;
}
char *get_internal_ip(char *external_ip, int test = 0){
    char *internal_ip = (char *)malloc(29);
    CSimpleIniA ini;
    ini.SetUnicode();
    if(test == 0){
	ini.LoadFile("/etc/openvpn/ipranges.ini");
    } else {
	ini.LoadFile("myfile.ini");
    }
    // get all sections
    CSimpleIniA::TNamesDepend sections;
    CSimpleIniA::TNamesDepend::const_iterator it;
    ini.GetAllSections(sections);

    for(it = sections.begin(); it != sections.end(); ++it)
    {
	char *section = (char *)it->pItem;
	if(check_ip_in_range(external_ip, cidr_to_range(section)) == 1){
		//printf("internal range %s", ini.GetValue(section,"internal.range","default"));
		internal_ip = get_ip_from_second_range(external_ip, cidr_to_range(section),  cidr_to_range((char *)ini.GetValue(section,"internal.range","default")));
	}
    }
    return internal_ip;
}
/*
    Checks if IP belongs to an external range (section) in ini configuration file
    Returns 0 if not belongs
    Returns 1 if belongs
*/
int in_ini_range(char *ipaddr, int test = 0){
    CSimpleIniA ini;
    ini.SetUnicode();
    int result = 0;
    if(test == 0){
	ini.LoadFile("/etc/openvpn/ipranges.ini");
    } else {
	ini.LoadFile("myfile.ini");
    }
    CSimpleIniA::TNamesDepend sections;
    CSimpleIniA::TNamesDepend::const_iterator it;
    ini.GetAllSections(sections);

    for(it = sections.begin(); it != sections.end(); ++it)
    {
	if(check_ip_in_range(ipaddr, cidr_to_range((char *)it->pItem)) == 1){
		result = 1;
	}
    }
    return result;
}
/*
int main(void) {
    char *str = new char[29];
    strcpy(str,"192.168.1.0/24");
    char *ipaddr = new char[29];
    strcpy(ipaddr,"192.168.100.11");
    char *internalRange = new char[29];
    strcpy(internalRange, "10.8.8.0/24");
    struct ip_range range = cidr_to_range(str);

/*    char ipbuf[INET_ADDRSTRLEN];
    //printf("Dest IP address: %s\n", inet_ntop(AF_INET, &range.first, ipbuf, sizeof(ipbuf)));
    printf("Start %u End %u", ntohl(range.first), ntohl(range.last));
    unsigned int nlAddr;
    std::map <int,const char *> ipList;
    int count = 0;
    for(int a = ntohl(range.first); a <= ntohl(range.last); a = a + 1){
	nlAddr = htonl((unsigned int )a);
	ipList[count] = inet_ntop(AF_INET, &nlAddr, ipbuf, sizeof(ipbuf) );
	//printf ( "%s\n", inet_ntop(AF_INET, &nlAddr, ipbuf, sizeof(ipbuf) )) ;
	count++;
    }
    printf("Map size: %lu", ipList.size());
*/
/*
    printf("sec range ip: %s\n",get_ip_from_second_range(ipaddr, range, cidr_to_range(internalRange)));

    CSimpleIniA ini;
    ini.SetUnicode();
    ini.LoadFile("myfile.ini");
    printf("ini value: %s\n", ini.GetValue("test","test","defailt") );

    // get all sections
    CSimpleIniA::TNamesDepend sections;
    CSimpleIniA::TNamesDepend::const_iterator it;
    ini.GetAllSections(sections);

    for(it = sections.begin(); it != sections.end(); ++it)
    {
	printf("section : %s", it->pItem);
    }
*/
    //printf(" %s\n",get_internal_ip(ipaddr,1));
    
    //const char *secIp = get_ip_from_second_range(ipaddr, range, cidr_to_range(internalRange));
    //
    //printf("ip from second range: %s", get_ip_from_second_range(ipaddr, range, cidr_to_range(internalRange)));
    //return 0;
/*
}
*/

