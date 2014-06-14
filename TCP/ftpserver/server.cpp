#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>

/* 加上IP和TCP数据结构 */
#include <netinet/ip.h>
#include <netinet/tcp.h>

//文件读写
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

#define dataLen 1024 //缓冲区大小

char currentDirPath[200]; //当前工作目录的绝对路径
char currentDirName[30]; //当前目录的名称
char help[]="get  get a file from server\n\
put   upload a file to server\n\
pwd   display the current directory of server\n\
dir   display the files in the current directory of server\n\
cd    change the directory of server\n\
?     display the whole command which equals 'help'\n\
quit  return\n";//帮助信息

char *getDirName(char *dirPathName);
//根据当前工作目录的绝对路径得到当前目录名称
void cmd_pwd(int sock); //处理pwd命令
void cmd_dir(int sock); //处理dir命令
void cmd_cd(int sock,char *dirName); //处理cd命令
void cmd_help(int sock); //处理?命令
void cmd_get(int sock,char*fileName); //处理get命令
void cmd_put(int sock,char *fileName); //处理put命令



int main(int argc,char *argv[]){
	int sock; //服务器用于监听的数据通道
	int sockmsg; //服务器用于监听的命令通道
	char client_cmd[10]; //客户端出发的命令
	char cmd_arg[20]; //客户端输入的文件或目录名,用在cd,put,get命令中
	struct sockaddr_in server; //服务器数据通道的信息
	struct sockaddr_in servermsg; //服务器命令通道的信息
	int datasock; //用于通信的数据通道
	int msgsock; //用于通信的命令通道
	pid_t child;

	//client子进程
	sock=socket(AF_INET,SOCK_STREAM,0);
	//创建用于传数据的套接字
	sockmsg=socket(AF_INET,SOCK_STREAM,0);
	//创建用于传消息的套接字
	int opt = 1,opt2 = 1;
	setsockopt(sock , SOL_SOCKET , SO_REUSEADDR , &opt , sizeof(opt));
	//实现sock的重用
	setsockopt(sockmsg , SOL_SOCKET , SO_REUSEADDR , &opt2 , sizeof(opt2));//实现sockmsg的重用
	if (sock < 0 || sockmsg < 0){
		//socket创建失败
		
		perror("opening stream socket");
		exit(1);
	}

	memset(&server,0,sizeof(server));
	server.sin_family=AF_INET; 					//设置协议族
	server.sin_addr.s_addr=htonl(INADDR_ANY); 	//监听所有地址
	server.sin_port=htons(45000);				//端口设为45000
	memset(&servermsg,0,sizeof(servermsg));
	servermsg.sin_family=AF_INET; //设置协议族
	servermsg.sin_addr.s_addr=htonl(INADDR_ANY);//监听所有地址
	servermsg.sin_port=htons(45001);			//端口设为45001

	if (bind(sock,(struct sockaddr *) &server,sizeof(server)) < 0 || 
		bind(sockmsg,(struct sockaddr *) &servermsg,sizeof(servermsg)) < 0)
	{//连接不成功
		perror("binding stream socket");
		exit(1);
	}

	int length = sizeof(server);
	int lengthmsg = sizeof(servermsg);
	if (getsockname(sock,(struct sockaddr *) &server,(socklen_t *)&length) < 0 ||
		getsockname(sockmsg,(struct sockaddr *) &servermsg,(socklen_t *)&lengthmsg) < 0)
	{//得到套接字的本地名字
		perror("getting socket name");
		exit(1);
	}
	printf("Socket port # %d %d\n",ntohs(server.sin_port),ntohs(servermsg.sin_port));
	memset(currentDirPath,0,sizeof(currentDirPath));
	getcwd(currentDirPath,sizeof(currentDirPath));//将当前工作目录的绝对路径复制到//currentDirPath中

	listen(sock,2); //监听数据通道
	listen(sockmsg,2); //监听命令通道

	while(1)
	{
		datasock = accept(sock,(struct sockaddr*)0,(socklen_t *)0); //与客户端的数据通道连接
		msgsock = accept(sockmsg,(struct sockaddr*)0,(socklen_t *)0); //与客户端的命令通道连接

		if (datasock == -1 || msgsock==-1)
			perror("accept");
		else
		{
			if((child = fork()) == -1)
				//出错
				printf("Fork Error!\n");

			if(child == 0)
			{
				//子进程
				printf("connection accepted! new client coming\n");
				write(datasock,help,strlen(help) + 1);

			loop:
				memset(client_cmd,0,sizeof(client_cmd));
				int rval = 0;
				rval = read(msgsock,client_cmd,sizeof(client_cmd));//读命令
				printf("%d\n",rval);

				if(rval < 0)
					perror("reading command failed\n");
				else if(rval == 0)
				{
					//连接关闭
					printf("connection closed.\n");
					close(datasock);
					close(msgsock);
				}
				else
				{
					printf("receive cmd:%s\n", client_cmd);
					if(strcmp(client_cmd,"pwd") == 0)
					{
						//为pwd命令
						printf("command pwd\n");
						cmd_pwd(datasock);
						printf("done\n\n");
						goto loop;
					}
					else if(strcmp(client_cmd,"dir") == 0)
					{
						//为dir命令
						printf("command dir\n");
						cmd_dir(datasock);
						printf("done\n\n");
						goto loop;
					}
					//为cd命令
					else if(strcmp(client_cmd,"cd") == 0)
					{
						printf("command cd\n");
						memset(cmd_arg,0,sizeof(cmd_arg));
						read(msgsock,cmd_arg,sizeof(cmd_arg));
						cmd_cd(datasock,cmd_arg);
						printf("done\n\n");
						goto loop;
					}
					else if(strcmp(client_cmd,"get") == 0)
					{
						//为get命令
						printf("command get\n");
						memset(cmd_arg,0,sizeof(cmd_arg));
						read(msgsock,cmd_arg,sizeof(cmd_arg));
						cmd_get(datasock,cmd_arg);
						printf("done\n\n");
						goto loop;
					}
					else if(strcmp(client_cmd,"put") == 0)
					{
						//为put命令
						printf("command put\n");
						memset(cmd_arg,0,sizeof(cmd_arg));
						read(msgsock,cmd_arg,sizeof(cmd_arg));
						cmd_put(datasock,cmd_arg);
						printf("done\n\n");
						goto loop;
					}
					else if(strcmp(client_cmd,"?") == 0)
					{
						//为?命令
						printf("command ?\n");
						cmd_help(datasock);
						printf("done\n\n");
						goto loop;
					}
					else if(strcmp(client_cmd,"quit") == 0)
					{
						//为quit命令
						printf("quit\n");
						goto endchild;
					}
					else if(strcmp(client_cmd,"$quit") == 0)
					{
						printf("server stop!\n");
						goto endchild;
					}
					else 
					{
						//错误的命令
						printf("bad request!\n");
						goto loop;
					}
				}

			endchild:
				//客户退出
				printf("connection closed.\n");
				close(datasock);
				close(msgsock);
				exit(1);
			}
		}
	}
}

