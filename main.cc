#include <swc_transform.h>
#include <stdio.h>

int main() {
    const unsigned char *ts = (const unsigned char*)("function add(a: number, b: number) { return a + b }");
    signed char *result = transform_sync(ts, 51);
    printf("Result: %s\n", result);
    return 0;
}
