#include "mirroraccel.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    int i = 0;
    mirror_accel_init();
    int port = mirror_accel_create("0.0.0.0:0");
    if (port > 0) {
        printf("listening on port: %d \npress any key to shutdown this service\n", port);
        getch();
        mirror_accel_destroy(port);
    }
    mirror_accel_uninit();
    return 0;
}