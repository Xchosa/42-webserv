#pragma once

#include <string>

struct CgiSession
{
	pid_t		_pid;
	int			_stdin_fd;				// in_pipe[1]
	int			_stdout_fd;				// out_pipe[0]
	std::string	_body;					// request._body, needs to be send to cgi
	std::string	_output;				// cgi output from out_pipe[0], concatinated
	bool		_waited = false;		// true if child reaped with waitpid()
	int			_exit_status = 0;
	time_t		_started;
};

// in_pipe (for transfer of body)
// 0 -> read end,  gets stdin of child
// 1 -> write end, parent writes body here (if exist, else directly close this fd and dont register epoll)

// out_pipe (for transfer output cgi)
// 0 -> read end,  parent reads cgi output here
// 1 -> write end, gets stdout of child
