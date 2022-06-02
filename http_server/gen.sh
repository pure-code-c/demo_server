g++ ./http_server.cc ../src/*.cc ../src/http/*.cc \
-o ./http_server \
-pthread -lmysqlclient -ljsoncpp -p