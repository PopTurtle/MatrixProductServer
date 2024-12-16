#include "requests_management.h"

#include <unistd.h>
void manage_request(const request *r) {
    sleep(2);
    printf("Received request from %ld\n", (long) request_pid(r));
}
