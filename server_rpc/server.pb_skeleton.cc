#include "signup.srpc.h"
#include "workflow/WFFacilities.h"
#include <workflow/Communicator.h>
#include <workflow/MySQLMessage.h>
#include <workflow/MySQLResult.h>
#include <string>
#include "../unixHeader.h"

using namespace srpc;
using std::string;

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo)
{
	wait_group.done();
}

class UserServiceServiceImpl : public UserService::Service
{
public:

	void Signup(ReqSignup *request, RespSignup *response, srpc::RPCContext *ctx) override
	{
		// TODO: fill server logic here
        //这个函数是服务端的被调函数
        //request是函数的参数
        //response是函数的返回值
        //ctx是上下文，ctx->get_series 可以决定回复返回值的时机

            //获取用户名和密码
            string username = request->username();
            string password = request->password();
            //加密密码，将加密后的密码存储在MySQL
            string salt("12345678");
            string encodedPassword(crypt(password.c_str(),salt.c_str()));
            string mysqlurl("mysql://root:11111111@localhost");
            //创建一个mysql任务，向mysql服务器发起请求。在回调函数中，对mysql服务器的响应做处理
            auto mysqlTask = WFTaskFactory::create_mysql_task(mysqlurl,1,
                     [response](WFMySQLTask *mysqltask){
                     //检测任务状态,即workflow是否有问题
                     int state = mysqltask->get_state();
                     int error = mysqltask->get_error();
                     if(state != WFT_STATE_SUCCESS){
                     printf("%s\n",WFGlobal::get_error_string(state,error));
                     return;
                     }
                     //检测SQL语句是否存在语法错误,即server发送的消息是否有问题
                     //如果是语法错误，需要返回提示信息给服务器,用于跟踪问题
                     auto mysqlResp = mysqltask->get_resp();
                     if(mysqlResp->get_packet_type() == MYSQL_PACKET_ERROR){
                     printf("ERROR %d: %s\n", mysqlResp->get_error_code(),
                            mysqlResp->get_error_msg().c_str());
                     response->set_information("Signup Failed");
                     response->set_code(101);
                     return;
                     }
                     //mysql数据库写入是否有问题，比如数据已存在
                     using namespace protocol;
                     MySQLResultCursor cursor(mysqlResp);
                     if(cursor.get_cursor_status() == MYSQL_STATUS_OK){
                         printf("Query OK. %llu row affected.\n",cursor.get_affected_rows());
                     response->set_information("Success");
                     response->set_code(100);
                     }else{
                     response->set_information("Signup Failed");
                     response->set_code(102);
                     }
                     });
            string sql("INSERT INTO cloudisk.tbl_user(user_name, user_pwd) VALUES('");
            sql += username + "','" + password + "')";
            mysqlTask->get_req()->set_query(sql);
            ctx->get_series()->push_back(mysqlTask);
	}
};

int main()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
    //设置要server绑定的端口号
	unsigned short port = 12345;
    //server负责等待客户端的接入
    //有任务时，server会创建一个序列
    //序列中有一个特殊的rpc任务，它的返回值/响应是在序列中的其他任务执行完后回复的
	SRPCServer server;
    //当有客户端接入时，虚函数Signup会被调用
	UserServiceServiceImpl userservice_impl;
	server.add_service(&userservice_impl);

	server.start(port);
	wait_group.wait();
	server.stop();
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
