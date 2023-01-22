#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h> 
#include <unistd.h> 
#include <pthread.h>

#define ROWS 6
#define COLUMNS 7
#define PORT 1010

using namespace std;

char client_message[2000];
char buffer[1024];

class ConnectFour{
public:
    char board[ROWS][COLUMNS];

    ConnectFour(){
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLUMNS; j++) {
                board[i][j] = ' ';
            }
        }
    }
    
    int place_piece(char player, int column) {
        // przy podaniu numeru mniejszego/wiekszego od rozmiaru planszy zwroc 0
        if (column < 1 || column > COLUMNS) {
            return 0;
        }

        // znajdz pierwsze wolne miejsce na planszy
        int row = ROWS - 1;
        while (row >= 0 && board[row][column - 1] != ' ') {
            row--;
        }

        // przy pelnej kolumnie, zwroc 2
        if (row < 0) {
            return 2;
        }

        board[row][column - 1] = player;
        return 1;
    }

    int check_winner() {
        // sprawdz czy ktos wygral w poziomie
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLUMNS - 3; j++) {
                if (board[i][j] != ' ' &&
                    board[i][j] == board[i][j + 1] &&
                    board[i][j] == board[i][j + 2] &&
                    board[i][j] == board[i][j + 3]) {
                    return 1;
                }
            }
        }

        // sprawdz czy ktos wygral w pionie
        for (int i = 0; i < ROWS - 3; i++) {
            for (int j = 0; j < COLUMNS; j++) {
                if (board[i][j] != ' ' &&
                    board[i][j] == board[i + 1][j] &&
                    board[i][j] == board[i + 2][j] &&
                    board[i][j] == board[i + 3][j]) {
                    return 1;
                }
            }
        }
        // przekatne
        for (int i = 0; i < COLUMNS - 3; i++) {
            for (int j = 0; j < ROWS; j++) {
                // idac w dol do prawej
                if (board[j][i] != ' ' &&
                    board[j][i] == board[j + 1][i + 1] &&
                    board[j][i] == board[j + 2][i + 2] &&
                    board[j][i] == board[j + 3][i + 3]) {
                    return 1;
                }

                // w gore do prawej
                if (j >= 3 &&
                    board[j][i] != ' ' &&
                    board[j][i] == board[j - 1][i + 1] &&
                    board[j][i] == board[j - 2][i + 2] &&
                    board[j][i] == board[j - 3][i + 3]) {
                    return 1;
                }
            }
        }

        // sprawdz od najbardziej po lewej kolumny
        for (int i = 0; i < COLUMNS - 3; i++) {
            for (int j = 0; j < ROWS - 3; j++) {
                // w dol do prawej
                if (board[j][i] != ' ' &&
                    board[j][i] == board[j + 1][i + 1] &&
                    board[j][i] == board[j + 2][i + 2] &&
                    board[j][i] == board[j + 3][i + 3]) {
                    return 1;
                }
            }
        }
    }
};

struct room{
    pthread_t player1;
    pthread_t player2;

    ConnectFour board;
    int sockets[2];
    int game_id = 0;
    int which_player = 0;
    int players_in_room = 0;
};

int room_counter = 0;
room rooms[1000];

