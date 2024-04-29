/***************************************************************************************************
* Project     : Sending SMS msgs from a z/OS to Cellphons thru CHT (ChungHwa Tecom) SMS Server
* Description : 
* 1. The PDS to contain this program should be defined with LRECL=100 or more or as a RECFM=VB PDS
* 2. The SMS Server's iP is hard-coded : 202.39.54.130
* 3. Both the Account name: Login[10] and Password: Pw[10] are provided from CHT. Fill them with
*    valid ones
* 4. One LRECL=170 sequential file is opened to read a serial of SMS message records to be sent
* 5. Each record's first 10 bytes is cellphone number, and the remaining 160 bytes are as the SMS 
*    message text to be sent to that cellphone.
* 6. Use CHTSMSJC.JCL to compile / Link the program
* 7. Use CHTSMSSQ.JCL to run the program
*                                                 Andrew Jan 20200110
***************************************************************************************************/
#define  _OE_SOCKETS                   /* added for z/OS */
#define  _POSIX_C_SOURCE 200112L       /* added for z/OS */
#include <arpa/inet.h>                 /* added for z/OS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SERV_PORT 8000

struct Send_Msg{
   unsigned char  msg_type;
   unsigned char  msg_coding;
   unsigned char  msg_priority;
   unsigned char  msg_country_code;
   unsigned char  msg_set_len;
   unsigned char  msg_content_len;
            char  msg_set[100];
            char  msg_content[160];
};

struct Ret_Msg{
   unsigned char  ret_code ;
   unsigned char  ret_coding ;
   unsigned char  ret_set_len;
   unsigned char  ret_content_len;
            char  ret_set[80];
            char  ret_content[160];
};

const unsigned char ascii_tab[] = {
    0x00, 0x01, 0x02, 0x03, 0x9C, 0x09, 0x86, 0x7F,  /* 00 - 07 */
    0x97, 0x8D, 0x8E, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,  /* 08 - 0F */
    0x10, 0x11, 0x12, 0x13, 0x9D, 0x85, 0x08, 0x87,  /* 10 - 17 */
    0x18, 0x19, 0x92, 0x8F, 0x1C, 0x1D, 0x1E, 0x1F,  /* 18 - 1F */
    0x80, 0x81, 0x82, 0x83, 0x84, 0x0A, 0x17, 0x1B,  /* 20 - 27 */
    0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x05, 0x06, 0x07,  /* 28 - 2F */
    0x90, 0x91, 0x16, 0x93, 0x94, 0x95, 0x96, 0x04,  /* 30 - 37 */
    0x98, 0x99, 0x9A, 0x9B, 0x14, 0x15, 0x9E, 0x1A,  /* 38 - 3F */
    0x20, 0xA0, 0xE2, 0xE4, 0xE0, 0xE1, 0xE3, 0xE5,  /* 40 - 47 */
    0xE7, 0xF1, 0xA2, 0x2E, 0x3C, 0x28, 0x2B, 0x7C,  /* 48 - 4F */
    0x26, 0xE9, 0xEA, 0xEB, 0xE8, 0xED, 0xEE, 0xEF,  /* 50 - 57 */
    0xEC, 0xDF, 0x21, 0x24, 0x2A, 0x29, 0x3B, 0xAC,  /* 58 - 5F */
    0x2D, 0x2F, 0xC2, 0xC4, 0xC0, 0xC1, 0xC3, 0xC5,  /* 60 - 67 */
    0xC7, 0xD1, 0xA6, 0x2C, 0x25, 0x5F, 0x3E, 0x3F,  /* 68 - 6F */
    0xF8, 0xC9, 0xCA, 0xCB, 0xC8, 0xCD, 0xCE, 0xCF,  /* 70 - 77 */
    0xCC, 0x60, 0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,  /* 78 - 7F */
    0xD8, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,  /* 80 - 87 */
    0x68, 0x69, 0xAB, 0xBB, 0xF0, 0xFD, 0xFE, 0xB1,  /* 88 - 8F */
    0xB0, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70,  /* 90 - 97 */
    0x71, 0x72, 0xAA, 0xBA, 0xE6, 0xB8, 0xC6, 0xA4,  /* 98 - 9F */
    0xB5, 0x7E, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,  /* A0 - A7 */
    0x79, 0x7A, 0xA1, 0xBF, 0xD0, 0xDD, 0xDE, 0xAE,  /* A8 - AF */
    0x5E, 0xA3, 0xA5, 0xB7, 0xA9, 0xA7, 0xB6, 0xBC,  /* B0 - B7 */
    0xBD, 0xBE, 0x5B, 0x5D, 0xAF, 0xA8, 0xB4, 0xD7,  /* B8 - BF */
    0x7B, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,  /* C0 - C7 */
    0x48, 0x49, 0xAD, 0xF4, 0xF6, 0xF2, 0xF3, 0xF5,  /* C8 - CF */
    0x7D, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,  /* D0 - D7 */
    0x51, 0x52, 0xB9, 0xFB, 0xFC, 0xF9, 0xFA, 0xFF,  /* D8 - DF */
    0x5C, 0xF7, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,  /* E0 - E7 */
    0x59, 0x5A, 0xB2, 0xD4, 0xD6, 0xD2, 0xD3, 0xD5,  /* E8 - EF */
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,  /* F0 - F7 */
    0x38, 0x39, 0xB3, 0xDB, 0xDC, 0xD9, 0xDA, 0x9F   /* F8 - FF */
};

