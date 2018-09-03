#pragma once
#include "arSocket.h"
#include "arJSON.h"

int __ar_send(SOCKET socket, const arJSON & jsonRoot, int flags = NULL);
int __ar_recv(SOCKET socket, arJSON& jsonRoot, int flags = NULL);