void *player1(void* arg){

    int room_no = room_counter;
    int n;
    char* message;

    printf("[PLAYER 1] has joined. ROOM ID: %d\n", room_no);

    message = (char*) malloc(sizeof("gamestart"));
    strcpy(message, "gamestart");
    send(rooms[room_no].sockets[0], message, sizeof(message) + 10, 0);

    for (;;){

        if (rooms[room_no].which_player == 1){

            n = recv(rooms[room_no].sockets[0], client_message, 2000, 0);

            if (strstr(client_message, "disconnect") != NULL || n < 1) {
                break;
            }

            char* message;
            
            sleep(1);

            if (strlen(client_message) > 1){

                sleep(1);
                message = (char*) malloc(sizeof("fail"));
                strcpy(message, "fail");
                send(rooms[room_no].sockets[0], message, sizeof(message) + 10, 0);
            }
            
            else if(client_message[0] - 48 > 7 || client_message[0] - 48 < 1){

                sleep(1);
                message = (char*)malloc(sizeof("fail"));
                strcpy(message, "fail");
                send(rooms[room_no].sockets[0], message, sizeof(message) + 10, 0);
            }

            else {

                int verify_move = rooms[room_no].board.place_piece('o', client_message[0] - 48);
                int check_win = rooms[room_no].board.check_winner();

                if(verify_move == 0 || verify_move == 2){
                    sleep(1);
                    message = (char*)malloc(sizeof("fail"));
                    strcpy(message, "fail");
                    send(rooms[room_no].sockets[0], message, sizeof(message), 0);
                }

                else if(check_win == 1){

                    sleep(1);
                    char client_notification[50];
                    strcpy(client_notification, "placement ");
                    strcat(client_notification, &client_message[0]);
                    message = (char*)malloc(sizeof(client_notification));
                    strcpy(message, client_notification);

                    send(rooms[room_no].sockets[0], message, sizeof(message) + 10, 0);
                    send(rooms[room_no].sockets[1], message, sizeof(message) + 10, 0);

                    sleep(1);
                    char winner_announcement[40] = "WP P1!\n";
                    message = (char*)malloc(sizeof(winner_announcement));
                    strcpy(message, winner_announcement);

                    send(rooms[room_no].sockets[0], message, sizeof(message) + 10, 0);
                    send(rooms[room_no].sockets[1], message, sizeof(message) + 10, 0);

                    sleep(1);
                    message = (char*)malloc(sizeof("disconnect"));
                    strcpy(message, "disconnect");

                    send(rooms[room_no].sockets[0], message, sizeof(message) + 10, 0);
                    send(rooms[room_no].sockets[1], message, sizeof(message) + 10, 0);
                    
                    break;
                }

                else if (verify_move == 1){

                    sleep(1);
                    char client_notification[50];
                    strcpy(client_notification, "placement ");
                    strcat(client_notification, &client_message[0]);
                    message = (char*)malloc(sizeof(client_notification));
                    strcpy(message, client_notification);
                    send(rooms[room_no].sockets[0], message, sizeof(message) + 10, 0);

                    rooms[room_no].which_player = 2;
                    sleep(1);
                    send(rooms[room_no].sockets[1], message, sizeof(message) + 10, 0);
                    sleep(1);

                    message = (char*)malloc(sizeof("srvready"));
                    strcpy(message, "srvready");
                    send(rooms[room_no].sockets[1], message, sizeof(message) + 10, 0);
                    memset(&client_message, 0, sizeof (client_message));
                }
            }
        }
    }

    rooms[room_no].which_player = 2;
    char terminate_announcement[30] = "disconnect";

    send(rooms[room_no].sockets[1], terminate_announcement, sizeof(terminate_announcement) + 10, 0);
    close(rooms[room_no].sockets[0]);
    close(rooms[room_no].sockets[1]);

    printf("[GAME %d] Player one has left the game.\n", room_no);
    pthread_exit(NULL);
}