const unsigned char ebcdic_tab[] = {
    0x00, 0x01, 0x02, 0x03, 0x37, 0x2D, 0x2E, 0x2F,  /* 00 - 07 */
    0x16, 0x05, 0x15, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,  /* 08 - 0F */
    0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26,  /* 10 - 17 */
    0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,  /* 18 - 1F */
    0x40, 0x5A, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D,  /* 20 - 27 */
    0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,  /* 28 - 2F */
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,  /* 30 - 37 */
    0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,  /* 38 - 3F */
    0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,  /* 40 - 47 */
    0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,  /* 48 - 4F */
    0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6,  /* 50 - 57 */
    0xE7, 0xE8, 0xE9, 0xAD, 0xE0, 0xBD, 0x5F, 0x6D,  /* 58 - 5F */
    0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,  /* 60 - 67 */
    0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,  /* 68 - 6F */
    0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,  /* 70 - 77 */
    0xA7, 0xA8, 0xA9, 0xC0, 0x4F, 0xD0, 0xA1, 0x07,  /* 78 - 7F */
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x06, 0x17,  /* 80 - 87 */
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x09, 0x0A, 0x1B,  /* 88 - 8F */
    0x30, 0x31, 0x1A, 0x33, 0x34, 0x35, 0x36, 0x08,  /* 90 - 97 */
    0x38, 0x39, 0x3A, 0x3B, 0x04, 0x14, 0x3E, 0xFF,  /* 98 - 9F */
    0x41, 0xAA, 0x4A, 0xB1, 0x9F, 0xB2, 0x6A, 0xB5,  /* A0 - A7 */
    0xBB, 0xB4, 0x9A, 0x8A, 0xB0, 0xCA, 0xAF, 0xBC,  /* A8 - AF */
    0x90, 0x8F, 0xEA, 0xFA, 0xBE, 0xA0, 0xB6, 0xB3,  /* B0 - B7 */
    0x9D, 0xDA, 0x9B, 0x8B, 0xB7, 0xB8, 0xB9, 0xAB,  /* B8 - BF */
    0x64, 0x65, 0x62, 0x66, 0x63, 0x67, 0x9E, 0x68,  /* C0 - C7 */
    0x74, 0x71, 0x72, 0x73, 0x78, 0x75, 0x76, 0x77,  /* C8 - CF */
    0xAC, 0x69, 0xED, 0xEE, 0xEB, 0xEF, 0xEC, 0xBF,  /* D0 - D7 */
    0x80, 0xFD, 0xFE, 0xFB, 0xFC, 0xBA, 0xAE, 0x59,  /* D8 - DF */
    0x44, 0x45, 0x42, 0x46, 0x43, 0x47, 0x9C, 0x48,  /* E0 - E7 */
    0x54, 0x51, 0x52, 0x53, 0x58, 0x55, 0x56, 0x57,  /* E8 - EF */
    0x8C, 0x49, 0xCD, 0xCE, 0xCB, 0xCF, 0xCC, 0xE1,  /* F0 - F7 */
    0x70, 0xDD, 0xDE, 0xDB, 0xDC, 0x8D, 0x8E, 0xDF   /* F8 - FF */
};

void ascii(char*);
void ebcdic(char*);

int FillMsg(char *src, char *target, int n)
{
   int i,len;
   len = strlen(target);
   for(i=0; i<len; i++)
     src[n++] = target[i];
   src[n++] = '\0';
   return(n);
}

void LogMessage(char *info, char *Login, char *MsgId, char *TelNum, char *Msg)
{
   char str_time[50];
   char TmpStr[300];
   time_t tnow;
   struct tm *lctime;
   FILE *fp;
   if( (fp=fopen("DD:SMS4CHT","a+")) == NULL )
      return;
   time(&tnow);
   lctime = localtime(&tnow);
   strftime(str_time, 80, "%Y%m%d.%H%M%S", lctime);
   sprintf(TmpStr,"%s %8s %8s %10s %s %s\n", str_time, Login, MsgId, TelNum, info, Msg);
   fputs(TmpStr, fp);
   fclose(fp);
}


