SRCS:=$(wildcard *.cc)
LIBS:= -lwfrest -lworkflow -lssl -lcrypt -lalibabacloud-oss-cpp-sdk -lcrypto -lcurl -lpthread -lSimpleAmqpClient -lsrpc -llz4 -lsnappy -lprotobuf
OBJS:=$(patsubst %.cc, %.o, $(SRCS))
CLIENT:= SignupClient
all: $(CLIENT) 


$(CLIENT) : signup.pb.o client.pb_skeleton.o  
	g++ $^ -o $@ $(LIBS) $(addprofix -I, $(INCLUDES)) -g

%.o : %.cc
	g++ -c $^ -o $@ $(LIBS) $(addprofix -I, $(INCLUDES)) -g


clean:
	rm -rf $(OBJS) $(CLIENT) 
