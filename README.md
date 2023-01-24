# Sieci Komputerowe 2 - Connect Four

Program pozwala użytkownikom zagrać w grę connect four, w której użytkownik wygrywa jeśli 4 jego "pionki" będą ułożone pod rząd w pionie, poziomie lub skosie. Wielowątkowy serwer umożliwia wiele równoległych par rozgrywek, obsługuje rozłączenie graczy, a także zły input od gracza, np. w postaci znaku lub zbyt małej/dużej wartości kolumny.

#Kompilacja

server:
g++ SKserver.cpp -o <nazwa_podana_przez_uzytkownika> -pthread

client:
gcc SKclient.c -o <nazwa_podana_przez_uzytkownika>

# Uruchamianie

server:
./<nazwa_pliku_serwera>

Aby uruchomić clienta należy dwa argumenty w następującej kolejności: 1. IP 2. Port do którego się łączy.

client:                  1   2
./<nazwa_pliku_clienta> IP PORT

