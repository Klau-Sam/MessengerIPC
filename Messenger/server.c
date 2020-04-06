#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


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


void login_respond(user Users[]){
    user L;
    user R;
    int j, processing = 0;

    int msgid;
    msgid = msgget(req1, 0600);

    if (msgrcv(msgid, &L, sizeof(L)-sizeof(long), LOGIN_REQ, IPC_NOWAIT) != -1){
        printf("Przyjalem zadanie.\n");
        for(j=0; j<9; j++){
            if((strcmp(L.login, Users[j].login) == 0) && (strcmp(L.password, Users[j].password) == 0) && (Users[j].state == 0)){
                Users[j].state = 1;
                strcpy(R.login, Users[j].login);
                strcpy(R.password, Users[j].password);
                R.my_groups[0] = Users[j].my_groups[0];
                R.my_groups[1] = Users[j].my_groups[1];
                R.my_groups[2] = Users[j].my_groups[2];
                R.respond_sign = 1;
                R.key = Users[j].key;
                processing = 1;
                printf("UDALO SIE!\nUzytkownik %s zostal zalogowany!\n", L.login);
                break;
            }
            else if((strcmp(L.login, Users[j].login) == 0) && (strcmp(L.password, Users[j].password) == 0) && (Users[j].state == 1)){
                R.respond_sign = 0;
                processing = 1;
                printf("Uzytkownik %s zalogowal sie juz wczesniej.\n", L.login);
                break;
            }
        }
        if (processing != 1){
            R.respond_sign = 0;
            printf("Logowanie nie powiodlo sie - najprawdopodobniej brak takiego uzytkownika w bazie.\n");
        }
        R.type = LOGIN_RES;
        msgsnd(msgid, &R, sizeof(R)-sizeof(long), 0);
    }

    return;
}

void logout_respond(user Users[]){
    user U;
    info F;
    int j, processing = 0;

    int msgid;
    msgid = msgget(req1, 0600);

    if (msgrcv(msgid, &U, sizeof(U)-sizeof(long), LOGOUT_REQ, IPC_NOWAIT) != -1){
        printf("Przyjalem zadanie.\n");
        for(j=0; j<9; j++){
            if((strcmp(U.login, Users[j].login) == 0) && (strcmp(U.password, Users[j].password) == 0) && (Users[j].state == 1)){
                Users[j].state = 0;
                F.answer = 1;
                processing = 1;
                printf("UDALO SIE!\nUzytkownik %s zostal wylogowany!\n", U.login);
                break;
            }
            else if((strcmp(U.login, Users[j].login) == 0) && (strcmp(U.password, Users[j].password) == 0) && (Users[j].state == 0)){
                F.answer = 0;
                processing = 1;
                printf("Uzytkownik %s musial zostac wylogowany wczesniej.\n", U.login);
                break;
            }
        }
        if (processing != 1){
            F.answer = 0;
            printf("Wylogowanie nie powiodlo sie - najprawdopodobniej brak takiego uzytkownika w bazie.\n");
        }
        F.type = LOGOUT_RES;
        msgsnd(msgid, &F, sizeof(F)-sizeof(long), 0);
    }
    return;
}

void show_logged_respond(user Users[]){
    info F;
    group Logged;
    int j, q = 0;

    int msgid;
    msgid = msgget(req1, 0600);

    if (msgrcv(msgid, &F, sizeof(F)-sizeof(long), LOG_USERS_REQ, IPC_NOWAIT) != -1){
        printf("Przyjalem zadanie.\n");
        for(j=0; j<9; j++){
            if (Users[j].state == 1){
                strcpy(Logged.logins[q], Users[j].login);
                q++;
            }
        }
        Logged.quan = q;
        Logged.type = LOG_USERS_RES;
        msgsnd(msgid, &Logged, sizeof(Logged)-sizeof(long), 0);
    }
    return;
}

void show_group_respond(group Groups[]){
    info F;
    group Chosen;
    int j;

    int msgid;
    msgid = msgget(req1, 0600);

    if (msgrcv(msgid, &F, sizeof(F)-sizeof(long), 7, IPC_NOWAIT) != -1){
        printf("Przyjalem zadanie.\n");
        for(j=0; j<Groups[F.key].quan; j++){
            strcpy(Chosen.logins[j], Groups[F.key].logins[j]);
        }
        Chosen.quan = Groups[F.key].quan;
        Chosen.type = GROUP_RES;
        msgsnd(msgid, &Chosen, sizeof(Chosen)-sizeof(long), 0);
    }
    return;
}

