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
    char message[300];
    int my_price = 0, suggest_my_price = 0;

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

        if(strcmp(message, "start") == 0) {
            puts("La vente aux encheres a commence");
        } else {
            puts(message);

	        // puts("Souhaitez-vous rester dans la vente aux encheres - (1: Oui / 0: Non)");
            // scanf("%d", &stay_in_market);
            // Si le client veut rester dans la vente,
            // On demande si il veut proposer son prix
            // si oui, on le lui demande et on l'envoie au serveur.
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
    }



    close(sock);
}