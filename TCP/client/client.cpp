#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/* 加上IP和TCP数据结构 */
#include <netinet/ip.h>
#include <netinet/tcp.h>

//文件读写
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>



#define dataLen 1024 //缓冲区大小
char user_cmd[10]; //客户端出发的命令
char cmd_arg[20]; //客户端输入的文件或目录名
char buf[dataLen]; //缓冲区


void cmd_pwd(int sock,int sockmsg); //处理pwd命令
void cmd_dir(int sock,int sockmsg); //处理dir命令
void cmd_cd(int sock,int sockmsg,char *dirName); //处理cd命令
void cmd_help(int sock,int sockmsg); //处理?命令
void cmd_get(int sock,int sockmsg,char *fileName); //处理get命令
void cmd_put(int sock,int sockmsg,char *fileName); //处理put命令
void cmd_quit(int sock,int sockmsg); //处理put命令

int main(int argc,char *argv[])
{
	int cmd_len,arg_len;
	int sock, sockmsg;
	struct sockaddr_in server, servermsg;
	struct hostent *hp;
	sock = socket(AF_INET,SOCK_STREAM,0);
	sockmsg = socket(AF_INET,SOCK_STREAM,0);

	if (sock < 0 || sockmsg < 0)
	{
		perror("opening stream socket");
		exit(1);
	}

	if(argc != 3){
		printf("Usage:./client <address> <port>\n");
		return 0;
	}

	server.sin_family=AF_INET;
	server.sin_port=htons(atoi(argv[2]));
	inet_pton(AF_INET,argv[1],&server.sin_addr);
	servermsg.sin_family=AF_INET;
	servermsg.sin_port=htons(atoi(argv[2]) + 1);
	inet_pton(AF_INET,argv[1],&servermsg.sin_addr);

	if (connect(sock,(struct sockaddr *)&server,sizeof server)<0
		||connect(sockmsg,(struct sockaddr *)&servermsg,sizeof servermsg)<0)
	{
		perror("connecting stream socket");
		exit(1);
	}

	char help[300];
	read(sock,help,300);
	printf("%s\n",help);
	while(1)
	{ 
		memset(user_cmd,0,10);
		memset(cmd_arg,0,20);
		printf("command: ");
		scanf("%s",user_cmd);
		cmd_len = strlen(user_cmd);

		if(strcmp(user_cmd,"quit") == 0  || strcmp(user_cmd,"$quit") == 0) 
		{
			// quit命令
			// $quit命令停止服务器
			cmd_quit(sock,sockmsg);
			close(sockmsg);
			close(sock);
			printf("connection closed\n\n");
			exit(0);
		}
		else if(strcmp(user_cmd,"?")==0)
			//?命令
			cmd_help(sock,sockmsg);
		else if(strcmp(user_cmd,"pwd")==0)
			//pwd命令
			cmd_pwd(sock,sockmsg);
		else if(strcmp(user_cmd,"dir")==0)
			//dir命令
			cmd_dir(sock,sockmsg);
		else if(strcmp(user_cmd,"cd")==0) 
		{
			// cd命令
			scanf("%s",cmd_arg);
			cmd_cd(sock,sockmsg,cmd_arg);
		}
		else if(strcmp(user_cmd,"get")==0) 
		{ 
			//get命令
			scanf("%s",cmd_arg);
			cmd_get(sock,sockmsg,cmd_arg);
		}
		else if(strcmp(user_cmd,"put")==0) 
		{
		 	// put命令
			scanf("%s",cmd_arg);
			cmd_put(sock,sockmsg,cmd_arg);
		}
		else
			printf("bad command!\n");
	}
}

//向服务器发出pwd命令,然后接收服务器端发来的当前目录名称,并把它打印出来
void cmd_pwd(int sock,int sockmsg)
{
	char dirName[30];
	write(sockmsg,user_cmd,sizeof(user_cmd));
	read(sock,dirName,30);
	printf("%s\n",dirName);
}