void *player2(void* arg)
{
    int room_no = room_counter;
    int n;
    printf("[PLAYER 2] has joined. ROOM ID: %d\n", room_no);
    char* message = (char*) malloc(sizeof("srvready"));
    sleep(1);
    rooms[room_no].which_player = 1;

    message = (char*) malloc(sizeof("gamestart"));
    strcpy(message, "gamestart");
    send(rooms[room_no].sockets[1], message, sizeof(message) + 10, 0);

    char game_start[40] = "srvready";
    send(rooms[room_no].sockets[0], game_start, sizeof(game_start) + 10, 0);

    for (;;){

        if (rooms[room_no].which_player == 2){

            n = recv(rooms[room_no].sockets[1], client_message, 2000, 0);

            if (strstr(client_message, "disconnect") != NULL || n < 1){
                break;
            }
            
            sleep(1);

            if (strlen(client_message) > 1){

                sleep(1);
                message = (char*) malloc(sizeof("fail"));
                strcpy(message, "fail");
                send(rooms[room_no].sockets[1], message, sizeof(message), 0);
            }

            else if (client_message[0] - 48 > 7 || client_message[0] - 48 < 1){

                sleep(1);
                message = (char*) malloc(sizeof("fail"));
                strcpy(message, "fail");
                send(rooms[room_no].sockets[1], message, sizeof(message), 0);
            }

            else {

                int verify_move = rooms[room_no].board.place_piece('x', client_message[0]- 48);
                int check_win = rooms[room_no].board.check_winner();

                if (verify_move == 0 || verify_move == 2){

                    sleep(1);
                    message = (char*) malloc(sizeof("fail"));
                    strcpy(message, "fail");
                    send(rooms[room_no].sockets[1], message, sizeof(message), 0);
                }

                else if (check_win == 1){

                    sleep(1);

                    char client_notification[50];
                    strcpy(client_notification, "placement ");
                    strcat(client_notification, &client_message[0]);

                    message = (char*) malloc(sizeof(client_notification));
                    strcpy(message, client_notification);

                    send(rooms[room_no].sockets[0], message, sizeof(message) + 10, 0);
                    send(rooms[room_no].sockets[1], message, sizeof(message) + 10, 0);
                    
                    sleep(1);
                    char winner_announcement[40] = "WP P2!.\n";
                    message = (char*) malloc(sizeof(winner_announcement));
                    strcpy(message, winner_announcement);

                    send(rooms[room_no].sockets[0], message, sizeof(message) + 10, 0);
                    send(rooms[room_no].sockets[1], message, sizeof(message) + 10, 0);
                    
                    sleep(1);
                    message = (char*) malloc(sizeof("disconnect"));
                    strcpy(message, "disconnect");
                    send(rooms[room_no].sockets[0], message, sizeof(message) + 10, 0);
                    send(rooms[room_no].sockets[1], message, sizeof(message) + 10, 0);
                    
                    break;
                }

                else if (verify_move == 1){

                    sleep(1);

                    char client_notification[50];
                    strcpy(client_notification, "placement ");
                    strcat(client_notification, &client_message[0]);
                    message = (char*) malloc(sizeof(client_notification));
                    strcpy(message, client_notification);

                    send(rooms[room_no].sockets[1], message, sizeof(message) + 10, 0);
                    rooms[room_no].which_player = 1;
                    sleep(1);
                    
                    send(rooms[room_no].sockets[0], message, sizeof(message) + 10, 0);
                    sleep(1);
                    
                    message = (char*)malloc(sizeof("srvready"));
                    strcpy(message, "srvready");
                    send(rooms[room_no].sockets[0], message, sizeof(message) + 10, 0);
                    memset(&client_message, 0, sizeof (client_message));
                } 
            }
        }
    }

    rooms[room_no].which_player = 1;
    char terminate_announcement[30] = "disconnect";

    send(rooms[room_no].sockets[0], terminate_announcement, sizeof(terminate_announcement) + 10, 0);
    close(rooms[room_no].sockets[0]);
    close(rooms[room_no].sockets[1]);
    printf("[GAME %d] Player two has left the game.\n", room_no);
    pthread_exit(NULL);
}

int main(){

    int server_socket;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;
    int connected_users = 0;

    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
    bind(server_socket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    if (listen(server_socket,50)==0){

        printf("Server started.\n");
    }

    else {

        printf("Error\n");
    }

    pthread_t thread_id;

    while(1){

        addr_size = sizeof(serverStorage);
        sleep(1);

        if (connected_users == 2){

            room_counter++;
            connected_users = 0;
        }

        rooms[room_counter].sockets[rooms[room_counter].players_in_room] = accept(server_socket, (struct sockaddr*)&serverStorage, &addr_size);
        
        connected_users++;

        if (rooms[room_counter].players_in_room == 0){

            rooms[room_counter].game_id = room_counter;

            if (pthread_create(&rooms[room_counter].player1, NULL, player1, &rooms[room_counter].sockets[rooms[room_counter].players_in_room]) != 0){

                printf("Failed to create player 1 thread\n");
            }

            else {

                pthread_detach(rooms[room_counter].player1);
                rooms[room_counter].players_in_room = 1;
            }
        }

        else

        if(rooms[room_counter].players_in_room == 1){

            rooms[room_counter].game_id = room_counter;

            if (pthread_create(&rooms[room_counter].player2, NULL, player2, &rooms[room_counter].sockets[rooms[room_counter].players_in_room]) != 0){

                printf("Failed to create player 2 thread\n");
            }

            else {

                pthread_detach(rooms[room_counter].player2);
                rooms[room_counter].players_in_room = 2;
            }
        }
    }

    close(server_socket);

    return 0;
}
