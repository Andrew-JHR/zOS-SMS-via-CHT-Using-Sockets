//ANDREWJC JOB   CLASS=A,MSGCLASS=X,NOTIFY=&SYSUID  
//STEP1    EXEC  PGM=CHTSMSSQ
//STEPLIB  DD    DISP=SHR,DSN=ANDREWJ.SOURCE.LMD
//SYSPRINT DD    SYSOUT=*
//* MSG4CHT is for a sequential file containing a serial of SMS msesages
//* to be sent. 
//* The first 10 bytes are cellphone numbers of each record
//* The remaining bytes of each record are SMS message contents
//* The sequential file is Fix-blocked: LRECL=170 (BLZSIZE=27880)
//MSG4CHT  DD    DISP=SHR,DSN=ANDREWJ.FB170
//SMS4CHT  DD    SYSOUT=*
