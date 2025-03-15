#include "signup.srpc.h"
#include "workflow/WFFacilities.h"

using namespace srpc;

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo)
{
	wait_group.done();
}

static void signup_done(RespSignup *response, srpc::RPCContext *context)
{
    std::cout << "code = " << response->code()
        <<" information = " << response->information() << "\n";
}

int main()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	const char *ip = "127.0.0.1";
	unsigned short port = 12345;

    //client 是服务端在客户端的代理
	UserService::SRPCClient client(ip, port);

    //ReqSignup是函数参数
	ReqSignup req;
    req.set_username("test");
    req.set_password("1234");
    //client中的signup是异步的
    client.Signup(&req,signup_done);

	wait_group.wait();
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
