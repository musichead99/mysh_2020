/*mysh.c:shell을 구현하는 프로그램, by 정성구. musichead99@naver.com*/
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
#define MAX_BUF 512

//parsing 작업하는 함수//
int tokenize(char str[],char* tok[]) 
{
	int i = 0;
	tok[i] = strtok(str," ");	

	for(;tok[i] != NULL && i < MAX_BUF;)
	{
		i++;
		tok[i] = strtok(NULL," ");
	}

	return i;
}

//일반적인 명령어들과 background processing 실행//
void normal_cmd(char *tok[], int flag)
{
	pid_t fork_return;
	if((fork_return = fork()) < 0)
	{
		printf("fork error\n");
		exit(-1);
	}
	else if(fork_return == 0)
	{	
		if(execvp(tok[0],tok) < 0)
		{
			
			printf("%s : command not found!\n",tok[0]);
			exit(-1);
		}
	}
	else
	{
		//flag가 1이 아니라면 자식 프로세스의 종료를 기다림//
		if(!flag)
		{
			wait();
		}
	}
}

//redirection 구현//
void redirection_cmd(char *tok[],int flag)
{
	pid_t fork_return;
	int i, j, fd;
	char *part1[MAX_BUF], *part2[MAX_BUF];

	for(i=0;i<MAX_BUF;i++)
	{
		if(!strcmp(tok[i],">") || !strcmp(tok[i],"<") || !strcmp(tok[i],">>"))
		break;

		part1[i] = tok[i];
	}
	part1[i] = '\0';
	
	for(i=i+1,j=0;tok[i] != NULL && i < MAX_BUF;i++,j++)
	{
		part2[j] = tok[i];
	}
	part2[j] = '\0';

	//'>', '>>' redirection 구현//
	if(flag == 2 || flag == 4)
	{
		if((fork_return = fork()) < 0)
		{
			printf("fork error\n");
			exit(-1);
		}
		else if(fork_return == 0)
		{
			if(flag == 2)
			{
				fd = open(part2[0],O_RDWR | O_CREAT, 0664);
			}
			else if(flag == 4)
			{
				fd = open(part2[0], O_RDWR | O_CREAT | O_APPEND, 0664);
			}
			dup2(fd,STDOUT_FILENO);
			close(fd);
			if(execvp(part1[0],part1) < 0)
			{
				printf("%s : command not found\n");
				exit(-1);
			}
		}
		else
		{
			wait();
		}
	}
	//'<' redirection 구현//
	else if(flag == 3) 
	{
		if((fork_return = fork()) < 0)
		{
			printf("fork error\n");
			exit(-1);
		}
		else if(fork_return == 0)
		{
			fd = open(part2[0],O_RDWR | O_CREAT, 0664);
			dup2(fd,STDIN_FILENO);
			close(fd);
			if(execvp(part1[0],part1) < 0)
			{
				printf("%s : command not found\n");
				exit(-1);
			}
		}
		else
		{
			wait();
		}
	}
}

//help나 ?시 출력하는 안내문//
void cmd_help(char *tok[])
{
	printf("--------------------mysh--------------------\n");
	printf("|                                          |\n");
	printf("|    쉘의 기본적인 로직을 구현했습니다.    |\n");
	printf("|          *추가로 구현한 기능들*          |\n");
	printf("| cd : change directory                    |\n");
	printf("| exit : exit mysh                         |\n");
	printf("| & : background processing                |\n");
	printf("| >,<,>> : redirection                     |\n");
	printf("| help,? : show this message               |\n");
	printf("--------------------------------------------\n");
}

//shell을 구동하는 함수//
int shell_run(char str[])
{
	char* token[MAX_BUF];
	int flag;
	tokenize(str,token);
	flag = advanced_func(token);
	//명령어를 입력하지 않았거나 &를 맨 앞에 입력한 경우//
	//바로 prompt 출력//
	if(token[0] == NULL)
	{
		//&의 경우는 실제 shell에서 출력되는 메세지 출력//
		if(flag){printf("syntax error near unexpected token '&'\n");}
		return 1;
	}
	//exit일 경우 mysh 종료//
	else if(!strcmp(token[0],"exit")) 
	{
		return 0;
	}
	else if(!strcmp(token[0],"help") || !strcmp(token[0],"?"))
	{
		cmd_help(token);
		return 1;
	}

	switch(flag){
		//background와 일반 명령어 처리//
		case 0: case 1:
			normal_cmd(token,flag);
			break;
		//redirection 처리//
		case 2: case 3: case 4: 
			redirection_cmd(token,flag);
			break;
		//cd 구현//
		case 5: 
			if(chdir(token[1]) < 0)
			{
				printf("chdir error\n");
				break;
			}
			break;
	}
	return 1;
}

//parsing한 명령어를 분석해 flag를 세팅하는 함수//
int advanced_func(char *token[])
{
	int i, flag = 0;

	for(i=0;token[i] != NULL && i < MAX_BUF;i++)
	{
		if(!strcmp(token[i],"&"))
		{
			token[i--] = '\0';
			flag = 1;
		}
		else if(!strcmp(token[i],">"))
		{
			flag = 2;
		}
		else if(!strcmp(token[i],"<"))
		{
			flag = 3;
		}
		else if(!strcmp(token[i],">>"))
		{
			flag = 4;
		}
		else if(!strcmp(token[i],"cd"))
		{
			flag = 5;
		}
	}
	return flag;
}

int main(int argc, char* argv[])
{
	char str[MAX_BUF];
	while(1)
	{
		printf("%s$ ",get_current_dir_name());
		fgets(str,sizeof(str)-1,stdin);
		str[strlen(str)-1] = '\0';
		if(!shell_run(str))
		{
			break;
		}
	}
	return 0;
}
