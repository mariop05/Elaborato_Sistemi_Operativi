cmake_minimum_required(VERSION 3.25)
project(Progetto_Sistemi_Operativi C)

set(CMAKE_C_STANDARD 11)

add_executable(client client.c matrix.c err_exit.c message_queue.c semaphore.c shared_memory.c utils.c fifo.c)
add_executable(server server.c matrix.c err_exit.c message_queue.c semaphore.c shared_memory.c utils.c fifo.c)