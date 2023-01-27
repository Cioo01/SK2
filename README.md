# Sieci Komputerowe 2 - Connect Four

Program pozwala użytkownikom zagrać w grę connect four, w której użytkownik wygrywa jeśli 4 jego "pionki" będą ułożone pod rząd w pionie, poziomie lub skosie. Wielowątkowy serwer umożliwia wiele równoległych par rozgrywek, obsługuje rozłączenie graczy, a także zły input od gracza, np. w postaci znaku lub zbyt małej/dużej wartości kolumny.

# Jak zagrać

Aby zagrać w connect four należy podać numer kolumny z przedziału 1-7. Aplikacja automatycznie umieści krążek we właściwym rzędzie.

#Kompilacja

server:
g++ SKserver.cpp -o <nazwa_podana_przez_uzytkownika> -pthread

client:
gcc SKclient.c -o <nazwa_podana_przez_uzytkownika>

# Uruchamianie serwera

./<nazwa_pliku_serwera>

# Uruchamianie klienta
./<nazwa_pliku_clienta> IP PORT

należy podać jako pierwszy argument IP, jako drugi port do którego się łączy client



