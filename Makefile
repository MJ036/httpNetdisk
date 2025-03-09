INCLUDES:=./ ./include
SRCS:=$(wildcard *.cc)
LIBS:= -lwfrest -lworkflow -lssl -lcrypto -lcrypt
OBJS:=$(patsubst %.cc, %.o, $(SRCS))
SERVER:= CloudiskServer


$(SERVER) : main.o CloudiskServer.o
	g++ $^ -o $@ $(LIBS) $(addprofix -I, $(INCLUDES)) -g

%.o : %.cc
	g++ -c $^ -o $@ $(LIBS) $(addprofix -I, $(INCLUDES)) -g

#HASH:=testHash
#TOKEN:=testToken

#$(HASH): Hash.o testHash.o
#	g++ $^ -o $@ $(LIBS) $(addprefix -I, $(INCLUDES)) -g

#$(TOKEN): Token.o testToken.o
#	g++ $^ -o $@ $(LIBS) $(addprefix -I, $(INCLUDES)) -g

#echo:
#	echo $(INCLUDES)
#	echo $(SRCS)

clean:
	rm -rf $(OBJS) $(SERVER) $(HASH) $(TOKEN)
