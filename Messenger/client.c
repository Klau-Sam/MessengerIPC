#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>


#define LOGIN_REQ 1
#define LOGIN_RES 2
#define LOGOUT_REQ 3
#define LOGOUT_RES 4
#define LOG_USERS_REQ 5
#define LOG_USERS_RES 6
#define GROUP_REQ 7
#define GROUP_RES 8
#define ADD_GR_REQ 9
#define ADD_GR_RES 10
#define DEL_GR_REQ 11
#define DEL_GR_RES 12
#define MES_USER_REQ 13
#define MES_USER_RES 14
#define MES_GR_REQ 15
#define MES_GR_RES 16
#define INFO_ABOUT_MESS 20
#define success 1
#define fail 0

// kolejka zwiazana z logowaniem
int req1 = 9190;
// kolejka zwiazana z roszadami w grupie
int req2 = 8080;
// kolejka zwiazana z wiadomosciami
int req3 = 4040;
// odnosnik do indywidualnego klucza kazdego z uzytkownikow
int res = 10300;


typedef struct user {
    long type;
    char login[20];
    char password[20];
    int state;
    int key;
    int respond_sign;
    int my_groups[3];
} user;

typedef struct info {
    long type;
    int answer;
    int key;
} info;

typedef struct group {
    long type;
    int quan;
    char logins[9][20];
    int keys[9];
} group;

typedef struct messenger {
    long type;
    char from[20];
    char mess[256];
    char to[20];
    int group;
} messenger;

user login(){
    char log[20], pas[20];
    printf("Podaj login: \n");
    scanf("%s", log);
    printf("Podaj haslo: \n");
    scanf("%s", pas);

    user L;
    strcpy(L.login, log);
    strcpy(L.password, pas);
    L.type = LOGIN_REQ;
    L.respond_sign = 0;

    int msgid;
    msgid = msgget(req1, IPC_CREAT|0600);

    if (msgsnd(msgid, &L, sizeof(L)-sizeof(long), 0) == -1){
        perror("Problem z wyslaniem zadania o logowaniu");
        exit(1);
    }

    user R;
    sleep(3);

    msgrcv(msgid, &R, sizeof(R)-sizeof(long), LOGIN_RES, 0);
    if (R.respond_sign == 1){
        printf("\nPotwierdzenie logowania uzytkownika: %s.\n", R.login);
    }
    else { printf("\nLogowanie nie powiodlo sie.\n"); }
    return R;
}

info logout(user User){
    User.type = LOGOUT_REQ;

    int msgid;
    msgid = msgget(req1, IPC_CREAT|0600);

    if (msgsnd(msgid, &User, sizeof(User)-sizeof(long), 0) == -1){
        perror("Problem z wyslaniem zadania o wylogowaniu");
        exit(1);
    }

    info F;
    sleep(3);

    msgrcv(msgid, &F, sizeof(F)-sizeof(long), LOGOUT_RES, 0);
    if (F.answer == 1) { printf("\nPotwierdzenie wylogowania uzytkownika: %s.\n", User.login); }
    else { printf("\nWylogowanie nie powiodlo sie.\n"); }
    return F;
}

void show_logged(){
    info F;
    F.type = LOG_USERS_REQ;
    F.answer = 0;

    int msgid;
    msgid = msgget(req1, IPC_CREAT|0600);

    if (msgsnd(msgid, &F, sizeof(F)-sizeof(long), 0) == -1){
        perror("Problem z wyslaniem zadania o pokazaniu zalogowanych uzytkownikow");
        exit(1);
    }

    group Logged;
    sleep(3);

    msgrcv(msgid, &Logged, sizeof(Logged)-sizeof(long), LOG_USERS_RES, 0);

    printf("\nLista zalogowanych:\n");
    int j, q = 1;
    for(j=0; j<Logged.quan; j++){
        printf("%d) %s\n", q, Logged.logins[j]);
        q++;
    }
    return;
}

void show_list_of_groups(user User){
    printf("\nDostepne grupy tematyczne:\n");
    printf("- grupa zerowa (nr 0) ~ tematy polityczne ");
    if (User.my_groups[0] == 1) { printf(" (nalezysz do niej)\n"); }
    else { printf(" (nie nalezysz do niej)\n"); }
    printf("- grupa pierwsza (nr 1) ~ rozmowy o sztuce i muzyce ");
    if (User.my_groups[1] == 1) { printf(" (nalezysz do niej)\n"); }
    else { printf(" (nie nalezysz do niej)\n"); }
    printf("- grupa druga (nr 2) ~ tematy podroznicze ");
    if (User.my_groups[2] == 1) { printf(" (nalezysz do niej)\n"); }
    else { printf(" (nie nalezysz do niej)\n"); }
    return;
}