//向服务器发出dir命令,然后接收服务器发来的文件及目录数目,打印出来,接着依次接
//收服务器发来的文件或目录信息,打印出来
void cmd_dir(int sock, int sockmsg)
{
	int i, fileNum=0;

	char fileInfo[50];
	write(sockmsg,user_cmd,sizeof(user_cmd));
	read(sock,&fileNum,sizeof(int));
	printf("--------------------------------------------------------\n");
	printf("file number : %d\n",fileNum);
	if(fileNum > 0)
	{
		for(i=0;i<fileNum;i++)
		{
			memset(fileInfo,0,sizeof(fileInfo));
			read(sock,fileInfo,sizeof(fileInfo));
			printf("%s\n",fileInfo);
		}
		printf("--------------------------------------------------------\n");
	}
	else if(fileNum == 0)
	{
		printf("directory of server point is empty.\n");
		return;
	}
	else
	{
		printf("error in command 'dir'\n");
		return;
	}
}


//向服务器发出cd命令,并把目录名传过去,然后接收当前路径,并打印出来。
void cmd_cd(int sock,int sockmsg,char *dirName)
{
	char currentDirPath[200];
	write(sockmsg,user_cmd,sizeof(user_cmd));
	write(sockmsg,cmd_arg,sizeof(cmd_arg));
	read(sock,currentDirPath,sizeof(currentDirPath));
	printf("now in directory : %s\n",currentDirPath);
}

//向服务器发出?命令,然后接收服务器发来的帮助信息
void cmd_help(int sock, int sockmsg)
{
	char help[300];
	write(sockmsg,user_cmd,sizeof(user_cmd));
	read(sock,help,300);
	printf("%s\n",help);
}

//把get命令和文件名发到服务器,然后一部分一部分地接收服务器发来的字节流,缓存到
//缓冲区,然后写到本地文件中
void cmd_get(int sock,int sockmsg,char *fileName)
{
	int fd;
	long fileSize;
	char localFilePath[200];
	write(sockmsg,user_cmd,sizeof(user_cmd));
	write(sockmsg,cmd_arg,sizeof(cmd_arg));
	printf("%s\n%s\n",user_cmd,cmd_arg);
	memset(localFilePath,0,sizeof(localFilePath));
	getcwd(localFilePath,sizeof(localFilePath));
	strcat(localFilePath,"/");
	strcat(localFilePath,fileName);

	fd = open(localFilePath,O_RDWR|O_CREAT, S_IREAD|S_IWRITE);
	if(fd!=-1)
	{
		memset(buf,0,dataLen);
		read(sock,&fileSize,sizeof(long));
		if(fileSize == 0)
		{
			printf("the file is not exist!\n");
			close(fd);
			return ;
		}
		while(fileSize>dataLen)
		{
			read(sock,buf,dataLen);
			write(fd,buf,dataLen);
			fileSize=fileSize-dataLen;
		}
		read(sock,buf,fileSize);
		write(fd,buf,fileSize);
		close(fd);
		printf("download completed\n");
	}
	else
		printf("open file %s failed\n",localFilePath);
}

//与cmd_get命令类似,先向服务器发出put命令和文件名,然后把文件一部分一部分地读
//到缓冲区,然后再发送到服务器。
void cmd_put(int sock,int sockmsg,char* fileName)
{
	
	int fd;
	long fileSize;
	int numRead;
	char filePath[200];
	struct stat fileSta;

	memset(filePath,0,sizeof(filePath));
	getcwd(filePath,sizeof(filePath));
	strcat(filePath,"/");
	strcat(filePath,fileName);
	if(!access(filePath,F_OK))
	{
		fd=open(filePath,O_RDONLY);
		if(fd!=-1)
		{
			write(sockmsg,user_cmd,sizeof(user_cmd));
			write(sockmsg,cmd_arg,sizeof(cmd_arg));

			fstat(fd,&fileSta);
			fileSize=(long) fileSta.st_size;
			write(sock,&fileSize,sizeof(long));
			memset(buf,0,dataLen);
			while(fileSize>dataLen)
			{
				read(fd,buf,dataLen);
				write(sock,buf,dataLen);
				fileSize=fileSize-dataLen;
			}

			read(fd,buf,fileSize);
			write(sock,buf,fileSize);
			close(fd);
			printf("upload completed\n");
		}
		else
		{
			printf("open file %s failed\n",filePath);
		}
			
	}
	else
	{
		printf("the file is not exist!\n");
	}
	
}


//向服务器发出quit命令
void cmd_quit(int sock,int sockmsg)
{
	write(sockmsg,user_cmd,sizeof(user_cmd));
}
