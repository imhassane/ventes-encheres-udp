#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>

int main(int argc, char **argv)
{
    int sock, envoye, recu;
    struct sockaddr_in adresseReceveur;
    int lgadresseReceveur;
    struct hostent *hote;

    int stay_in_market = 1;
    char message[300];
    int my_price = 0, suggest_my_price = 0;

    // Version avec le select
    struct timeval timeout;
    fd_set lect;
    int fd;

    timeout.tv_sec = 10;

    // Entrée standard.
    fd = 0;

    /* cr'eation de la socket */
    if ((sock = socket( AF_INET, SOCK_DGRAM, 0 )) == -1) 
    {
        perror("socket sock"); 
        exit(1);
    }

    /* recherche de l'@ IP de la machine distante */
    if ((hote = gethostbyname(argv[2])) == NULL)
    {
        perror("gethostbyname"); 
        close(sock); 
        exit(2);
        }

    /* pr'eparation de l'adresse distante : port + la premier @ IP */
    adresseReceveur.sin_family = AF_INET;
    adresseReceveur.sin_port = htons(atoi(argv[1]));
    bcopy(hote->h_addr, &adresseReceveur.sin_addr, hote->h_length);
    printf("L'adresse en notation pointee %s\n", inet_ntoa(adresseReceveur.sin_addr));

    strcpy(message,"demande");

    lgadresseReceveur = sizeof(adresseReceveur);
    if ((envoye = sendto( sock, message, strlen(message)+1, 0, (struct sockaddr *) &adresseReceveur, lgadresseReceveur )) != strlen(message)+1) 
    {
    perror("sendto un message");
    close(sock); 
    exit(1);
    }

    if((recu = recvfrom(sock, message, sizeof(message), 0, (struct sockaddr *) &adresseReceveur, &lgadresseReceveur)) == -1 ) {
        perror("recvfrom message d'acceptation");
        close(sock);
        exit(1);
    }
    puts("Vous avez ete accepte dans la vente aux encheres");


    while(stay_in_market == 1) {
    	FD_ZERO(&lect);
    	FD_SET(sock, &lect);
    	FD_SET(fd, &lect);

	select(sock+1, &lect, NULL, NULL, &timeout);

	if(FD_ISSET(fd, &lect)) {

            puts("Souhaitez-vous proposer un prix pour ce produit - (1: Oui / 0: Non)");
            scanf("%d", &suggest_my_price);
            if(stay_in_market == 1 && suggest_my_price == 1) {
                puts("--Entrez votre prix");
                scanf("%d", &my_price);
                if(my_price > 0) {
                if((envoye = sendto(sock, &my_price, sizeof(my_price), 0, (struct sockaddr *)  &adresseReceveur, lgadresseReceveur)) != sizeof(my_price)) {
                        perror("sendto my_price");
                        close(sock);
                        exit(1);
                }
                suggest_my_price = 0;
                }
            }
	}

	if(FD_ISSET(sock, &lect)) {
		// Traitements entrée socket
		if((recu = recvfrom(sock, message, sizeof(message), 0, (struct sockaddr *) &adresseReceveur, &lgadresseReceveur)) == -1 ) {
           		 perror("recvfrom message d'acceptation");
            		close(sock);
            		exit(1);
        	}

		if(strcmp(message, "start") == 0) {
            		puts("La vente aux encheres a commence");
        	} else {
            		puts(message);
            	}


        	if(strcmp(message, "NULL") == 0) {
            		puts("Fin des encheres");
            		if((recu = recvfrom(sock, message, sizeof(message), 0, (struct sockaddr *) &adresseReceveur, &lgadresseReceveur)) == -1 ) {
                		perror("recvfrom fin enchere prix final");
                		close(sock);
               			 exit(1);
            		}
            		printf("Le gagnant l'a remportee avec la proposition de %s euros.\n", message);
            		if(atoi(message) == my_price) {
               			 puts("Je viens de remporter les encheres");
               			 close(sock);
            		}

            		stay_in_market = 0;
            		break;

		}

        }
    }



    close(sock);
}
