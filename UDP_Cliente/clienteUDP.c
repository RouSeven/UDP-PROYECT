/* 
----------Cliente UDP------------
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MYPORT 4950 /* el puerto donde se enviaran los datos */ 

#define MAXBUFLEN 256 /* Max. cantidad de bytes que podra recibir en una llamada a recvfrom() */ 


typedef struct segment SEGMENT;

struct segment{
  int segNum;
  int control;
  int lineLen;
  char line[256];
};

int main(int argc, char *argv[]) 
{ 

  int sockfd; 
  int port=MYPORT;
  char msg[MAXBUFLEN];
  char * ip;
  
  struct sockaddr_in their_addr; /* Almacenara la direccion IP y numero de puerto del servidor */ 
  int addr_len, numbytes; 
  char buf[MAXBUFLEN];

  if (argc >= 2)
  {
    ip = argv[1];
    
    if (argc == 3) 
    {
      port = atoi(argv[2]);
    }
  }
  else
  { 
    fprintf(stderr,"Uso: cteUDP <ip> [puerto]\n");
    exit(1);
  } 

  /* Creamos el socket */ 
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  {
    perror("Error al crear socket"); 
    exit(1);
  } 

  their_addr.sin_family = AF_INET; /* host byte order - LE*/ 
  their_addr.sin_port = htons(MYPORT); /* network byte order - BE*/ 
  their_addr.sin_addr.s_addr=inet_addr(ip);
  bzero(&(their_addr.sin_zero), 8); 
  addr_len = sizeof(struct sockaddr);

  do{
  	/* Solicitamos mensaje */ 
  	printf("\nMensaje a enviar: ");
  	gets(msg);   
  
  	/* enviamos el mensaje, esta linea contiene una barra invertida al final, indicando que sigue abajo*/ 
  	if ((numbytes=sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) 
  	{
    	perror("Error al enviar mensaje con: sendto"); 
    	exit(1);
  	} 

  	printf("\tenviados %d bytes hacia %s\n",numbytes,inet_ntoa(their_addr.sin_addr));
	
	
	// Para manejo de comandos
	char comando[25];
	char param1[25];
	char * cm=comando;
	char * par=param1;
  
	//Que contiene buf??
	sscanf(msg,"%s %s",&comando, &param1);	// Tiene forma de comando?
	if(strcmp(comando,"DOWNLOAD")==0)
	{
	    printf("\tComando:%s Parámetro:%s",comando, param1);	// Imprime comando y parámetros
	    
	    //Hay que crear el archivo de nombre param1 a recibir línea por línea
	    
	    FILE *archivo;
	    SEGMENT seg;
	    //char caracter;
	    
	    strcat(param1,".copy");
 
	    if ((archivo = fopen(param1,"w"))== NULL){		// No se pudo crear el archivo.
                 printf("\n\tError: no se pudo crear el archivo: %s. \n",param1);
		 
	    }else{	// Archivo creado OK
 		printf("\n\tArchivo %s creado y listo para recibir...\n",param1);
		int i=0;
		char * data=0;;
		int datalen=0;
		
		do{
		  
		  if ((numbytes=recvfrom(sockfd, buf, MAXBUFLEN, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) 
		  {
		    perror("\n\tError: No se pudo realizar la recepción de datos con: recvfrom\n"); 
		    exit(1);
		  } 

		  /* Se visualiza lo recibido */ 
		  //printf("paquete proveniente de : %s\n",inet_ntoa(their_addr.sin_addr));
		  //printf("longitud del paquete en bytes : %d\n",numbytes);
		  buf[numbytes] = '\0'; 
		  //printf("el paquete contiene : %s\n",buf);
		  
		  seg.segNum=ntohs(((SEGMENT *)buf)->segNum);
		  seg.control=ntohs(((SEGMENT *)buf)->control);
		  seg.lineLen=ntohs(((SEGMENT *)buf)->lineLen);
		  sprintf(seg.line,"%s",((SEGMENT *)buf)->line);
		  
		  printf("\n\tSegmento recibido:[%d], Control:[%d]\n\t[%d]Caracteres como:[%s]\n",seg.segNum,seg.control,seg.lineLen,seg.line);
		  
		  if(seg.control==0)	// Cuando se alcanza el EOF, el servidor envía -1 como longitud de línea
		    fputs(seg.line, archivo);
		  else if(seg.control==2)
		  {
		    printf("\n\tArchivo no encontrado en el servidor...\n");
		    break;
		  }
		  else if(seg.control==1)
		  {
		    printf("\n\tFin de archivo recibido en el segmento:[%d], [%d]Caracteres como:[%s]\n",seg.segNum,seg.lineLen,seg.line);
		    break;
		  }
		  
		}while (seg.control==0);
		
	    }
	    fclose(archivo);
	
	}else{		// No es un comando válido, simplemente imprime lo que recibe del servidor de echo.
	

	  if ((numbytes=recvfrom(sockfd, buf, MAXBUFLEN, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) 
	  {
		perror("\n\terror en recvfrom"); 
    		exit(1);
	  } 

	  /* Se visualiza lo recibido */ 
	  printf("\n\tpaquete proveniente de : %s\n",inet_ntoa(their_addr.sin_addr));
	  printf("\n\tlongitud del paquete en bytes : %d\n",numbytes);
	  buf[numbytes] = '\0'; 
	  printf("\n\tel paquete contiene : %s\n",buf);
  
	}


  }while(strcmp("CLOSE",msg));
  close(sockfd); 

  return 0; 

} 