void show_group(){
    int n;
    printf("\nUzytkownikow z ktorej grupy chcesz zobaczyc? (0-2)\n");
    scanf("%d", &n);

    info F;
    F.type = GROUP_REQ;
    F.answer = 0;
    F.key = n;

    int msgid;
    msgid = msgget(req1, IPC_CREAT|0600);

    if (msgsnd(msgid, &F, sizeof(F)-sizeof(long), 0) == -1){
        perror("Problem z wyslaniem zadania o pokazaniu uzytkownikow z danej grupy");
        exit(1);
    }

    group Chosen;
    sleep(3);

    msgrcv(msgid, &Chosen, sizeof(Chosen)-sizeof(long), GROUP_RES, 0);

    printf("\nGrupa tematyczna nr %d:\n", n);
    int j, q = 1;
    for(j=0; j<Chosen.quan; j++){
        printf("%d) %s\n", q, Chosen.logins[j]);
        q++;
    }
    return;
}

int add_to_group(user User){
    printf("\n");
    int n;
    if (User.my_groups[0] == 0) { printf("Mozesz dolaczyc do grupy nr 0.\n"); }
    if (User.my_groups[1] == 0) { printf("Mozesz dolaczyc do grupy nr 1.\n"); }
    if (User.my_groups[2] == 0) { printf("Mozesz dolaczyc do grupy nr 2.\n"); }
    printf("\nDo ktorej grupy tematycznej chcesz dolaczyc? Podaj numer.\n");
    scanf("%d", &n);
    User.state = n;
    User.type = ADD_GR_REQ;

    int msgid;
    msgid = msgget(req2, IPC_CREAT|0600);

    if (msgsnd(msgid, &User, sizeof(User)-sizeof(long), 0) == -1){
        perror("Problem z wyslaniem zadania o dodaniu do grupy");
        exit(1);
    }

    info F;
    sleep(3);

    msgrcv(msgid, &F, sizeof(F)-sizeof(long), ADD_GR_RES, 0);
    if (F.answer == 1) { printf("\nDodano uzytkownika %s do grupy nr %d.\n", User.login, n); }
    else { printf("\nUzytkownik %s juz jest w danej grupie albo grupa nie istnieje.\n", User.login); }
    return n;
}

int delete_from_group(user User){
    int n;
    printf("\n");
    if (User.my_groups[0] == 1) { printf("Nalezysz do grupy nr 0.\n"); }
    if (User.my_groups[1] == 1) { printf("Nalezysz do grupy nr 1.\n"); }
    if (User.my_groups[2] == 1) { printf("Nalezysz do grupy nr 2.\n"); }
    printf("\nZ ktorej grupy chcesz zostac usuniety?\n");
    scanf("%d", &n);
    User.state = n;
    User.type = DEL_GR_REQ;

    int msgid;
    msgid = msgget(req2, IPC_CREAT|0600);

    if (msgsnd(msgid, &User, sizeof(User)-sizeof(long), 0) == -1){
        perror("Problem z wyslaniem zadania o usunieciu z grupy");
        exit(1);
    }

    info F;
    sleep(3);

    msgrcv(msgid, &F, sizeof(F)-sizeof(long), DEL_GR_RES, 0);
    if (F.answer == 1) { printf("\nUzytkownik %s zostal usuniety z grupy nr %d.\n", User.login, n); }
    else { printf("\nUzytkownika %s nie ma w danej grupie albo grupa nie istnieje.\n", User.login); }
    return n;
}

void message(user User){
    messenger M;
    char c;
    strcpy(M.from, User.login);

    printf("\nDo kogo chcesz napisac? Podaj jej/jego login:\n");
    scanf("%s", M.to);
    while((c = getchar()) != '\n' && c != EOF);
    printf("Tresc wiadomosci (masz do wykorzystania 256 znakow, tylko bez enterow!!!): ");
    fgets(M.mess, sizeof(M.mess), stdin);
    printf("\n");
    fflush(stdin);

    M.type = MES_USER_REQ;

    int msgid;
    msgid = msgget(req3, IPC_CREAT|0600);

    if (msgsnd(msgid, &M, sizeof(M)-sizeof(long), 0) == -1){
        perror("Problem z wyslaniem zadania o wyslaniu wiadomosci do uzytkownika");
        exit(1);
    }

    info F;
    sleep(3);

    msgrcv(msgid, &F, sizeof(F)-sizeof(long), MES_USER_RES, 0);
    if (F.answer == 1) { printf("\nWiadomosc do uzytkownika zostala wyslana.\n"); }
    else { printf("\nWiadomosc nie zostala dostarczona z jakiegos powodu, najprawdopodobniej nie mamy adresata w bazie.\n"); }
    return;
}

void receive(user User){
    info Message_or_not;
    messenger Delivered;

    int msgid, temp, counter = 0;
    temp = res;
    temp = temp + User.key;
    msgid = msgget(temp, IPC_CREAT|0600);

    while (msgrcv(msgid, &Message_or_not, sizeof(Message_or_not)-sizeof(long), INFO_ABOUT_MESS, IPC_NOWAIT) != -1){
        counter = counter + Message_or_not.answer;
    }

    while (counter > 0) {
        msgrcv(msgid, &Delivered, sizeof(Delivered)-sizeof(long), 0, 0);
        printf("\n\nWiadomosc od: %s ", Delivered.from);
        if (Delivered.group != -1) { printf("z grupy nr %d ", Delivered.group); }
        printf("o tresci:\n%s", Delivered.mess);
        printf("\n\n");
        counter--;
    }
    return;
}