/**
*getDirName 得到当前工作目录名称
*dirPathName 当前工作目录的绝对地址
*输出:当前工作目录名称
*/
char *getDirName(char *dirPathName)
{
	int i, pos, len;
	char *dirName;

	//当前工作目录名称
	if(dirPathName == NULL)
	{
		printf("directory absoultly path is null!\n");
		return NULL;
	}

	len=strlen(dirPathName);
	for(i = len - 1;i >= 0;i --)
	{
		//找到最后一个/号,之后的字符串即为当前工作目录名称
		if(dirPathName[i] == '/')
		{
			pos=i;
			break;
		}
	}

	dirName = (char *)malloc(len - pos + 1);
	for(i = pos + 1;i <= len;i ++)
		dirName[i - pos - 1] = dirPathName[i];
	return dirName;
}

//调用getDirName函数来得到当前工作目录名称,然后发到客户端
void cmd_pwd(int sock)
{
	int len;
	char *savePointer = getDirName(currentDirPath); //得到当前工作目录名称
	strcpy(currentDirName,savePointer);
	len = strlen(currentDirName) + 1;
	write(sock,currentDirName,len);					//发到客户端
}

//先遍历当前目录下的所有文件及子目录,得到文件及目录数,把该数目发给客户端后,
//再一次遍历当前目录下的所有文件及子目录,把文件或目录的名称及信息发给客户端。
void cmd_dir(int sock)
{
	DIR *pdir;
	char fileInfo[50];
	int i, fcounts = 0, len;
	struct dirent *pent;
	int fd;
	struct stat fileSta;
	char filePath[200];

	pdir = opendir(currentDirPath);
	while((pent = readdir(pdir))!= NULL)
	{
		//计算文件及目录数
		fcounts ++;
	}

	write(sock,&fcounts,sizeof(int));
	closedir(pdir);

	if(fcounts <= 0)
		return;
	else
	{
		pdir=opendir(currentDirPath);
		for(i = 0; i < fcounts;i ++)
		{
			pent = readdir(pdir);
			memset(fileInfo,0,sizeof(fileInfo));

			char *fileName = pent->d_name;
			memset(filePath,0,sizeof(filePath));
			strcpy(filePath,currentDirPath);
			strcat(filePath,"/");
			strcat(filePath, fileName);
			fd = open(filePath, O_RDONLY, S_IREAD);
			fstat(fd,&fileSta);

			if(S_ISDIR(fileSta.st_mode))
			{	//检查是否为目录
				strcat(fileInfo,"dir\t");
				strcat(fileInfo,fileName);
			}
			else
			{
				strcat(fileInfo,"file\t");
				strcat(fileInfo,fileName);
			}
			write(sock,fileInfo,sizeof(fileInfo));
		}
		closedir(pdir);
	}
}

