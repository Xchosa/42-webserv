#!/bin/bash

cat > urls.txt <<'EOF'
http://127.0.0.1:8081/
http://127.0.0.1:8082/
http://127.0.0.1:8084/
http://127.0.0.1:8085/
http://127.0.0.1:8087/
EOF



siege -b -c50 -t30S -f urls.txt 2>&1 | tee siege-results.txt