// void show_groups(struct group groups[]){
//     printf("Lista grup:\n");
//     int i, j;
//     for(i=0; i<2; i++){
//         printf("Grupa tematyczna nr %d:\n", i+1);
//         for(j=0; j<9; j++){
//             if(!strcmp(groups[i].logins[j],"\0")) break;
//             printf("%d) %s\n", j+1, groups[i].logins[j]);
//         }
//     }
//     return;
// }

void add_to_group_respond(group Groups[], user Users[]){
    user U;
    info F;
    int j, i, processing = 0;

    int msgid;
    msgid = msgget(req2, 0600);

    if (msgrcv(msgid, &U, sizeof(U)-sizeof(long), ADD_GR_REQ, IPC_NOWAIT) != -1){
        printf("Przyjalem zadanie.\n");
        printf("%s\n", U.login);
        printf("%d\n", Groups[U.state].quan);
        for(i=0; i<Groups[U.state].quan; i++){
            if(!strcmp(Groups[U.state].logins[i], U.login)){
                processing = 1;
                F.answer = 0;
                break;
            }
        }
        if(!processing){
            strcpy(Groups[U.state].logins[Groups[U.state].quan], U.login);
            Groups[U.state].keys[Groups[U.state].quan] = U.key;
            Groups[U.state].quan++;
            for(j=0; j<9; j++){
                if(!strcmp(Users[j].login, U.login)) {
                    Users[j].my_groups[U.state] = 1;
                    break;
                }
            }
            F.answer = 1;
        }
        F.type = ADD_GR_RES;
        msgsnd(msgid, &F, sizeof(F)-sizeof(long), 0);
    }
    return;
}

void delete_from_group_respond(group Groups[], user Users[]){
    user U;
    info F;
    int i, processing = 0;

    int msgid;
    msgid = msgget(req2, 0600);

    if (msgrcv(msgid, &U, sizeof(U)-sizeof(long), DEL_GR_REQ, IPC_NOWAIT) != -1){
        printf("Przyjalem zadanie.\n");
        for(i=0; i<Groups[U.state].quan; i++){
            if(!strcmp(Groups[U.state].logins[Groups[U.state].quan-1], U.login)){
                strcpy(Groups[U.state].logins[Groups[U.state].quan-1], "\0");
                printf("%d\n", Groups[U.state].quan);
                Groups[U.state].quan--;
                F.answer = 1;
                processing = 1;
                break;
            }
            if(!strcmp(Groups[U.state].logins[i], U.login)){
                Groups[U.state].quan--;
                strcpy(Groups[U.state].logins[i], Groups[U.state].logins[Groups[U.state].quan]);
                strcpy(Groups[U.state].logins[Groups[U.state].quan], "\0");
                F.answer = 1;
                processing = 1;
                break;
            }
        }
        if(!processing){
            F.answer = 0;
        }
        F.type = DEL_GR_RES;
        msgsnd(msgid, &F, sizeof(F)-sizeof(long), 0);
    }
    return;
}

void message_respond(user Users[]){
    messenger M;
    info About_message;
    info F;
    F.type = MES_USER_RES;
    int i, temp, processing = 0;

    int msgid, msgid2;
    msgid = msgget(req3, 0600);

    if (msgrcv(msgid, &M, sizeof(M)-sizeof(long), MES_USER_REQ, IPC_NOWAIT) != -1){
        printf("Przyjalem zadanie.\n");
        for(i=0; i<9; i++){
            if(!strcmp(Users[i].login, M.to)){
                temp = res;
                temp = temp + Users[i].key;
                processing = 1;
                break;
            }
        }
        if (!processing) {
            F.answer = 0;
        }
        else {
            F.answer = 1;
            About_message.type = INFO_ABOUT_MESS;
            About_message.answer = 1;
            M.group = -1;
            msgid2 = msgget(temp, IPC_CREAT|0600);
            if (msgsnd(msgid2, &About_message, sizeof(About_message)-sizeof(long), 0) == -1){
                perror("Problem z wyslaniem powiadomienia o nadchodzacej wiadomosci");
                exit(1);
            }
            if (msgsnd(msgid2, &M, sizeof(M)-sizeof(long), 0) == -1){
                perror("Problem z wyslaniem wiadomosci wlasciwej");
                exit(1);
            }
        }
        msgsnd(msgid, &F, sizeof(F)-sizeof(long), 0);
    }
    return;
}

