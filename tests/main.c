#include "dht.h"
#include <assert.h>
#include <stdio.h>

static void dht_init_test(void);

int main(void) {
    dht_init_test();

    return 0;
}

static void dht_init_test(void) {
    struct Dht dht;

    dht_init(&dht);

    printf("dht_init_test passed\n");
}

