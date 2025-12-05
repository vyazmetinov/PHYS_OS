### wshs_shm.c (Shared Memory)
```bash
# Компиляция
gcc -o wshs_s wshs_shm.c

# Запуск (в разных терминалах)
./wshs_s wash   # Мойщик
./wshs_s wipe   # Вытиральщик
```

### wshs_pipe.c (Pipes)
```bash
# Компиляция
gcc -o wshs_p wshs_pipe.c

# Запуск (процессы fork'аются внутри программы)
./wshs_p
```

### wshs_sem.c (Semaphores + Files)
```bash
# Компиляция
gcc -o wshs_sem wshs_sem.c

# Предварительно нужно очистить файл qwipe.txt
echo "" > qwipe.txt

# Запуск (в разных терминалах, сначала wipe!)
./wshs_sem wipe   # Вытиральщик (запустить первым)
./wshs_sem wash   # Мойщик
```

### wshs_sk.c (Sockets)
```bash
# Компиляция
gcc -o wshs_sk wshs_sk.c

# Удалить старый сокет если остался
rm -f /tmp/wshs_sk.sock

# Запуск (в разных терминалах, сначала wipe!)
./wshs_sk wipe   # Вытиральщик (запустить первым - сервер)
./wshs_sk wash   # Мойщик (клиент)
```

### wshs_msg.c (Message Queues)
```bash
# Компиляция
gcc -o wshs_m wshs_msg.c

# Запуск (в разных терминалах, сначала wash!)
./wshs_m wash   # Мойщик (запустить первым)
./wshs_m wipe   # Вытиральщик
```

### runsim.c (Параллельный запуск)
```bash
# Компиляция
gcc -o runsim runsim.c

# Запуск (например, ограничение в 3 параллельных процесса)
./runsim 3
# После запуска вводить команды построчно
```