/*  МЕТОДИЧЕСКИЕ УКАЗАНИЯ.

  Написать модуль ядра, который будет лнять функции файрвола. */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/inet.h>
#include <linux/slab.h>
#include <asm/string.h>

#define SYS_ENTRY_FILENAME "my_firewall"
#define IP_TABLE_SIZE 5
#define IP_SIZE 17
#define IP_NUM_CNT 4

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Karlamachev R. H.");
MODULE_DESCRIPTION("Firwall linux kernel module");

static struct kobject *fw_kobject;
static char   *IP_to_block = NULL;
static char   ipTable[IP_TABLE_SIZE][IP_SIZE];
static char   *usrBuff = NULL;
static struct nf_hook_ops nfin;

//Запишем IP - адреса в буфер для передачи в пространство пользователя
static void store_ips_to_buffer(char *buf)
{
  int i = 0, j = 0;    
  for(i = 0; ipTable[i][0] != 0 && i < IP_TABLE_SIZE; i++)
  {       
    j = 0; 
    while(ipTable[i][j] != '\n')
      j++;
    j = j + 1;
    strncpy(buf, ipTable[i], j); 
    buf = buf+j;
  }
  buf++; 
  *buf = '\0';
}

//Вывод таблицы ip в пространство пользователя
static ssize_t ip_show(struct kobject *kobj, 
                       struct kobj_attribute *attr, 
                       char *buf) 
{
  int i = 0;

  if(ipTable[0][0] == 0)
    return sprintf(buf, "%s\n", "No ip loaded to filter"); 
  
  for (i = 0; ipTable[i][0] != 0 && i < IP_TABLE_SIZE; i++);
  
  //Выделим память для помещения всех записей IP адресов в таблице блокировки:
  //размер char * на количество адресов * размер записи в таблице + количество '\n' + '\0'
  usrBuff = kzalloc(sizeof(char)*i*IP_SIZE+i+1, GFP_KERNEL); 
  //Поместим IP адреса в выделенный буфер
  store_ips_to_buffer(usrBuff);

  usrBuff = &usrBuff[0];

  return sprintf(buf, "%s", usrBuff);     
}

//Функция сравнения переданного IP с имеющимися в таблице блокировки для возможного удаления
static int delete_ip(void)
{
  int isDeleted = 0;
  int i = 0, j = 0;  

  char *buff = kzalloc(20, GFP_KERNEL);
  strncpy(buff, IP_to_block, IP_SIZE);  
  
  for(i = 0; ipTable[i][0] != 0 && i < IP_TABLE_SIZE; i++)
  {   
    if(strcmp(buff, ipTable[i]) == 0)
    {
      printk(KERN_INFO "IP %s deleted from firewall blocking table\n", ipTable[i]);      
      j = i;
      if(j < IP_TABLE_SIZE - 1)
      {
        do
        {
          memcpy(ipTable[j], ipTable[j+1], IP_SIZE);       
          j++;
        }while(ipTable[j][0] != 0 && j < IP_TABLE_SIZE-1);

        if(ipTable[j][0] != 0) 
          memset(ipTable[j], 0, IP_SIZE);
      } else
          memset(ipTable[j], 0, IP_SIZE);
      isDeleted = 1;      
    }
  }
  kfree(buff);
  return isDeleted;
}

static int add_ip(void)
{  
  int i = 0;  
  for(i = 0; ipTable[i][0] != 0 && i < IP_TABLE_SIZE; i++);
  if (i == IP_TABLE_SIZE) 
  {
    printk(KERN_INFO "No free cells in firewall blocking table\n");
    return -1;
  } else  
      strncpy(ipTable[i], IP_to_block, IP_SIZE);  
  printk(KERN_INFO "IP %s added to firewall blocking table\n", ipTable[i]);
  return 0;
}

static void digits_cnt(int *charCnt, int *digitsCnt)
{
  if(*charCnt > 0 && *charCnt <= 3)
    (*digitsCnt)++;    
}

static int is_correct_IP(void)
{   
  int isIP = 0;
  int charCnt1 = 0;
  int charCnt2 = 0;
  int digitsCnt = 0;
  int dotsCnt = 0;
  int i = 0;

  int ipNum[IP_NUM_CNT];
  int *ipNumPtr = ipNum;
  char *num = kzalloc(10, GFP_KERNEL);
  char *buff = kzalloc(20, GFP_KERNEL);
  char *temp = NULL; 

  strncpy(buff, IP_to_block, IP_SIZE);

  temp = buff;

  while(*buff != '\n' && charCnt1 < IP_SIZE)
  {  
    buff++;  
    charCnt1++;    
    charCnt2++;
    if(*buff == '.' && dotsCnt < 3) 
    {
      dotsCnt++;
      digits_cnt(&charCnt2, &digitsCnt);
      //printk(KERN_INFO "Count of chars: %d\n", charCnt2);
      //printk(KERN_INFO "Count of digits: %d\n", digitsCnt);
      
      memset(&num[0], 0, 20);
      strncpy(&num[0], temp, charCnt2);     
      printk(KERN_INFO "Char string to integer: %s", num);    
      buff++;   
      temp = buff;     
      
      if(sscanf(&num[0], "%du", ipNumPtr) <= 0)
      {
        printk(KERN_INFO "Conversation char string to integer fault\n");
        kfree(num);
        kfree(buff);
        return -EINVAL;
      }
      ipNumPtr++;
      charCnt2 = 0;
    } 

    if(dotsCnt == 3 && *buff == '\n')
    {
      digits_cnt(&charCnt2, &digitsCnt); 
      //printk(KERN_INFO "Count of digits: %d\n", digitsCnt);
      memset(&num[0], 0, 20); 
      strncpy(&num[0], temp, charCnt2);            
      if(sscanf(&num[0], "%du", ipNumPtr) <= 0)
      {
        printk(KERN_INFO "Conversation char string to integer fault\n");
        kfree(num);
        kfree(buff);
        return -EINVAL;
      }            
    }              
  }   
  if(digitsCnt == IP_NUM_CNT) 
  {
    digitsCnt = 0;    
    for(i = 0; i < IP_NUM_CNT; i++)
    {
      if(ipNum[i] >= 0 && ipNum[i] < 256)
        digitsCnt++;            
    }
    printk(KERN_INFO "Count of digits: %d\n", digitsCnt);
    printk(KERN_INFO "IP from user space: %d.%d.%d.%d", ipNum[0], ipNum[1],  ipNum[2], ipNum[3]);    
    if(digitsCnt == IP_NUM_CNT)
      isIP = 1;
  }
  kfree(num); 
  kfree(buff); 
  return isIP;
}

