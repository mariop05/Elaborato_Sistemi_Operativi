/// @file fifo.h
/// @brief Contiene la definizioni di variabili e
///         funzioni specifiche per la gestione delle FIFO.

#include <sys/types.h>

#pragma once

int createFifo(char *pathname, mode_t mode);
int openFifo(char *pathname, int flags);
void letturaSuFile(int fd, char *buffer);
void closeFifo(int fd);
void removeFifo(char *pathname);