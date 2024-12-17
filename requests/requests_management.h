#ifndef REQUESTS_MANAGEMENT__H
#define REQUESTS_MANAGEMENT__H

#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#include "matrix.h"
#include "communication.h"

extern void manage_request(const request *r);

#endif