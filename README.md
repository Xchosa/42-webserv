# Webserv

Writing your own HTTP server



Testing

port already in use 

lsof -i :8081

kill -9 `<PID>	(-9 for sigkill) or fuser -k 8081/tcp`

more errors: ss as more reliable tool as netstat:  ss -tulpn | grep :8081
