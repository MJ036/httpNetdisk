INCLUDES:=./ ./include
SRCS:=$(wildcard *.cc)
LIBS:= -lwfrest -lworkflow -lssl -lcrypt -lalibabacloud-oss-cpp-sdk -lcrypto -lcurl -lpthread -lSimpleAmqpClient -lsrpc -llz4 -lsnappy -lprotobuf
OBJS:=$(patsubst %.cc, %.o, $(SRCS))
SERVER:= CloudiskServer
all: $(SERVER) 


$(SERVER) : $(OBJS)
	g++ $^ -o $@ $(LIBS) $(addprofix -I, $(INCLUDES)) -g

%.o : %.cc
	g++ -c $^ -o $@ $(LIBS) $(addprofix -I, $(INCLUDES)) -g

#HASH:=testHash
#TOKEN:=testToken
#RABBITMQ:=TestRabbitmq

#$(HASH): Hash.o testHash.o
#	g++ $^ -o $@ $(LIBS) $(addprefix -I, $(INCLUDES)) -g

#$(TOKEN): Token.o testToken.o
#	g++ $^ -o $@ $(LIBS) $(addprefix -I, $(INCLUDES)) -g

#$(RABBITMQ): TestRabbitmq.o Rabbitmq.o Oss.o
#	g++ $^ -o $@ $(LIBS) $(addprefix -I, $(INCLUDES)) -g

#echo:
#	echo $(INCLUDES)
#	echo $(SRCS)

clean:
	rm -rf $(OBJS) $(SERVER) $(HASH) $(TOKEN) $(RABBITMQ)