//Function for many symbol data enter
static ssize_t __used ip_store(struct kobject *kobj, 
                               struct kobj_attribute *attr, 
                               const char *buf, 
                               size_t count) 
{
  int isIP = 0;     
  int isDeleted = 0;  
  
  strncpy(IP_to_block, buf, PAGE_SIZE - 1);
  
  printk(KERN_INFO "Data from user space: %s", IP_to_block);

  isIP = is_correct_IP();

  if(isIP <= 0)
  {
    printk(KERN_INFO "Data from user space is not valid\n");
    return -EINVAL;
  } 
  
  //Если отправлен адрес IP имеющийся в таблице, удалим его из таблицы фильтрации пакетов  
  isDeleted = delete_ip();

  //Иначе добавим в таблицу
  if(!isDeleted)  
    add_ip();   
  
  return count;
}

//Функция перехвата пакетов из сети и проверки наличия ip-адреса в таблице блокировки
static unsigned int hook_func_in(void *priv, 
                                 struct sk_buff *skb, 
                                 const struct nf_hook_state *state) 
{
  struct ethhdr *eth;
  struct iphdr *ip_header;

  int i = 0, j = 0;

  char sourceIP[IP_SIZE];
  char blockedIP[IP_SIZE];  
  memset(sourceIP, 0, IP_SIZE);
  memset(blockedIP, 0, IP_SIZE);
  
  eth = (struct ethhdr*)skb_mac_header(skb);

  ip_header = (struct iphdr*)skb_network_header(skb);

  /* Работаем только с IP */
  if (skb->protocol != htons(ETH_P_IP))
    return NF_ACCEPT;

  snprintf(sourceIP, IP_SIZE, "%pI4", &ip_header->saddr);

  printk(KERN_INFO "src IP addr: %s\n", sourceIP);

  for(i = 0; ipTable[i][0] != 0 && i < IP_TABLE_SIZE; i++)
  {
    j = IP_SIZE-2;
    strncpy(blockedIP, ipTable[i], IP_SIZE);
    
    while(blockedIP[j] != '\n')
      j--;
    blockedIP[j] = '\0';
    if(strcmp(sourceIP, blockedIP) == 0)
    {
      printk(KERN_INFO "src IP addr: %s blocked\n", sourceIP);
      return NF_DROP;
    }
  }  
  //Пропустим пакет, если проверка пройдена
  return NF_ACCEPT;
}

static struct kobj_attribute fw_attribute = __ATTR(IP_to_block, 0660, ip_show, ip_store);

//Put attribute to attribute group
static struct attribute *register_attrs[] = {
    &fw_attribute.attr,
    NULL,   /* NULL terminate the list */
};

static struct attribute_group  reg_attr_group = {
    .attrs = register_attrs
};

static int __init init_main(void)
{   
  memset(ipTable, 0, IP_TABLE_SIZE*IP_SIZE);

  IP_to_block = kzalloc(PAGE_SIZE, GFP_KERNEL);

  fw_kobject = kobject_create_and_add(SYS_ENTRY_FILENAME, kernel_kobj);

  if(!fw_kobject)
      return -ENOMEM;

  //Create attributes (files)
  if(sysfs_create_group(fw_kobject, &reg_attr_group))
  {
    kobject_put(fw_kobject);
    printk(KERN_INFO "Failed to create the SYS_ENTRY_FILENAME file in /sys/kernel/\n");
    return -ENOMEM;
  }
  
  nfin.hook     = hook_func_in;
  nfin.hooknum  = NF_INET_PRE_ROUTING;
  nfin.pf       = PF_INET;
  nfin.priority = NF_IP_PRI_FIRST;
  nf_register_net_hook(&init_net, &nfin); 
  printk(KERN_INFO "Firewall initialized successfully\n");

  return 0;
}
 
static void __exit cleanup_main(void)
{
  printk(KERN_INFO "Cleanup_module runned\n");
  nf_unregister_net_hook(&init_net, &nfin); 
  printk(KERN_INFO "Firewall uninitialized successfully\n");  
  if(IP_to_block != NULL)
    kfree(IP_to_block);  
  if(usrBuff != NULL)
    kfree(usrBuff);  
  kobject_put(fw_kobject);
}

module_init(init_main);
module_exit(cleanup_main);