# SOCKET聊天小程序

[TOC]
##  1.聊天协议
### 1.语法  
1. 输入时必须要输入发送对象的昵称 [^注1] 而且是在第一个冒号之前***recv:message***。
2. 想对多个对象发消息时必须要把每个对象昵称用‘|’来分割开***user1_name \| user2_name:message***. 
3. 收到消息时客户端将会打印出***[user_name]:message***。


### 2.语义  
1. 点开客户端后必须发送一条SENDBUFF为自己取个昵称，当服务端接收到改昵称后会把该SOCKET 与昵称放在一个全局map[^注2]里。然后给所有的客户端提示 ***"欢迎 username 加入该服务器"*** 并为该用户显示服务器在线人员。
2. 如果在建立映射关系时发现该昵称已被该服务器在线的人所使用，它就不会把它存到map里而是给该SOCKET发个提示说该昵称已被使使用请重新链接之后关闭该线程和SOCKET。
3. 服务端接受到消息后会对RECVBUFF进行接受对象和发送信息的分离操作。
    * 通过**find_SendId()**函数从RECVBUFF中提取该消息接受对象的昵称。
    * 通过**cut_buff()**函数从RECVBUFF中提取消息部分。
    * 通过第一步提取出来的用户名一个一个映射出接收者的SOCKET来发消息。
2. 发送对象为该服务器的所有人时我们只要在冒号前面写个***all***就可以，然后服务端会把消息发送给所有map里现存的用户。
3. 当服务器发现某一消息的接受者昵称不在map里，那么服务器会给发送者一个该用户不存在的消息。
4. 当RECVBUFF等于"quit"时，表示该客户端想要退出服务，那么这时候先会给所有其他的用户发送该用户退出服务器的消息然后把给用户的昵称和SOCKET从map里删除，最后才把SOCKET给关掉。

### 3.时序

#### 1.客户端

1. 给想要链接的服务器发送**connect**请求。
2. 生成一个新的线程来为自己接受消息并打印到客户端界面。
3. 进入循环用**cin.getline()**来输入SENDBUFF并把消息发出去。

#### 2.服务器端

1. 与客户端不同的就是服务端在初始化完后主线程就进入了一个循环来不断地监听该SOCKET收到的**connect**请求。
2. **accept**成功后它会生成一个新的SOCKET并把它放入一个新的线程作为参数。
3. 进入新的线程后它就把该SOCKET收到的第一个RECVBUFF[^注3]与该SOCKET建立起一个映射关系并存到全局map里。
4. 进入循环并不断的处理RECVBUFF来执行相应的文本处理和发送消息功能。 

## 2.代码讲解 
注： 该代码是Linux版本，不适应用Windows

### 1.客户端

1. 客户端整体函数框架就是一个用主函数和一个线程函数构成。


2. Client_recv（LP ipParameter）线程函数

   * 进到线程里面后先查看传进来的参数是否是一个完整的SOCKET。

   * 进入循环里面判断recv返回的值是否正常，并且利用它的阻塞机制一条一条的打印消息。  


3. 主函数
   * 主要完成了标准的SOCKET的初始化过程。
   * 完成了connect的后验证是否链接成功。
   * 如果成功建成了的话它会为该SOCKET生成一个新的线程来不断的接受消息，因为在后面的循环语句里不但有send（）函数有阻塞，cin.getline()过程也有阻塞因此会出现在未输入的情况下无法收到消息或者在未接受到消息的情况下无法输入的情况。
   * 进入循环后把我insert的值传给Sendbuff，然后在判断我发出去的消息是否是退出命令“quit”，如果是那么我们就跳出这个循环关闭客户端。
   * 上面我们能够直接退出客户端而不关闭SOCKET的原因是sercer端会判断收到的消息是否是退出命令，如果是它会关闭SOCKET。 

  



### 2.服务器端

1. 服务器端的函数是由解析文本函数[^注4]，线程函数，主函数组成。  
 

2. find_key(SOCKET s)解析函数
   
      * 该函数是遍历全局变量map（list_socket）的SOCKET判断是否与传进去的s相等，如果有相等的话会把该SOCKET的key返回出来。  
      

3. faind_SendId(string s)解析函数[^注5]

   * 我们在这儿用到了vector<string>结构来保存我们从s里面解析出的消息接受对象昵称。

   * 第一个循环是在提取冒号前面的内容，因为在聊天协议语法当中规定过在第一冒号前输入消息接受对象的昵称或者**all**指令。

   * 第二个循环是通过竖线分离出各个接受对象的昵称并放入vector结构当中，如果只是一个对象或者是**all**指令的话会通过冒号放入到vector结构中。

   * 最后返回出vector结构。  


4. cut_buff(string s)解析函数

   * 该函数就很简单只是单纯的用循环把冒号后面的部分提取出来并用反转函数反转一下，因为在聊天协议语法当中规定了信息必须写在冒号后面。


5. Client_Thread（ipParameter）线程函数

   * 通过聊天协议语法当中规定的客户端点开后要发送一个自己的昵称，因此在进入线程后就要执行一次recv，这样就可以通过recv的阻塞机制来阻止客户端没有昵称时不能接受任何消息的目的。

   * 但我们也要考虑重名的问题，因此声明一种我不太了解的结构（不好意思。学的c++有点浅而且很久没用了不太熟）通过它的返回值来判定该昵称是否已经在list_socket中已经存在，如果存在的话会插入失败，然后给该socket发送一条重名的提示并把该SOCKET关了；反则就会插入成功，然后给该服务器在线的所有人发送用户上线的消息，而该用户会多收到该服务器在线人员的信息。

   * 进入循环持续接受消息并把收到的RECVBUFF用上面的两个函数处理确定消息内容和接受对象或者命令语句来完成该SOCKET的发送功能。

   * 特别强调的就是在收到退出命令时先发送消息然后从map中删除该SOCKET的信息最后才关闭SOCKET。因为如果我们在收到退出命令后就直接关闭SOCKET的话不能够再发送用户退出的消息Send功能失效了。


6. 主函数

   * 主函数前面部分也只是完成了常规的SOCKET初始话过程。

   * 当初始化完服务器SOCKET以后它会进入循环不断的接受客户端的connect并生成一个新的SOCKET放入到一个新的线程里。




## 



[^注1]:无论什么时候昵称后面不能有空格。
[^注2]:key->user_name,value->socket
[^注3]:客户端作为昵称传过来的消息。
[^注4]:那个反转函数可以不算，因为我的SENDBUFF和RECVBUFF都是char*处理起来很麻烦才加的。
[^注5]:因为是char* 类型的RECVBUFF，所以在昵称输入时空格也会算昵称的一部分的问题，但在该函数里没有解决。
