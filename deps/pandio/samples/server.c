#include <stdio.h>
#include <pandio.h>
#include <string.h>

int counter = 1;

void handle_write(pd_write_t *write_op, int status) {
    //printf("Written successfully with status: %d\n", status);
    free(write_op->data.buf);
    free(write_op);
}

void handle_read(pd_tcp_t *client, char *buf, size_t len) {
    if ((int)len < 0) {
        printf("Status: %d\n", (int)len);
        printf("Error: %s\n", pd_errname((int)len));
        printf("Message: %s\n", pd_errstr((int)len));
        pd_tcp_close(client);
        return;
    }

    printf("%.*s\n", (int)len, buf);
    free(buf);
    const char *response = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: 5\r\n\r\nHello";
    pd_write_t *write_op = malloc(sizeof(pd_write_t));
    pd_write_init(write_op, strdup(response), strlen(response), handle_write);

    pd_tcp_write(client, write_op);
    pd_tcp_shutdown(client);
    //pd_threadpool_end();
}

void handle_close(pd_tcp_t *client) {
    printf("Connection closed\n");
    free(client);
}

void handle_connection(pd_tcp_server_t *server, pd_socket_t socket, int status) {
    pd_tcp_t *client = malloc(sizeof(pd_tcp_t));
    pd_tcp_init(server->ctx, client);
    client->on_data = handle_read;
    client->on_close = handle_close;

    pd_tcp_accept(client, socket);
    //int stat = pd_tcp_keepalive(client, true, 1);
    //printf("Keep alive result: %d\n", stat);
    //printf("Received connection #%d\n", counter++);
}

void expensive_task(pd_task_t *task) {
    printf("Job finished\n");
}

void task_done(pd_task_t *task) {
    printf("Task done and this is a main thread\n");
    free(task);
}

void interval_task(pd_timer_t *timer) {
    printf("Timer callback\n");
}

void close_server(pd_timer_t *timer) {
    printf("Closing server...\n");
    pd_tcp_server_t *server = timer->data;
    pd_tcp_server_close(server);
    free(server);
}

int main() {
    pd_io_t *ctx = malloc(sizeof(pd_io_t));
    pd_io_init(ctx);

    pd_tcp_server_t *server = malloc(sizeof(pd_tcp_server_t));
    pd_tcp_server_init(ctx, server);
    //pd_threadpool_end();
    printf("Starting to listen...\n");
    int status = pd_tcp_listen(server, 5000, handle_connection);
    if (status < 0) {
        printf("Listener failed.\n");
    }

    pd_timer_t timer;
    pd_timer_init(ctx, &timer);
    timer.data = server;
    pd_timer_start(&timer, close_server, 10 * 1000);

    pd_io_run(ctx);
}
