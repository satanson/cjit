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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>

#ifndef LIBC_MINGW32
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/poll.h>
#endif

#include <libtcc.h>
#include <ketopt.h>

/////////////
// from file.c
extern long  file_size(const char *filename);
extern char* file_load(const char *filename);
extern char *load_stdin();
extern char* dir_load(const char *path);

#ifdef LIBC_MINGW32
extern char *win32_mkdtemp();
// from win-compat.c
extern void win_compat_usleep(unsigned int microseconds);
extern ssize_t win_compat_getline(char **lineptr, size_t *n, FILE *stream);
#else
extern char *posix_mkdtemp();
#endif
// from io.c
extern void _out(const char *fmt, ...);
extern void _err(const char *fmt, ...);
// from repl.c
#ifdef LIBC_MINGW32
extern int cjit_exec_win(TCCState *TCC, const char *ep, int argc, char **argv);
#else
extern int cjit_exec_fork(TCCState *TCC, const char *ep, int argc, char **argv);
#endif
extern int cjit_cli_tty(TCCState *TCC);
#ifdef KILO_SUPPORTED
extern int cjit_cli_kilo(TCCState *TCC);
#endif
/////////////

void handle_error(void *n, const char *m) {
  (void)n;
  _err("%s",m);
}

#define MAX_ARG_STRING 1024
static int parse_value(char *str) {
  int i = 0;
  int value_pos = 0;
  bool equal_found = false;
  while (str[i] != '\0') {
    if (equal_found && str[i] == '=') {
      return -1; // can't include equal twice
    }
    if (str[i] == '=') {
      str[i]=0x0;
      value_pos = i + 1;
      equal_found = true;
      continue;
    }
    if (!isalnum(str[i]) && str[i] != '_') {
      return -1; // Invalid character found
    }
    i++;
    if(i>MAX_ARG_STRING) {
      return -1; // string too long
    }
  }
  if(equal_found)
    return(value_pos);
  else return(0);
}

const char cli_help[] =
  "\n"
  "Synopsis: cjit [options] files(*)\n"
  "  (*) can be any source (.c) or built object (dll, dylib, .so)\n"
  "Options:\n"
  " -h \t print this help\n"
  " -v \t print version information\n"
  " -q \t stay quiet and only print errors and output\n"
  " -D sym\t define a macro symbol or key=value\n"
  " -C \t set compiler flags (default from env var CFLAGS)\n"
  " -I dir\t also search folder 'dir' for header files\n"
  " -l lib\t search the library named 'lib' when linking\n"
  " -L dir\t also search inside folder 'dir' for -l libs\n"
  " -e fun\t entry point function (default 'main')\n"
  " --live\t run interactive editor for live coding\n"
  " --tgen\t create the runtime temporary dir and exit\n";

