#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_CLIENTS 3
#define PORT 1234
#define BUFFER_LEN 256

//объявление обработчика сигнала
volatile sig_atomic_t was_sighup = 0;

void sighup_handler(int signal) {
	was_sighup = 1;
}

int main(void) {
	printf("Server's PID: %d\n", getpid()); // Вывод PID

	//создание серверного сокета
	int socket_fd; //сокет
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0) { //в случае ошибки возвращает -1
		perror("Couldn't create socket");
		exit(1);
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));//заполняет всё нулями
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//все адреса локального хоста преобразуются в схему, используемую в сети (для 32 бит IP)
	server_addr.sin_port = htons(PORT); //аналогично с портом (для 16 бит port)

	//назначение сокету порта тср
	if (bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in)) != 0) {
		perror("Couldn't bind socket");//
		exit(1);
	}
	listen(socket_fd, MAX_CLIENTS);//переводим сокет в режим ожидания соединения
	printf("Server listen at 0.0.0.0:%d\n", PORT);//выводим сообщение о том, что сервер ожидает подключений

//регистрация обработчика сигнала
	struct sigaction sa;
	sigaction(SIGHUP, NULL, &sa);
	sa.sa_handler = sighup_handler;
	sa.sa_flags |= SA_RESTART; 
	sigaction(SIGHUP, &sa, NULL);

//блокировка сигнала
	sigset_t mask;
	sigset_t sighup_mask;
	sigemptyset(&sighup_mask);
	sigaddset(&sighup_mask, SIGHUP);//связываем маску с сигналом
	sigprocmask(SIG_BLOCK, &sighup_mask, &mask); //добавляем сигнал, который необходимо блокировать когда работает обработчик

	int clients[MAX_CLIENTS] = {0};//номера клиентов
	int clients_num = 0; //количество клиентов

	char data[BUFFER_LEN] = {0};//сообщения от клиентов

	while (1) {
		//основной цикл
		int nfds = socket_fd; //число на единицу большее максимального файлового дескриптора
		
    fd_set fds;//создаём файловый дескриптор
		FD_ZERO(&fds);//очищаем его для создания нового набора
		FD_SET(socket_fd, &fds);//устанавливает бит для файлового дескриптора socket_fd
		for (int i = 0; i < clients_num; i++) {
			FD_SET(clients[i], &fds);//устанавливаем биты для файловых дескрипторов клиентов

			if (clients[i] > nfds) {//если файловый дескриптор больше nfds, передать его в nfds
				nfds = clients[i];
			}
		}

		int res = pselect(nfds + 1, &fds, NULL, NULL, NULL, &mask);
                      //nfds, readfds, writefds, errorfds, timeout, sigmask

		
    // если errno = eintr значит был сигнал 
		if (res == -1 && errno != EINTR) { 
			perror("pselect error");
			exit(1);
		}

		if (was_sighup) { // в случае отключения сервера (hangup)
			printf("Signal received\n");
			was_sighup = 0;
			break;
		}

		if (FD_ISSET(socket_fd, &fds)) { //проверяем, установлен ли бит файлового дескриптора socket_fd
			int conn = accept(socket_fd, NULL, NULL); //ожидание и принятие входящего соединения.
			if (conn < 0) {
				perror("Couldn't accept new connection");
				continue;
			}

			if (clients_num + 1 > MAX_CLIENTS) { //проверка количества подключённых клиентов
				printf("Too many clients\n");
				close(conn);
				continue;
			}

			clients[clients_num] = conn; // когда принимаем подключение клиента 
			clients_num++;
			printf("New client connected: %d\n", conn-MAX_CLIENTS);
		}

		for (int i = 0; i < clients_num; i++) {
			if (FD_ISSET(clients[i], &fds)) { //проверка на установление для клиента файлового дескриптора
				//обработка сообщения клиента
				memset(&data, 0, BUFFER_LEN);

				ssize_t n = read(clients[i], &data, BUFFER_LEN - 1);

				if (n > 0) {
					n--;
					data[n] = '\0';
				}

				if (n <= 0 || !strcmp("q", data)) { //клиент выходит при вводе q
					close(clients[i]);
					printf("Client %d closed connection\n", clients[i]-MAX_CLIENTS);
          
          // Перемещаем последнего клиента на место отключенного клиента
          clients[i] = clients[clients_num - 1];
					clients[clients_num - 1] = 0;

					clients_num--;
					i--;

					continue;
				}

				printf("Client %d sent: %s\n", clients[i]-MAX_CLIENTS, data);
			}
		}
	}

	close(socket_fd);

	return EXIT_SUCCESS;
}
