/* 
----------Servidor UDP------------
gcc srvUDP1.c -o srvUDP1
./srvUDP1
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MYPORT 4950 /* puerto donde
los cliente envian los paquetes */ 

#define MAXBUFLEN 256 /* Max. cantidad de bytes que podra recibir en una llamada a recvfrom() */ 


typedef struct segment SEGMENT;

struct segment{
  int segNum;
  int control;
  int lineLen;
  char line[256];
};
  

main(int argc, char *argv[]) 
{ 
  int sockfd; 
  int port;	/* El puerto a utilizar */	

  struct sockaddr_in my_addr;	/* direccion IP y numero de puerto local */ 

  struct sockaddr_in their_addr;	/* direccion IP y numero de puerto del cliente */ 

  /* addr_len contendra el tamanio de la estructura sockadd_in
    y numbytes el numero de bytes recibidos */ 
  int addr_len, numbytes; 
  char buf[MAXBUFLEN];	/* Buffer de recepcion */ 
  char msg[MAXBUFLEN];

  // Para manejo de comandos
  char comando[25];
  char param1[25];
  
  /* Tratamiento de la linea de comandos. */
  if (argc < 2)
  {
    /* No especificó el puerto a escuchar. */
    port=MYPORT;
  }
  else
  {
    /* Asigna a Port el Puerto especificado. */
    port=atoi(argv[1]);
  }
  
  /* se crea el socket */ 
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  { 
    perror("error al crear el socket"); 
    exit(1);
  } 

  /* Se establece la estructura my_addr para luego llamar a bind() */ 
  my_addr.sin_family = AF_INET;  /* host byte order */ 
  my_addr.sin_port = htons(port); /* network byte order (BigEndian)*/ 
  my_addr.sin_addr.s_addr = INADDR_ANY; /* se asigna automaticamente la direccion IP local */ 
  bzero(&(my_addr.sin_zero), 8);  /* rellena con ceros el resto de la estructura */

  /* Se le da un nombre al socket */ 
  if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) 
  {
    perror("error en el bind"); 
    exit(1);
  } 
  
  printf("Servidor a la escucha por el puerto: %d\n",ntohs(my_addr.sin_port));
  
  /* Se reciben los datos */ 
  addr_len = sizeof(struct sockaddr); 
  char buffer[MAXBUFLEN];
  do{
  	if ((numbytes=recvfrom(sockfd, buf, MAXBUFLEN, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) 
  	{
    		perror("error en recvfrom"); 
    		exit(1);
  	} 

  	/* Se visualiza lo recibido */ 
  	buf[numbytes] = '\0'; 
  	printf("\nMensaje Recibido: [%s]",buf);
	printf("\n\tMensaje proveniente de : %s:%d",inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port));
   	printf("\n\tLongitud del paquete en bytes : %d",numbytes);
  	
	
	//Que contiene buf??
	sscanf(buf,"%s %s",&comando, &param1);	// Tiene forma de comando?
	if(strcmp(comando,"DOWNLOAD")==0)
	{
	    int i=0;
	    char * data=0;;
	    int datalen=0;
		
	    printf("\n\tComando Detectado:%s Parámetro:%s",comando, param1);	// Imprime comando y parámetros
	    
	    //Hay que abrir y envíar línea por línea el contenido del archivo de nombre param1
	    
	    FILE *archivo;
	    SEGMENT seg;
	    //char caracter;
 
	    if ((archivo = fopen(param1,"r"))== NULL){		// Si no se pudo abrir el archivo --> Se avisa al Cte (control=2)
                 printf("\n\tError: no se pudo abrir el archivo: %s. \n",param1);
		 
		 seg.line[0]=0;
		 seg.segNum=0;
		 seg.control=htons(2);
		 seg.lineLen=0;
		 printf("\n\tEnviando Indicador de archivo no encontrado (control):[%d]\n",ntohs(seg.control));
		  
		 data=(char*)&seg;
		 datalen=sizeof(SEGMENT)-sizeof(seg.line)+strlen(seg.line);
		
		 if ((numbytes=sendto(sockfd, data, datalen, 0, (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) 
		 {
		    perror("\n\tError al enviar indicador de archivo no encontrado: sendto\n"); 
		    exit(1);
		 }
		 
	    }else{	// Se pudo abrir el archivo
 		printf("\n\tArchivo %s abierto y listo para enviar...\n",param1);
				
		while (fgets(seg.line,256,archivo) != NULL)
		{
		  seg.segNum=i;
		  seg.control=0;
		  seg.lineLen=strlen(seg.line);
		  printf("\n\tEnviando segmento:[%d], Control [%d], \n\t[%d]Caracteres como:%s",seg.segNum,seg.control,seg.lineLen,seg.line);
		  
		  seg.segNum=htons(seg.segNum);
		  seg.control=htons(seg.control);
		  seg.lineLen=htons(seg.lineLen);
		  data=(char*)&seg;
		  datalen=sizeof(SEGMENT)-sizeof(seg.line)+strlen(seg.line);
		  
		  if ((numbytes=sendto(sockfd, data, datalen, 0, (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) 
		  {
		    perror("\n\tError al enviar archivo: sendto\n"); 
		    exit(1);
		  }
		  i++;
		}
		
		seg.line[0]=0;
		seg.control=htons(1);
		seg.segNum=0;
		seg.lineLen=0;
		printf("\n\tEnviando Indicador de fin del archivo (control):[%d]\n",ntohs(seg.control));
		  
		data=(char*)&seg;
		datalen=sizeof(SEGMENT)-sizeof(seg.line)+strlen(seg.line);
		
		if ((numbytes=sendto(sockfd, data, datalen, 0, (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) 
		{
		    perror("\n\tError al enviar EOF: sendto\n"); 
		    exit(1);
		}
		fclose(archivo);
	    }
	    	
	}else{		// No es un comando válido, actua como servidor de echo.
	
	  sprintf(buffer,"ip:%s puerto:%d msg:%s",inet_ntoa(their_addr.sin_addr),ntohs(their_addr.sin_port),buf);
	
	  if ((numbytes=sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) 
	  {
	    perror("Error al enviar mensaje con: sendto"); 
	    exit(1);
	  }

	  printf("Enviados: %d bytes:--%s--\n",numbytes, buffer);
	  
	}
  }while(strcmp(buf,"CLOSE"));
  /* devolvemos recursos al sistema */ 
  close(sockfd); 

	} 
