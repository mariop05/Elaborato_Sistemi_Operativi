#pragma once
#ifndef MESSAGE_QUEUE_HH
#define MESSAGE_QUEUE_HH

#include <sys/msg.h>

struct mymsg {
    long mtype; /* Message type */
    char mtext[100]; /* Message body */
};

#endif