void message_to_group(user User){
    printf("\n");
    if (User.my_groups[0] == 1) { printf("Nalezysz do grupy nr 0.\n"); }
    if (User.my_groups[1] == 1) { printf("Nalezysz do grupy nr 1.\n"); }
    if (User.my_groups[2] == 1) { printf("Nalezysz do grupy nr 2.\n"); }
    printf("\n");

    messenger Group_message;
    char c;
    strcpy(Group_message.from, User.login);

    printf("Do ktorej grupy chcesz wyslac wiadomosc? Podaj cyfre.\n");
    scanf("%d", &Group_message.group);
    if (User.my_groups[Group_message.group] == 1) {
        while((c = getchar()) != '\n' && c != EOF);
        printf("Tresc wiadomosci (masz do wykorzystania 256 znakow, tylko bez enterow!!!): ");
        fgets(Group_message.mess, sizeof(Group_message.mess), stdin);
        printf("\n");
        fflush(stdin);
        Group_message.type = MES_GR_REQ;

        int msgid;
        msgid = msgget(req3, IPC_CREAT|0600);

        if (msgsnd(msgid, &Group_message, sizeof(Group_message)-sizeof(long), 0) == -1){
            perror("Problem z wyslaniem zadania o wyslaniu wiadomosci do grupy.");
            exit(1);
        }

        info F;
        sleep(3);

        msgrcv(msgid, &F, sizeof(F)-sizeof(long), MES_GR_RES, 0);
        if (F.answer == 1) { printf("\nWiadomosc do grupy zostala wyslana.\n"); }
        else { printf("\nWiadomosc nie zostala dostarczona z jakiegos powodu, najprawdopodobniej wybrana grupa adresowa nie istnieje.\n"); }
    } else { printf("\nWiadomosc nie moze byc dostarczona, poniewaz nie nalezysz do danej grupy.\n"); }
    return;
}

int main(){
    printf("Witaj kliencie!\n");

    int n, i = 1, flag = 1, g;
    user User;
    info Information;

    while (i) {
        printf("Jakie dzialanie chcesz podjac? Wybierz numer:\n");
        printf("\t1 - Chce sie zalogowac.\n\t2 - Chce zobaczyc zalogowanych uzytkownikow.\n\t3 - Chce zobaczyc dostepne grupy.\n\t4 - Chce zobaczyc osoby z danej grupy tematycznej.\n\t5 - Chce dolaczyc do grupy.\n\t6 - Chce odejsc z grupy.\n");
        printf("\t7 - Chce wyslac wiadomosc do uzytkownika.\n\t8 - Chce wyslac wiadomosc do calej grupy.\n\t9 - Chce odczytac moje wiadomosci.\n\t10 - Chce sie wylogowac.\nJakakolwiek inna liczba - koniec zabawy.\n");
        scanf("%d", &n);
        switch (n){
        case 1:
            if (flag) {
                User = login();
                if (User.respond_sign) { flag = 0; }
            } else { printf("\nJuz jestes zalogowany.\n"); }
            break;
        case 2:
            if (!flag) {
                show_logged();
            } else { printf("\nMusisz sie najpierw zalogowac.\n"); }
            break;
        case 3:
            if (!flag) {
                show_list_of_groups(User);
            } else { printf("\nMusisz sie najpierw zalogowac.\n"); }
            break;
        case 4:
            if (!flag) {
                show_group();
            } else { printf("\nMusisz sie najpierw zalogowac.\n"); }
            break;
        case 5:
            if (!flag) {
                g = add_to_group(User);
                User.my_groups[g] = 1;
            } else { printf("\nMusisz sie najpierw zalogowac.\n"); }
            break;
        case 6:
            if (!flag) {
                g = delete_from_group(User);
                User.my_groups[g] = 0;
            } else { printf("\nMusisz sie najpierw zalogowac.\n"); }
            break;
        case 7:
            if (!flag) {
                message(User);
            } else { printf("\nMusisz sie najpierw zalogowac.\n"); }
            break;
        case 8:
            if (!flag) {
                message_to_group(User);
            } else { printf("\nMusisz sie najpierw zalogowac.\n"); }
            break;
        case 9:
            if (!flag) {
                receive(User);
            } else { printf("\nMusisz sie najpierw zalogowac.\n"); }
            break;
        case 10:
            if (!flag) {
                Information = logout(User);
                if (Information.answer) { flag = 1; }
            } else { printf("\nMusisz sie najpierw zalogowac.\n"); }
            break;
        default:
            i = 0;
            break;
        }
        printf("\n");
    }
    return 0;
}


