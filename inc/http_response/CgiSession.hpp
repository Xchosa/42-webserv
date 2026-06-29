#pragma once

#include <string>

struct CgiSession
{
	pid_t		_pid;
	int			_stdin_fd;				// in_pipe[1]
	int			_stdout_fd;				// out_pipe[0]
	std::string	_body;					// request._body, was noch gesendet werden muss an cgi
	std::string	_output;				// cgi output aus out_pipe[0], concatinated
	bool		_stdout_eof = false; // noch benoetigt?
	bool		_waited = false;		// true wenn child mit waitpid() abgewarted wurde, damit waitpid nur einmal aufgerufen wird
	int			_exit_status = 0;
	time_t		_started;
};

// in_pipe (fuer uebertrag von body)
// 0 -> Leseende, wird zu stdin von child
// 1 -> Schreibende, Parent schreibt hier Body rein (wenn vorhanden, ansonsten gleich closen)

// out_pipe (fuer uebertrag output cgi)
// 0 -> Leseende, Parent liest cgi output von child von hier
// 1 -> Schreibende, wird zu stdout von child
