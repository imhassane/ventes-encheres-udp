/* emetteur portReceveur machineReceveur */

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

int main(int argc, char **argv)
{
    int sock, sock_selling, envoye, recu;
    struct sockaddr_in adresseReceveur;
    int lgadresseReceveur;
    struct hostent *hote;

    int stay_in_market = 1;
    char message[20];

    /* cr'eation de la socket */
    if ((sock = socket( AF_INET, SOCK_DGRAM, 0 )) == -1) 
    {
        perror("socket sock"); 
        exit(1);
    }

    if(sock_selling = socket(AF_INET, SOCK_DGRAM, 0) == -1) {
        perror("socket socket_selling");
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
    }
    


    close(sock);
}