int main(int argc, char *argv[])
{
int i,iPos,ret;
int sockfd;
struct Send_Msg sendMsg;
struct Ret_Msg retMsg;
struct sockaddr_in servaddr;

char IpAddr[20] = "202.39.54.130";
char Login[10] = "xxxxxxxx";
char Pw[10] = "xxxxxxxx";
char TelNum[20] = "";
char Msg_Content[160] = "";
char MsgId[25] = "";
char Send_Type[3] = {0x30, 0x31};

/* the following are ascii versions */
char aLogin[10] = "";
char aPw[10] = "";
char aTelNum[20] = "";
char aMsg_Content[160] = "";

strcpy(aLogin, Login);
strcpy(aPw, Pw);
ascii(aLogin);
ascii(aPw);

/* read TelNum & Message from infile */
FILE *infile;
char inbuf[171];
inbuf[170] = '\0';
int num;
if ((infile = fopen("dd:MSG4CHT","rb,type=record")) == NULL)
  {
   LogMessage("Fail:Could not open infile! ","", "", "", "");
  }
else
  {
   while (!feof(infile))
     {
      num = fread(inbuf, sizeof(char), 170, infile);
      if (feof(infile))
         break;
      strncpy(TelNum, inbuf,10);
      strcpy(Msg_Content, inbuf+10);

      Msg_Content[159] = '\0';

      strcpy(aTelNum, TelNum);
      strcpy(aMsg_Content, Msg_Content);

      ascii(aTelNum);
      ascii(aMsg_Content);

      /******* Startup Socket *******/
      if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
         printf("socket error");
      bzero(&servaddr, sizeof(servaddr));
      servaddr.sin_family = AF_INET;
      servaddr.sin_port   = htons(SERV_PORT);
      if (inet_pton(AF_INET, IpAddr, &servaddr.sin_addr) <= 0){
         printf("socket inet_pton error for %s", IpAddr);
         exit(0);
      }
      if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
         printf("socket connect error");
         exit(0);
      }


      /**********************************************/
      memset((char *)&sendMsg, 0, sizeof(sendMsg));
      sendMsg.msg_type = 0;
      iPos = 0;
      iPos = FillMsg(&sendMsg.msg_set[0], aLogin, iPos);
      iPos = FillMsg(&sendMsg.msg_set[0], aPw, iPos);
      sendMsg.msg_set_len = iPos;

      if((ret = write(sockfd , (char *)&sendMsg, sizeof(sendMsg)))<0){
         printf("socket sending User/Pwd error");
         close(sockfd);
         LogMessage("Fail:Socket Sending_User/Pwd_Error! ",Login, MsgId, TelNum, Msg_Content);
         exit(0);
      }

      memset((char *)&retMsg, 0, sizeof(retMsg));
      if((ret = read(sockfd , (char *)&retMsg, sizeof(retMsg)))<0){
         printf("socket receiving User/Pwd error");
         close(sockfd);
         LogMessage("Fail:Socket Receiving_User/Pwd_Error! " , Login , MsgId, TelNum, Msg_Content);
         exit(0);
      }
      if(retMsg.ret_code != 0 ){
         printf("Login/Password_Error !\n");
         close(sockfd);
         LogMessage("Fail:Login/Password_Error! ", Login, MsgId, TelNum, Msg_Content);
         exit(0);
      }
      ebcdic(retMsg.ret_content);
      printf("%s", retMsg.ret_content);

      /**************************************/
      memset((char *)&sendMsg, 0, sizeof(sendMsg));
      sendMsg.msg_type = 1;
      sendMsg.msg_coding = 1;
      /* strcpy(Send_Type,"01"); this is unnessary */

      iPos = 0;
      iPos = FillMsg(&sendMsg.msg_set[0], aTelNum, iPos);
      iPos = FillMsg(&sendMsg.msg_set[0], Send_Type, iPos);
      sendMsg.msg_set_len = iPos;


      strcpy(sendMsg.msg_content,aMsg_Content);
      sendMsg.msg_content_len = strlen(aMsg_Content);

      if((ret = write(sockfd , (char *)&sendMsg, sizeof(sendMsg)))<0){
         printf("socket sending message error");
         close(sockfd);
         LogMessage("Fail:Socket Sending_Message_Error! ",Login, MsgId, TelNum, Msg_Content);
         exit(0);
      }

      memset((char *)&retMsg, 0, sizeof(retMsg));
      if((ret = read(sockfd , (char *)&retMsg, sizeof(retMsg)))<0){
         printf("socket receiving message error");
         close(sockfd);
         LogMessage("Fail:Socket Receiving_User/Pwd_Error! ",Login, MsgId, TelNum, Msg_Content);
         exit(0);
      }

      strcpy(MsgId, retMsg.ret_content);

      ebcdic(MsgId);

      printf("\nret_code(%d),Message Id(%s)\n", retMsg.ret_code,MsgId);
      LogMessage("Success! ",Login, MsgId, TelNum, Msg_Content);
      close(sockfd);
     }
   if (fclose(infile))
      LogMessage("Fail:Could not close infile! ","", "", "", "");
  }
}

/* this is converting codes from ebcdic to ascii */
void ascii(char * buf)
{
  char * b = buf;
  while (*b != '\0')
     {
       *b = ascii_tab[*b];
       b++;
     }
}

/* this is converting codes from ascii to ebcdic */
void ebcdic(char * buf)
{
  char * b = buf;
  while (*b != '\0')
     {
       *b = ebcdic_tab[*b];
       b++;
     }
}
