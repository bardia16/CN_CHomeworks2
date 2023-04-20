# CN_CHomeworks2
In this code, we want to simulate a wireless computer network. 

There are three main parts to our nodes. One is the mapper, the second is the client, the third is the master. We use UDP for communications between master and client and between client and mapper, and TCP for communications between master and mapper. Each class has a handle read which is used when we sent a packet to the node of that class. The handler for our client recieves the packet and prints it. The handler for master takes the packet, removes the header, repackages it into another packet and sends it to all mappers. The handler for maps takes the header, matches it, repackages it into another packet and sends it to the client. Each class also has a start and a constructer, which makes the sockets we want which is states in the comments of our code.




تحلیل پاارامتر ها:
Throughput:
در اول کار که از کلاینت به مستر پکت ارسال میشود این مقدار ماکزیمم و برابر 1 است
اما رفته رفته این مقدار به دلیل تاخیر و مخصوصا بخاطر پروتکل های تی سی پی که اکنالج دارند کم میشود
که در آخر به مقدار حدود 0.3 میرسد

average end to end delay:
در این بخش برای بیشتر انتقال ها مقداری ثابت است اما برای اکنالج کردن ارسال از مستر به مپر ها این مقدار زیاد میشود که به دلیل خاصیت پروتکل تی سی پی است
