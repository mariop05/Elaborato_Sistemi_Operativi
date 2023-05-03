/// @file fifo.h
/// @brief Contiene la definizioni di variabili e
///         funzioni specifiche per la gestione delle FIFO.
#pragma once
#ifndef FIFO_HH
#define FIFO_HH

#include <sys/types.h>

int createFifo(char *pathname, mode_t mode);
int openFifo(char *pathname, int flags);
void letturaSuFile(int fd, char *buffer);
void closeFifo(int fd);
void removeFifo(char *pathname);

#endif