/**
遍历当前目录查看是否有与客户输入的目录名相同的目录,这里要区分普通目录和上一
级目录(即..),若为普通目录则直接在存放当前目录绝对地址的字符串的后面加上新进的
目录,若为上一级目录,则要在存放当前目录绝对地址的字符串中删去最后一个目录,即最
后一个“/”号之后的内容。
*/
void cmd_cd(int sock,char *dirName)
{
	DIR *pdir;
	struct dirent *pent;
	char filename[30];
	int i,fcounts=0;
	int flag=0;

	pdir=opendir(currentDirPath);
	pent=readdir(pdir);
	while(pent!=NULL)
	{
		fcounts++;
		pent=readdir(pdir);
	}
	closedir(pdir);

	if(fcounts<=0)
		return;
	else
	{
		pdir=opendir(currentDirPath);

		for(i=0;i<fcounts;i++)
		{
			pent=readdir(pdir);

			if(strcmp(pent->d_name,dirName)==0)
			{
				if(strcmp(dirName,"..") == 0) 
				{
					int i,pos = 0;
					int len=strlen(currentDirPath);
					for(i = len - 1;i >= 0;i --)
					{
						if(currentDirPath[i] == '/')
						{
							pos=i;
							break;
						}
					}
					currentDirPath[pos]='\0';
				}
				else
				{
					strcat(currentDirPath,"/");
					strcat(currentDirPath,dirName);
				}

				char *savePointer = getDirName(currentDirPath);
				strcpy(currentDirName,savePointer);
				flag=1;
				break;
			}
		}

		if(flag==1)
			write(sock,currentDirPath,sizeof(currentDirPath));
		closedir(pdir);
	}
}

void cmd_help(int sock)
{
	int len=strlen(help)+1;
	write(sock,help,len);
}

//把文件内容一部分一部分地读到缓冲区,然后发给客户端
void cmd_get(int sock,char*fileName)
{
	int fd;
	struct stat fileSta;
	long fileSize;
	char filePath[200], buf[dataLen];

	memset(filePath,0,sizeof(filePath));
	strcpy(filePath,currentDirPath);
	strcat(filePath,"/");
	strcat(filePath,fileName);

	fd=open(filePath,O_RDONLY, S_IREAD);
	if(fd != -1)
	{
		fstat(fd,&fileSta);
		fileSize=(long) fileSta.st_size;
		write(sock,&fileSize,sizeof(long));

		memset(buf,0,dataLen);
		while(fileSize > dataLen)
		{
			read(fd,buf,dataLen);
			write(sock,buf,dataLen);
			fileSize=fileSize-dataLen;
		}

		read(fd,buf,fileSize);
		write(sock,buf,fileSize);
		close(fd);
		printf("transfer completed\n");

	}
	else
	{
		fileSize = 0;
		printf("open file %s failed\n",filePath);
		write(sock,&fileSize,sizeof(long));
	}
		
}


//从客户端一部分一部分地读文件到续冲区,然后写入文件。
void cmd_put(int sock,char *fileName)
{
	int fd;
	long fileSize;
	char filePath[200], buf[dataLen];

	strcpy(filePath,currentDirPath);
	strcat(filePath,"/");
	strcat(filePath,fileName);

	fd=open(filePath,O_RDWR|O_CREAT, S_IREAD|S_IWRITE);
	if(fd!=-1)
	{
		memset(buf,0,dataLen);
		read(sock,&fileSize,sizeof(long));

		while(fileSize>dataLen)
		{
			read(sock,buf,dataLen);
			write(fd,buf,dataLen);
			fileSize=fileSize-dataLen;
		}

		read(sock,buf,fileSize);
		write(fd,buf,fileSize);
		close(fd);
		printf("transfer completed\n");
	}
	else
		printf("open file %s failed\n",filePath);
}







