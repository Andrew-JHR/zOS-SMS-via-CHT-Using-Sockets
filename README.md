# Sending SMS messages to cellphones through ChungHwa Telcom on z/OS

This repository demonstrates how to send cellphones the SMS messages using TCP sockets on z/OS with the SMS server provided by CHT (ChungHwa Telcom). Other service provider may have similar interfaces to interact with.

1. The IP address in the program: 202.39.54.130 is one of CHT's servers which provides SMS services on a paid basis.

2. A valid pair of account and password must be put into the program: Login[10] and Pw[10]. This pair of account/password is obtained from CHT upon the purchase agreement.

3. **CHTSMSPM.C** is supposed to receive an argument containing 10 digits of a cellphone number as the first 10 bytes immediately followed by the SMS message itself.

4. **CHTSMSSQ.C** is expecting an input sequential file whose LRECL is 170. Each record of the sequential file is an SMS message -- within 160 bytes -- to be sent to the cellphone whose number is the first 10 digits of the very record.

5. **CHTSMSJC.JCL** is used to compile and Link-edit **CHTSMSPM.C** and **CHTSMSSQ.C**.

6. Use **CHTSMSPM.JCL** to run the LMD of **CHTSMSPM.C**.

7. Use **CHTSMSSQ.JCL** to run the LMD of **CHTSMSSQ.C**.