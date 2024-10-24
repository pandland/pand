#include <stdio.h>
#include <pandio.h>

#define n 16

void print_bytes(unsigned char *bytes, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("%02x ", bytes[i]);
    }
    printf("\n");
}

int main() {
    unsigned char buf[n];
    int status = pd_random(buf, 16);

    if (status < 0) {
        printf("error: %s\n", pd_errstr(status));
        return 1;
    }

    print_bytes(buf, n);

    unsigned char *buf2 = malloc(sizeof(unsigned char) * n);
    if ((status = pd_random(buf2, n)) == 0) {
        print_bytes(buf2, n);
    } else {
        printf("error: %s\n", pd_errstr(status));
        return 1;
    }

    return 0;
}