void message_to_group_respond(group Groups[]){
    messenger Group_message;
    info About_message;
    info F;
    F.type = MES_GR_RES;
    About_message.type = INFO_ABOUT_MESS;
    About_message.answer = 1;
    int i, temp, processing = 0;

    int msgid, msgid2;
    msgid = msgget(req3, 0600);

    if (msgrcv(msgid, &Group_message, sizeof(Group_message)-sizeof(long), MES_GR_REQ, IPC_NOWAIT) != -1){
        printf("Przyjalem zadanie.\n");
        for(i=0; i<Groups[Group_message.group].quan; i++){
            if (strcmp(Group_message.from, Groups[Group_message.group].logins[i]) != 0){
                temp = res;
                temp = temp + Groups[Group_message.group].keys[i];
                processing = 1;
                msgid2 = msgget(temp, IPC_CREAT|0600);
                if (msgsnd(msgid2, &About_message, sizeof(About_message)-sizeof(long), 0) == -1){
                    perror("Problem z wyslaniem powiadomienia o nadchodzacej wiadomosci");
                    exit(1);
                }
                if (msgsnd(msgid2, &Group_message, sizeof(Group_message)-sizeof(long), 0) == -1){
                    perror("Problem z wyslaniem wiadomosci do jednego uzytkownika z grupy");
                    exit(1);
                }
            }
        }
        if (!processing) {
            F.answer = 0;
        }
        else {
            F.answer = 1;
        }
        msgsnd(msgid, &F, sizeof(F)-sizeof(long), 0);
    }
    return;
}

void clear(){
    int temp, i;
    int msgid1 = msgget(req1, IPC_CREAT|0600);
    int msgid2 = msgget(req2, IPC_CREAT|0600);
    int msgid3 = msgget(req3, IPC_CREAT|0600);
    msgctl(msgid1, IPC_RMID, NULL);
    msgctl(msgid2, IPC_RMID, NULL);
    msgctl(msgid3, IPC_RMID, NULL);
    for(i = 30; i<39; i++){
        temp = res;
        temp = temp + i;
        int msgid = msgget(temp, IPC_CREAT|0600);
        msgctl(msgid, IPC_RMID, NULL);
    }
    return;
}


int main(){
    int i, j, k, g = 0, quantity = 0;
    char buf[20], person[20] = "";

    user Users[9];
    group Groups[3];

    FILE *d = fopen("dane.txt", "r");

    // wczytywanie uzytkownikow
    for(j=0; j<9; j++){
        fscanf(d, "%s", Users[j].login);
        fscanf(d, "%s", Users[j].password);
        fscanf(d, "%d", &Users[j].key);
        Users[j].state = 0;
        Users[j].my_groups[0] = 0;
        Users[j].my_groups[1] = 0;
        Users[j].my_groups[2] = 0;
    }

    // wczytywanie grup
    for(j=0; j<3; j++){
        fscanf(d, "%s", buf);
        for(i=0; i<sizeof(buf); i++){
            if(buf[i] == '\0'){
                Groups[j].keys[g] = atoi(person);
                for(k=0; k<9; k++)
                    if(Groups[j].keys[g] == Users[k].key){
                        strcpy(Groups[j].logins[g], Users[k].login);
                        Users[k].my_groups[j] = 1;
                        quantity++;
                        break;
                    }
                i++;
                break; }
            if(buf[i] == ','){
                Groups[j].keys[g] = atoi(person);
                for(k=0; k<9; k++){
                    if(Groups[j].keys[g] == Users[k].key){
                        strcpy(Groups[j].logins[g], Users[k].login);
                        Users[k].my_groups[j] = 1;
                        quantity++;
                        break;
                    }
                }
                g++; i++;
                strcpy(person, "\0");
            }
            strncat(person,&buf[i],1);
        }
        g = 0;
        strcpy(person, "\0");
        Groups[j].quan = quantity;
        quantity = 0;
    }

    fclose(d);
    clear();

    while (1) {
    login_respond(Users);
    show_logged_respond(Users);
    show_group_respond(Groups);
    add_to_group_respond(Groups, Users);
    delete_from_group_respond(Groups, Users);
    message_respond(Users);
    message_to_group_respond(Groups);
    logout_respond(Users);
    }
    return 0;
}
