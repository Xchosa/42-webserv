#pragma once

#include <string>

struct CgiSession
{
	pid_t		_pid;
	int			_stdin_fd;				// in_pipe[1]  -> parent schreibt hier body rein wenn vorhanden
	int			_stdout_fd;				// out_pipe[0] -> parent liest output von cgi hier
	std::string	_body;					// request._body, was noch gesendet werden muss an cgi
	size_t		_body_sent = 0;
	std::string	_output;				// cgi output aus out_pipe[0], concatinated
	bool		_stdout_eof = false;
	bool		_waited = false;		// true wenn child mit waitpid() abgewarted wurde, damit waitpid nur einmal aufgerufen wird
	int			_exit_status = 0;
	time_t		_started;
};
