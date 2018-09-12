# CSCI415-assign2

TO COMPILE
gcc -pthread -o proxy proxy.c

TESTING

Run ./proxy <port number>

On same host that server is running on

export http_proxy=http://localhost:<port number>

curl http://ethangellerman.me/
