/* receveur portReceveur */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10


void arretParControlC(int sig) 
{
	printf("terminaison par un Control-C\n");
	fflush(stdout);
	exit(1);
	/* Actions a faire pour la partie 1.3*/

}

int main (int argc, char **argv)
{
    int sock, sock_selling, recu, envoye;

    struct sockaddr_in adresseLocale;
    int lgadresseLocale;

    struct sockaddr_in adresseEmetteur;
    int lgadresseEmetteur;

    struct sockaddr_in adresseTopClient;

    struct hostent *hote;
    struct sigaction action;

    char message[200] ;
    char product_description[] = "Nom: IPhone, Description: telephone tres intelligent";

    int wait_client = 1, nb_client = 0;
    int end_selling = 0, max_price = 0, recv_price = 0, current_price = 0;

    struct sockaddr_in liste_clients[MAX_CLIENTS];

    /* handler de signal SIGINT */
    action.sa_handler = arretParControlC;
    sigaction(SIGINT, &action, NULL);

    /* cr'eation de la socket */
    if ((sock = socket( AF_INET, SOCK_DGRAM, 0 )) == -1) 
        {
        perror("socket"); 
        exit(1);
        }

    /* pr'eparation de l'adresse locale : port + toutes les @ IP */
    adresseLocale.sin_family = AF_INET;
    adresseLocale.sin_port = htons(atoi(argv[1]));
    adresseLocale.sin_addr.s_addr = htonl(INADDR_ANY);

    /* attachement de la socket a` l'adresse locale */
    lgadresseLocale = sizeof(adresseLocale);
    if ((bind( sock, (struct sockaddr *) &adresseLocale, lgadresseLocale )) == -1) 
        {
        perror("bind"); 
        exit(1);
        }

    puts("En attente des potentiels acheteurs");
    while(wait_client == 1 && nb_client < MAX_CLIENTS ) {

        /* reception d'un message */
        if ((recu = recvfrom( sock, message, sizeof(message), 0, (struct sockaddr *) &adresseEmetteur, &lgadresseEmetteur )) == -1) 
        {
            perror("recvfrom un message"); 
            close(sock); 
            exit(1);
        }
        if(strcmp(message, "demande") == 0) {
            liste_clients[nb_client] = adresseEmetteur;
 
            strcpy(message, "accept");

            if ((envoye = sendto( sock, message, strlen(message)+1, 0, (struct sockaddr *) &adresseEmetteur, lgadresseEmetteur )) != strlen(message)+1) 
            {
                perror("sendto accept message"); 
                close(sock); 
                exit(1);
            }
            nb_client++;
            puts("Nouveau client");
        }

        puts("Voulez-vous accepter plus de client (1 - Oui / 0 - Non)");
        scanf("%1d", &wait_client);

    }
    close(sock);
    if(( bind(sock_selling, (struct  sockaddr *) &adresseLocale, lgadresseLocale)) == -1) {
	perror("bind sock_selling");
	exit(1);
    }
    puts("Debut des encheres");
    printf("\t--%d acheteurs participent\n", nb_client);

    // On définit le prix de base avant le début des encheres.
    puts("Entrez le prix de base du produit");
    scanf("%d", &current_price);

    // Envoie du message pour annoncer le debut de la vente aux encheres.
    puts("Avertissement des clients");
    int i;
    for(i = 0; i < nb_client; i++) {
        strcpy(message, "start");
        if ((envoye = sendto( sock_selling, message, strlen(message)+1, 0, (struct sockaddr *) &liste_clients[i], lgadresseEmetteur )) != strlen(message)+1) 
        {
            perror("sendto start message"); 
            close(sock_selling); 
            exit(1);
        }
        if ((envoye = sendto( sock_selling, product_description, strlen(product_description)+1, 0, (struct sockaddr *) &liste_clients[i], lgadresseEmetteur )) != strlen(product_description)+1) 
        {
            perror("sendto description message"); 
            close(sock_selling); 
            exit(1);
        }

        sprintf(message, "Prix du produit: %d", current_price);
        if ((envoye = sendto( sock_selling, message, strlen(message) + 1, 0, (struct sockaddr *) &liste_clients[i], lgadresseEmetteur )) != strlen(message) + 1) 
        {
            perror("sendto price message"); 
            close(sock_selling); 
            exit(1);
        }
    }


    while(end_selling == 0) {
        if((recu = recvfrom(sock_selling, &recv_price, sizeof(recv_price), 0, (struct sockaddr *) &adresseEmetteur, &lgadresseEmetteur)) == -1) {
		perror("recvfrom end_selling");
		close(sock_selling);
		exit(1);
        }
        if(recv_price > max_price) {
        	max_price = recv_price;
        	adresseTopClient = adresseEmetteur;
        	// On avertit tout le monde du nouveau prix.
        	sprintf(message, "Le nouveau prix est: %d", max_price);
        	for(i=0; i<nb_client; i++) {
			if((envoye = sendto(sock_selling, message, strlen(message) + 1, 0, (struct sockaddr *) &liste_clients[i], lgadresseEmetteur)) != strlen(message) + 1) {
			   perror("sendto nouveau prix max");
			   close(sock_selling);
			   exit(1);
			}
        	}

        }
    }

    close(sock);
}
