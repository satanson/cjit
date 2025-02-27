/* CJIT https://dyne.org/cjit
 *
 * Copyright (C) 2024 Dyne.org foundation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

// #ifder LIBC_MUSL
// no need for ifdef here
// since this file is compiled in only by musl build

#include <libtcc.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <poll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <time.h>
#include <dirent.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/syscall.h>
#include <termios.h>
#include <sys/uio.h>
#include <sys/shm.h>

void tcc_add_libc_symbols(TCCState *TCC) {
  /* stdio */
  tcc_add_symbol(TCC, "stdin", &stdin);
  tcc_add_symbol(TCC, "stdout", &stdout);
  tcc_add_symbol(TCC, "stderr", &stderr);
  tcc_add_symbol(TCC, "fprintf", &fprintf);
  tcc_add_symbol(TCC, "printf", &printf);
  tcc_add_symbol(TCC, "puts", &puts);
  tcc_add_symbol(TCC, "fputs", &fputs);
  tcc_add_symbol(TCC, "fputc", &fputc);
  tcc_add_symbol(TCC, "putc", &putc);
  tcc_add_symbol(TCC, "putchar", &putchar);
  tcc_add_symbol(TCC, "perror", &perror);
  tcc_add_symbol(TCC, "vfprintf", &vfprintf);
  tcc_add_symbol(TCC, "vprintf", &vprintf);
  tcc_add_symbol(TCC, "vsprintf", &vsprintf);
  tcc_add_symbol(TCC, "vsnprintf", &vsnprintf);
  tcc_add_symbol(TCC, "fscanf", &fscanf);
  tcc_add_symbol(TCC, "scanf", &scanf);
  tcc_add_symbol(TCC, "sscanf", &sscanf);
  tcc_add_symbol(TCC, "fgetc", &fgetc);
  tcc_add_symbol(TCC, "getc", &getc);
  tcc_add_symbol(TCC, "getchar", &getchar);
  tcc_add_symbol(TCC, "ungetc", &ungetc);
  tcc_add_symbol(TCC, "fgets", &fgets);
  tcc_add_symbol(TCC, "strerror", &strerror);

      /* buffered io */
  tcc_add_symbol(TCC, "fopen", &fopen);
  tcc_add_symbol(TCC, "freopen", &freopen);
  tcc_add_symbol(TCC, "fclose", &fclose);
  tcc_add_symbol(TCC, "fflush", &fflush);
  tcc_add_symbol(TCC, "fread", &fread);
  tcc_add_symbol(TCC, "fwrite", &fwrite);
  tcc_add_symbol(TCC, "fseek", &fseek);
  tcc_add_symbol(TCC, "ftell", &ftell);
  tcc_add_symbol(TCC, "rewind", &rewind);
  tcc_add_symbol(TCC, "fgetpos", &fgetpos);
  tcc_add_symbol(TCC, "fsetpos", &fsetpos);
  tcc_add_symbol(TCC, "feof", &feof);
  tcc_add_symbol(TCC, "ferror", &ferror);
  tcc_add_symbol(TCC, "clearerr", &clearerr);

  /* stdlib */

  /* memory */
  tcc_add_symbol(TCC, "malloc", &malloc);
  tcc_add_symbol(TCC, "free", &free);
  tcc_add_symbol(TCC, "realloc", &realloc);
  tcc_add_symbol(TCC, "calloc", &calloc);
  tcc_add_symbol(TCC, "strdup", &strdup); /* Implies malloc */
  tcc_add_symbol(TCC, "getline", &getline); /* Implies malloc */
  tcc_add_symbol(TCC, "getdelim", &getdelim); /* Implies malloc */
  tcc_add_symbol(TCC, "strndup", &strndup); /* Implies malloc */
  tcc_add_symbol(TCC, "asprintf", &asprintf); /* Implies malloc */
  tcc_add_symbol(TCC, "vasprintf", &vasprintf); /* Implies malloc */
  tcc_add_symbol(TCC, "posix_memalign", &posix_memalign); /* Implies malloc */
  tcc_add_symbol(TCC, "aligned_alloc", &aligned_alloc); /* Implies malloc */

  /* process control */
  tcc_add_symbol(TCC, "exit", &exit);
  tcc_add_symbol(TCC, "wait", &wait);
  tcc_add_symbol(TCC, "kill", &kill);
  tcc_add_symbol(TCC, "fork", &fork);
  tcc_add_symbol(TCC, "vfork", &vfork);
  tcc_add_symbol(TCC, "signal", &signal);
  tcc_add_symbol(TCC, "raise", &raise);
  tcc_add_symbol(TCC, "alarm", &alarm);
  tcc_add_symbol(TCC, "pause", &pause);
  tcc_add_symbol(TCC, "sleep", &sleep);
  tcc_add_symbol(TCC, "usleep", &usleep);
  tcc_add_symbol(TCC, "execve", &execve);
  tcc_add_symbol(TCC, "execl", &execl);
  tcc_add_symbol(TCC, "execlp", &execlp);
  tcc_add_symbol(TCC, "execle", &execle);
  tcc_add_symbol(TCC, "execv", &execv);
  tcc_add_symbol(TCC, "execvp", &execvp);
  tcc_add_symbol(TCC, "execvpe", &execvpe);
  tcc_add_symbol(TCC, "system", &system);
  tcc_add_symbol(TCC, "popen", &popen);
  tcc_add_symbol(TCC, "pclose", &pclose);
  tcc_add_symbol(TCC, "waitpid", &waitpid);
  tcc_add_symbol(TCC, "wait4", &wait4);
  tcc_add_symbol(TCC, "abort", &abort);
  tcc_add_symbol(TCC, "atexit", &atexit);
  tcc_add_symbol(TCC, "system", &system);
  tcc_add_symbol(TCC, "getenv", &getenv);
  tcc_add_symbol(TCC, "putenv", &putenv);
  tcc_add_symbol(TCC, "setenv", &setenv);
  tcc_add_symbol(TCC, "unsetenv", &unsetenv);
  tcc_add_symbol(TCC, "clearenv", &clearenv);
  tcc_add_symbol(TCC, "getpid", &getpid);
  tcc_add_symbol(TCC, "getppid", &getppid);
  tcc_add_symbol(TCC, "getuid", &getuid);
  tcc_add_symbol(TCC, "geteuid", &geteuid);
  tcc_add_symbol(TCC, "getgid", &getgid);
  tcc_add_symbol(TCC, "getegid", &getegid);
  tcc_add_symbol(TCC, "setuid", &setuid);
  tcc_add_symbol(TCC, "seteuid", &seteuid);
  tcc_add_symbol(TCC, "setgid", &setgid);
  tcc_add_symbol(TCC, "setegid", &setegid);
  tcc_add_symbol(TCC, "getgroups", &getgroups);
  tcc_add_symbol(TCC, "setgroups", &setgroups);
  tcc_add_symbol(TCC, "getpgid", &getpgid);
  tcc_add_symbol(TCC, "setpgid", &setpgid);
  tcc_add_symbol(TCC, "setsid", &setsid);
  tcc_add_symbol(TCC, "getsid", &getsid);
  tcc_add_symbol(TCC, "getpgrp", &getpgrp);
  tcc_add_symbol(TCC, "setpgrp", &setpgrp);

  /* numeric conversions */
  tcc_add_symbol(TCC, "atoi", &atoi);
  tcc_add_symbol(TCC, "atol", &atol);
  tcc_add_symbol(TCC, "atoll", &atoll);
  tcc_add_symbol(TCC, "atof", &atof);
  tcc_add_symbol(TCC, "strtol", &strtol);
  tcc_add_symbol(TCC, "strtoll", &strtoll);
  tcc_add_symbol(TCC, "strtoul", &strtoul);
  tcc_add_symbol(TCC, "strtoull", &strtoull);
  tcc_add_symbol(TCC, "strtof", &strtof);
  tcc_add_symbol(TCC, "strtod", &strtod);
  tcc_add_symbol(TCC, "strtof", &strtof);
  tcc_add_symbol(TCC, "strtoimax", &strtoimax);
  tcc_add_symbol(TCC, "strtoumax", &strtoumax);

  /* string */
  tcc_add_symbol(TCC, "memcpy", &memcpy);
  tcc_add_symbol(TCC, "memmove", &memmove);
  tcc_add_symbol(TCC, "memset", &memset);
  tcc_add_symbol(TCC, "memcmp", &memcmp);
  tcc_add_symbol(TCC, "strlen", &strlen);
  tcc_add_symbol(TCC, "strcpy", &strcpy);
  tcc_add_symbol(TCC, "strncpy", &strncpy);
  tcc_add_symbol(TCC, "strcat", &strcat);
  tcc_add_symbol(TCC, "strncat", &strncat);
  tcc_add_symbol(TCC, "strcmp", &strcmp);
  tcc_add_symbol(TCC, "strncmp", &strncmp);
  tcc_add_symbol(TCC, "strcasecmp", &strcasecmp);
  tcc_add_symbol(TCC, "strncasecmp", &strncasecmp);
  tcc_add_symbol(TCC, "strstr", &strstr);
  tcc_add_symbol(TCC, "strchr", &strchr);
  tcc_add_symbol(TCC, "strrchr", &strrchr);
  tcc_add_symbol(TCC, "strspn", &strspn);
  tcc_add_symbol(TCC, "strcspn", &strcspn);
  tcc_add_symbol(TCC, "strpbrk", &strpbrk);
  tcc_add_symbol(TCC, "strtok", &strtok);
  tcc_add_symbol(TCC, "strtok_r", &strtok_r);
  tcc_add_symbol(TCC, "strerror", &strerror);
  tcc_add_symbol(TCC, "strndup", &strndup);
  tcc_add_symbol(TCC, "strerror", &strerror);
  tcc_add_symbol(TCC, "strerror_r", &strerror_r);
  tcc_add_symbol(TCC, "strsignal", &strsignal);
  tcc_add_symbol(TCC, "strcoll", &strcoll);
  tcc_add_symbol(TCC, "strxfrm", &strxfrm);
  tcc_add_symbol(TCC, "strsignal", &strsignal);
  tcc_add_symbol(TCC, "strtok", &strtok);
  tcc_add_symbol(TCC, "strtok_r", &strtok_r);
  tcc_add_symbol(TCC, "strxfrm", &strxfrm);

  /* time */
  tcc_add_symbol(TCC, "time", &time);
  tcc_add_symbol(TCC, "clock", &clock);
  tcc_add_symbol(TCC, "difftime", &difftime);
  tcc_add_symbol(TCC, "mktime", &mktime);
  tcc_add_symbol(TCC, "asctime", &asctime);
  tcc_add_symbol(TCC, "ctime", &ctime);
  tcc_add_symbol(TCC, "gmtime", &gmtime);
  tcc_add_symbol(TCC, "localtime", &localtime);
  tcc_add_symbol(TCC, "strftime", &strftime);
  tcc_add_symbol(TCC, "gettimeofday", &gettimeofday);
  tcc_add_symbol(TCC, "settimeofday", &settimeofday);
  tcc_add_symbol(TCC, "tzset", &tzset);
  tcc_add_symbol(TCC, "tzname", &tzname);
  tcc_add_symbol(TCC, "daylight", &daylight);
  tcc_add_symbol(TCC, "timezone", &timezone);
  tcc_add_symbol(TCC, "nanosleep", &nanosleep);
  tcc_add_symbol(TCC, "clock_gettime", &clock_gettime);
  tcc_add_symbol(TCC, "clock_settime", &clock_settime);
  tcc_add_symbol(TCC, "clock_getres", &clock_getres);
  tcc_add_symbol(TCC, "timer_create", &timer_create);
  tcc_add_symbol(TCC, "timer_delete", &timer_delete);
  tcc_add_symbol(TCC, "timer_settime", &timer_settime);
  tcc_add_symbol(TCC, "timer_gettime", &timer_gettime);
  tcc_add_symbol(TCC, "timer_getoverrun", &timer_getoverrun);
  tcc_add_symbol(TCC, "clock_nanosleep", &clock_nanosleep);
  tcc_add_symbol(TCC, "clock_getcpuclockid", &clock_getcpuclockid);
  tcc_add_symbol(TCC, "clock_settime", &clock_settime);
  tcc_add_symbol(TCC, "clock_getres", &clock_getres);
  tcc_add_symbol(TCC, "strptime", &strptime);
  tcc_add_symbol(TCC, "timegm", &timegm);

  /* file control */
  tcc_add_symbol(TCC, "open", &open);
  tcc_add_symbol(TCC, "openat", &openat);
  tcc_add_symbol(TCC, "creat", &creat);
  tcc_add_symbol(TCC, "close", &close);
  tcc_add_symbol(TCC, "read", &read);
  tcc_add_symbol(TCC, "write", &write);
  tcc_add_symbol(TCC, "pread", &pread);
  tcc_add_symbol(TCC, "pwrite", &pwrite);
  tcc_add_symbol(TCC, "lseek", &lseek);
  tcc_add_symbol(TCC, "dup", &dup);
  tcc_add_symbol(TCC, "dup2", &dup2);
  tcc_add_symbol(TCC, "pipe", &pipe);
  tcc_add_symbol(TCC, "pipe2", &pipe2);
  tcc_add_symbol(TCC, "fcntl", &fcntl);
  tcc_add_symbol(TCC, "ioctl", &ioctl);
  tcc_add_symbol(TCC, "fsync", &fsync);
  tcc_add_symbol(TCC, "fdatasync", &fdatasync);
  tcc_add_symbol(TCC, "ftruncate", &ftruncate);
  tcc_add_symbol(TCC, "sync", &sync);
  tcc_add_symbol(TCC, "fchdir", &fchdir);
  tcc_add_symbol(TCC, "chdir", &chdir);
  tcc_add_symbol(TCC, "getcwd", &getcwd);
  tcc_add_symbol(TCC, "opendir", &opendir);
  tcc_add_symbol(TCC, "fdopendir", &fdopendir);
  tcc_add_symbol(TCC, "readdir", &readdir);
  tcc_add_symbol(TCC, "readdir_r", &readdir_r);
  tcc_add_symbol(TCC, "rewinddir", &rewinddir);
  tcc_add_symbol(TCC, "closedir", &closedir);
  tcc_add_symbol(TCC, "dirfd", &dirfd);
  tcc_add_symbol(TCC, "telldir", &telldir);
  tcc_add_symbol(TCC, "seekdir", &seekdir);
  tcc_add_symbol(TCC, "scandir", &scandir);
  tcc_add_symbol(TCC, "alphasort", &alphasort);

  /* polling */
  tcc_add_symbol(TCC, "poll", &poll);
  tcc_add_symbol(TCC, "select", &select);
  tcc_add_symbol(TCC, "pselect", &pselect);
  tcc_add_symbol(TCC, "epoll_create", &epoll_create);
  tcc_add_symbol(TCC, "epoll_create1", &epoll_create1);
  tcc_add_symbol(TCC, "epoll_ctl", &epoll_ctl);
  tcc_add_symbol(TCC, "epoll_wait", &epoll_wait);
  tcc_add_symbol(TCC, "epoll_pwait", &epoll_pwait);

  /* socket */
  tcc_add_symbol(TCC, "socket", &socket);
  tcc_add_symbol(TCC, "socketpair", &socketpair);
  tcc_add_symbol(TCC, "bind", &bind);
  tcc_add_symbol(TCC, "listen", &listen);
  tcc_add_symbol(TCC, "accept", &accept);
  tcc_add_symbol(TCC, "accept4", &accept4);
  tcc_add_symbol(TCC, "connect", &connect);
  tcc_add_symbol(TCC, "shutdown", &shutdown);
  tcc_add_symbol(TCC, "send", &send);
  tcc_add_symbol(TCC, "sendto", &sendto);
  tcc_add_symbol(TCC, "sendmsg", &sendmsg);
  tcc_add_symbol(TCC, "recv", &recv);
  tcc_add_symbol(TCC, "recvfrom", &recvfrom);
  tcc_add_symbol(TCC, "recvmsg", &recvmsg);
  tcc_add_symbol(TCC, "getsockname", &getsockname);
  tcc_add_symbol(TCC, "getpeername", &getpeername);
  tcc_add_symbol(TCC, "getsockopt", &getsockopt);
  tcc_add_symbol(TCC, "setsockopt", &setsockopt);
  tcc_add_symbol(TCC, "getaddrinfo", &getaddrinfo);
  tcc_add_symbol(TCC, "freeaddrinfo", &freeaddrinfo);
  tcc_add_symbol(TCC, "gai_strerror", &gai_strerror);
  tcc_add_symbol(TCC, "inet_aton", &inet_aton);
  tcc_add_symbol(TCC, "inet_addr", &inet_addr);
  tcc_add_symbol(TCC, "inet_ntoa", &inet_ntoa);
  tcc_add_symbol(TCC, "inet_pton", &inet_pton);
  tcc_add_symbol(TCC, "inet_ntop", &inet_ntop);
  tcc_add_symbol(TCC, "gethostbyname", &gethostbyname);
  tcc_add_symbol(TCC, "gethostbyaddr", &gethostbyaddr);
  tcc_add_symbol(TCC, "ntohs", &ntohs);
  tcc_add_symbol(TCC, "ntohl", &ntohl);
  tcc_add_symbol(TCC, "htons", &htons);
  tcc_add_symbol(TCC, "htonl", &htonl);

  /* mmap and shared memory */
  tcc_add_symbol(TCC, "mmap", &mmap);
  tcc_add_symbol(TCC, "munmap", &munmap);
  tcc_add_symbol(TCC, "mprotect", &mprotect);
  tcc_add_symbol(TCC, "msync", &msync);
  tcc_add_symbol(TCC, "mlock", &mlock);
  tcc_add_symbol(TCC, "munlock", &munlock);
  tcc_add_symbol(TCC, "mlockall", &mlockall);
  tcc_add_symbol(TCC, "munlockall", &munlockall);
  tcc_add_symbol(TCC, "madvise", &madvise);
  tcc_add_symbol(TCC, "mincore", &mincore);
  tcc_add_symbol(TCC, "shm_open", &shm_open);
  tcc_add_symbol(TCC, "shm_unlink", &shm_unlink);
  tcc_add_symbol(TCC, "shmget", &shmget);
  tcc_add_symbol(TCC, "shmat", &shmat);
  tcc_add_symbol(TCC, "shmdt", &shmdt);
  tcc_add_symbol(TCC, "shmctl", &shmctl);

  /* tty */
  tcc_add_symbol(TCC, "isatty", &isatty);
  tcc_add_symbol(TCC, "ttyname", &ttyname);
  tcc_add_symbol(TCC, "ttyname_r", &ttyname_r);
  tcc_add_symbol(TCC, "tcgetattr", &tcgetattr);
  tcc_add_symbol(TCC, "tcsetattr", &tcsetattr);
  tcc_add_symbol(TCC, "tcsendbreak", &tcsendbreak);
  tcc_add_symbol(TCC, "tcdrain", &tcdrain);
  tcc_add_symbol(TCC, "tcflush", &tcflush);
  tcc_add_symbol(TCC, "tcflow", &tcflow);
  tcc_add_symbol(TCC, "tcgetsid", &tcgetsid);
  tcc_add_symbol(TCC, "cfgetospeed", &cfgetospeed);
  tcc_add_symbol(TCC, "cfgetispeed", &cfgetispeed);
  tcc_add_symbol(TCC, "cfsetospeed", &cfsetospeed);
  tcc_add_symbol(TCC, "cfsetispeed", &cfsetispeed);
  tcc_add_symbol(TCC, "cfsetspeed", &cfsetspeed);

  /* semaphores */
  tcc_add_symbol(TCC, "sem_init", &sem_init);
  tcc_add_symbol(TCC, "sem_destroy", &sem_destroy);
  tcc_add_symbol(TCC, "sem_open", &sem_open);
  tcc_add_symbol(TCC, "sem_close", &sem_close);
  tcc_add_symbol(TCC, "sem_unlink", &sem_unlink);
  tcc_add_symbol(TCC, "sem_wait", &sem_wait);
  tcc_add_symbol(TCC, "sem_trywait", &sem_trywait);
  tcc_add_symbol(TCC, "sem_post", &sem_post);
  tcc_add_symbol(TCC, "sem_getvalue", &sem_getvalue);
  tcc_add_symbol(TCC, "sem_timedwait", &sem_timedwait);

  /* posix threads */
  tcc_add_symbol(TCC, "pthread_create", &pthread_create);
  tcc_add_symbol(TCC, "pthread_exit", &pthread_exit);
  tcc_add_symbol(TCC, "pthread_join", &pthread_join);
  tcc_add_symbol(TCC, "pthread_detach", &pthread_detach);
  tcc_add_symbol(TCC, "pthread_self", &pthread_self);
  tcc_add_symbol(TCC, "pthread_equal", &pthread_equal);
  tcc_add_symbol(TCC, "pthread_cancel", &pthread_cancel);
  tcc_add_symbol(TCC, "pthread_setcancelstate", &pthread_setcancelstate);
  tcc_add_symbol(TCC, "pthread_setcanceltype", &pthread_setcanceltype);
  tcc_add_symbol(TCC, "pthread_testcancel", &pthread_testcancel);
  tcc_add_symbol(TCC, "pthread_kill", &pthread_kill);
  tcc_add_symbol(TCC, "pthread_sigmask", &pthread_sigmask);
  tcc_add_symbol(TCC, "pthread_attr_init", &pthread_attr_init);
  tcc_add_symbol(TCC, "pthread_attr_destroy", &pthread_attr_destroy);
  tcc_add_symbol(TCC, "pthread_attr_getdetachstate", &pthread_attr_getdetachstate);
  tcc_add_symbol(TCC, "pthread_attr_setdetachstate", &pthread_attr_setdetachstate);
  tcc_add_symbol(TCC, "pthread_attr_getstacksize", &pthread_attr_getstacksize);
  tcc_add_symbol(TCC, "pthread_attr_setstacksize", &pthread_attr_setstacksize);
  tcc_add_symbol(TCC, "pthread_attr_getstack", &pthread_attr_getstack);
  tcc_add_symbol(TCC, "pthread_attr_setstack", &pthread_attr_setstack);
  tcc_add_symbol(TCC, "pthread_attr_getguardsize", &pthread_attr_getguardsize);
  tcc_add_symbol(TCC, "pthread_attr_setguardsize", &pthread_attr_setguardsize);
  tcc_add_symbol(TCC, "pthread_attr_getschedparam", &pthread_attr_getschedparam);
  tcc_add_symbol(TCC, "pthread_attr_setschedparam", &pthread_attr_setschedparam);
  tcc_add_symbol(TCC, "pthread_attr_getschedpolicy", &pthread_attr_getschedpolicy);
  tcc_add_symbol(TCC, "pthread_attr_setschedpolicy", &pthread_attr_setschedpolicy);
  tcc_add_symbol(TCC, "pthread_attr_getinheritsched", &pthread_attr_getinheritsched);
  tcc_add_symbol(TCC, "pthread_attr_setinheritsched", &pthread_attr_setinheritsched);
  tcc_add_symbol(TCC, "pthread_attr_getscope", &pthread_attr_getscope);
  tcc_add_symbol(TCC, "pthread_attr_setscope", &pthread_attr_setscope);
  tcc_add_symbol(TCC, "pthread_setschedparam", &pthread_setschedparam);
  tcc_add_symbol(TCC, "pthread_getschedparam", &pthread_getschedparam);
  tcc_add_symbol(TCC, "pthread_mutex_init", &pthread_mutex_init);
  tcc_add_symbol(TCC, "pthread_mutex_destroy", &pthread_mutex_destroy);
  tcc_add_symbol(TCC, "pthread_mutex_lock", &pthread_mutex_lock);
  tcc_add_symbol(TCC, "pthread_mutex_trylock", &pthread_mutex_trylock);
  tcc_add_symbol(TCC, "pthread_mutex_unlock", &pthread_mutex_unlock);
  tcc_add_symbol(TCC, "pthread_mutexattr_init", &pthread_mutexattr_init);
  tcc_add_symbol(TCC, "pthread_mutexattr_destroy", &pthread_mutexattr_destroy);
  tcc_add_symbol(TCC, "pthread_mutexattr_gettype", &pthread_mutexattr_gettype);
  tcc_add_symbol(TCC, "pthread_mutexattr_settype", &pthread_mutexattr_settype);
  tcc_add_symbol(TCC, "pthread_mutexattr_getpshared", &pthread_mutexattr_getpshared);
  tcc_add_symbol(TCC, "pthread_mutexattr_setpshared", &pthread_mutexattr_setpshared);
  tcc_add_symbol(TCC, "pthread_mutexattr_getprotocol", &pthread_mutexattr_getprotocol);
  tcc_add_symbol(TCC, "pthread_mutexattr_setprotocol", &pthread_mutexattr_setprotocol);
  tcc_add_symbol(TCC, "pthread_mutexattr_getrobust", &pthread_mutexattr_getrobust);
  tcc_add_symbol(TCC, "pthread_mutexattr_setrobust", &pthread_mutexattr_setrobust);
  tcc_add_symbol(TCC, "pthread_mutex_consistent", &pthread_mutex_consistent);
}
