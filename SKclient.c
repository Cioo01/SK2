#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <unistd.h> 
#include <pthread.h>
#include <stdbool.h>

#define ROWS 6
#define COLUMNS 7

char board[ROWS][COLUMNS];

void initialize_board() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            board[i][j] = ' ';
        }
    }
}

void print_board() {
    printf("  1 2 3 4 5 6 7\n");

    for (int i = 0; i < ROWS; i++) {
        printf("%c ", '1' + i);
        for (int j = 0; j < COLUMNS; j++) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
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

int main(int argc, char** argv){

    initialize_board();

    if (argc < 2) {
        printf("Enter IP Address and port no.\n");
        return -1;
    }
    
    char message[1000];
    char buffer[1024];
    int client_socket;

    struct sockaddr_in server_addr;
    socklen_t addr_size;
    bool response = false;

    int player = 1;

    client_socket = socket(PF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    
    memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));
    addr_size = sizeof(server_addr);
    connect(client_socket, (struct sockaddr *) &server_addr, addr_size);

    for(;;){

        if (recv(client_socket, buffer, 1024, 0) < 0){
            if (!response){
                printf("Connection failed. Wait for server response.\n");
            }
            response = true;
            connect(client_socket, (struct sockaddr *) &server_addr, addr_size);
        }
        else{
            response = false;
            if (strstr(buffer, "gamestart") != NULL){

                print_board();
                printf("You've successfully connected to server, the game should start promptly!\n");
            }
            else
            if (strstr(buffer, "srvready") != NULL || strstr(buffer, "fail") != NULL){
                if (strstr(buffer, "fail") != NULL){
                    printf("Incorrect move, column full or you tried to place the piece out of the board!\n");
                }
                else{
                    printf("Give me a column's number where you would like to place your piece: \n");
                }
                fflush(stdin);
                scanf("%1s", &message);
                fflush(stdin);
                if (strstr(message, "exit") != NULL){
                    printf("Quitting.\n");
                    break;
                }

                if (send(client_socket, message, strlen(message), 0) < 0){
                    printf("Send failed.\n");
                }

                memset(&message, 0, sizeof (message));
            }
            else if (strstr(buffer, "disconnect") != NULL){

                printf("Game's over!\n");
                break;
            }
            else if (strstr(buffer, "placement") != NULL){

                char move = buffer[10];

                if (player == 1){

                    player = 2;
                    place_piece('O', move - 48);
                }
                else{

                    player = 1;
                    place_piece('X', move - 48);
                }
                
                print_board();
            }
            else{
                printf("Server message: %s\n", buffer);
            }
        }
    }

    close(client_socket);
    return 0;
}