int main(int argc, char **argv) {
  TCCState *TCC = NULL;
  // const char *progname = "cjit";
  char *tmpdir = NULL;
  const char *default_main = "main";
  char *entry = (char*)default_main;
  bool live_mode = false;
  bool quiet = false;
  int arg_separator = 0;
  int res = 1;
  int i, c;
  TCC = tcc_new();
  if (!TCC) {
    _err("Could not initialize tcc");
    exit(1);
  }
  // get the extra cflags from the CFLAGS env variable
  // they are overridden by explicit command-line options
  if(getenv("CFLAGS")) {
    char *extra_cflags = NULL;
    extra_cflags = getenv("CFLAGS");
    _err("CFLAGS: %s",extra_cflags);
    tcc_set_options(TCC, extra_cflags);
  }
  static ko_longopt_t longopts[] = {
    { "help", ko_no_argument, 100 },
    { "live", ko_no_argument, 301 },
    { "tgen", ko_no_argument, 401 },
    { NULL, 0, 0 }
  };
  ketopt_t opt = KETOPT_INIT;
  while ((c = ketopt(&opt, argc, argv, 1, "qhvD:L:l:C:I:e:", longopts)) >= 0) {
	  if(c == 'q') {
		  quiet = true;
	  }
    if (c == 'v') {
	    _err("CJIT %s by Dyne.org",VERSION);
      // _err("Running version: %s\n",VERSION);
      // version is always shown
#ifdef LIBC_MINGW32
      _err("Built with MINGW32 libc");
#endif
#ifdef LIBC_MUSL
      _err("Built with Musl libc");
#endif
      tcc_delete(TCC);
      exit(0); // print and exit
    } else if (c=='h' || c==100) { // help
      _err(cli_help,VERSION);
      tcc_delete(TCC);
      exit(0); // print and exit
    } else if (c == 'D') { // define
      int _res;
      _res = parse_value(opt.arg);
      if(_res==0) { // -Dsym (no key=value)
        tcc_define_symbol(TCC, opt.arg, NULL);
      } else if(_res>0) { // -Dkey=value
        tcc_define_symbol(TCC, opt.arg, &opt.arg[_res]);
      } else { // invalid char
        _err("Invalid char used in -D define symbol: %s", opt.arg);
        tcc_delete(TCC);
        exit(1);
      }
    } else if (c == 'L') { // library path
	    if(!quiet)_err("lib path: %s",opt.arg);
	    tcc_add_library_path(TCC, opt.arg);
    } else if (c == 'l') { // library link
	    if(!quiet)_err("lib: %s",opt.arg);
	    tcc_add_library(TCC, opt.arg);
    } else if (c == 'C') { // cflags compiler options
	    if(!quiet)_err("cflags: %s",opt.arg);
      tcc_set_options(TCC, opt.arg);
    } else if (c == 'I') { // include paths in cflags
	    if(!quiet)_err("inc: %s",opt.arg);
      tcc_add_include_path(TCC, opt.arg);
    } else if (c == 'e') { // entry point (default main)
	    if(!quiet)_err("entry: %s",opt.arg);
      if(entry!=default_main) free(entry);
      entry = malloc(strlen(opt.arg)+1);
      strcpy(entry,opt.arg);
    } else if (c == 301) { //
#ifdef LIBC_MINGW32
	    _err("Live mode not supported in Windows");
#else
	    if(!quiet)_err("Live mode activated");
	    live_mode = true;
#endif
    } else if (c == 401) { //
#ifndef LIBC_MINGW32
      tmpdir = posix_mkdtemp();
#else
      tmpdir = win32_mkdtemp();
#endif
      if(!quiet)_err("Temporary exec dir: %s",tmpdir);
      tcc_delete(TCC);
      exit(0);
    }
    else if (c == '?') _err("unknown opt: -%c\n", opt.opt? opt.opt : ':');
    else if (c == ':') _err("missing arg: -%c\n", opt.opt? opt.opt : ':');
    else if (c == '-') { // -- separator
      arg_separator = opt.ind+1; break;
    }
  }
  if(!quiet)_err("CJIT %s by Dyne.org",VERSION);

  //////////////////////////////////////
  // initialize the tmpdir for execution
  // from here onwards use goto endgame
  // as the main and only exit
#ifndef LIBC_MINGW32
  tmpdir = posix_mkdtemp();
#else
  tmpdir = win32_mkdtemp();
#endif
  if(!tmpdir) {
    _err("Error creating temp dir: %s",strerror(errno));
    goto endgame;
  }

  // finally set paths
  tcc_add_include_path(TCC, tmpdir);
  tcc_add_library_path(TCC, tmpdir);
  // tcc_set_lib_path(TCC,tmpdir); // this overrides all?

  // set output in memory for just in time execution
  tcc_set_output_type(TCC, TCC_OUTPUT_MEMORY);

#if defined(LIBC_MUSL)
  tcc_add_libc_symbols(TCC);
#endif

  if (argc == 0 ) {
    _err("No input file: live mode!");
    live_mode = true;
  }
  if(live_mode) {
    if (!isatty(fileno(stdin))) {
      _err("Live mode only available in terminal (tty not found)");
      goto endgame;
    }
#ifdef KILO_SUPPORTED
    res = cjit_cli_kilo(TCC);
#else
    res = cjit_cli_tty(TCC);
#endif
    goto endgame;
  }

  // number of args at the left hand of arg separator, or all of them
  int left_args = arg_separator? arg_separator: argc;

  char *stdin_code = NULL;
  if(opt.ind >= argc) {
#ifdef LIBC_MINGW32
    _err("No files specified on commandline");
    goto endgame;
#endif
    if(!quiet)_err("No files specified on commandline, reading code from stdin");
    stdin_code = load_stdin(); // allocated returned buffer, needs free
    if(!stdin_code) {
      _err("Error reading from standard input");
      goto endgame;
    }
    if( tcc_compile_string(TCC,stdin_code) < 0) {
      _err("Code runtime error in stdin");
      free(stdin_code);
      goto endgame;
    }
  } else if(opt.ind < left_args) {
    // process files on commandline before separator
	  if(!quiet)_err("Source code:");
    for (i = opt.ind; i < left_args; ++i) {
      const char *code_path = argv[i];
      if(!quiet)_err("%c %s",(*code_path=='-'?'|':'+'),
           (*code_path=='-'?"standard input":code_path));
      if(*code_path=='-') { // stdin explicit
#ifdef LIBC_MINGW32
        _err("Code from standard input not supported on Windows");
        goto endgame;
#endif
        stdin_code = load_stdin(); // allocated returned buffer, needs free
        if(!stdin_code) {
          _err("Error reading from standard input");
        } else if( tcc_compile_string(TCC,stdin_code) < 0) {
          _err("Code runtime error in stdin");
          free(stdin_code);
          goto endgame;
        } else free(stdin_code);
      } else { // load any file path
        tcc_add_file(TCC, code_path);
      }
    }
  }
  // error handler callback for TCC
  tcc_set_error_func(TCC, stderr, handle_error);

#ifdef LIBC_MINGW32
  // add symbols for windows compatibility
  tcc_add_symbol(TCC, "usleep", &win_compat_usleep);
  tcc_add_symbol(TCC, "getline", &win_compat_getline);
#endif

  // relocate the code (link symbols)
  if (tcc_relocate(TCC) < 0) {
    _err("TCC symbol relocation error (some library missing?)");
    goto endgame;
  }

  // number of args at the left hand of arg separator, or all of them
  int right_args = argc-left_args+1;//arg_separator? argc-arg_separator : 0;
  char **right_argv = &argv[left_args-1];//arg_separator?&argv[arg_separator]:0
#ifndef LIBC_MINGW32
  res = cjit_exec_fork(TCC, entry, right_args, right_argv);
#else
  res = cjit_exec_win(TCC, entry, right_args, right_argv);
#endif

  endgame:
  // free TCC
  if(TCC) tcc_delete(TCC);
  if(entry!=default_main) free(entry);
  exit(res);
}
