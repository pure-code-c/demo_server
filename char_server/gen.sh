g++ ./chat_server.cc ../src/*.cc ../src/http/*.cc \
-o ./chat_server \
-pthread -lmysqlclient -ljsoncpp -p -pipe