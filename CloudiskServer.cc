#include "CloudiskServer.h"
#include "Token.h"
#include "Hash.h"
#include "unixHeader.h"

#include <csignal>
#include <nlohmann/detail/meta/type_traits.hpp>
#include <nlohmann/json_fwd.hpp>
#include <wfrest/HttpDef.h>
#include <wfrest/HttpMsg.h>
#include <workflow/MySQLMessage.h>
#include <workflow/MySQLResult.h>
#include <wfrest/Json.h>
#include <nlohmann/json.hpp>

#include <iostream>
#include <workflow/WFGlobal.h>
#include <workflow/WFTask.h>
#include <workflow/WFTaskFactory.h>
#include <workflow/Workflow.h>
#include <workflow/mysql_types.h>

using namespace wfrest;
using std::string;
using std::cout;
using std::endl;
using std::vector;

void CloudiskServer::start(unsigned short port){
    //track可以打印函数执行过程中的信息，即运行的状态
    if(_httpserver.track().start(port) == 0){
        //该方法，可以打印出已添加的url路由
        _httpserver.list_routes();
        _waitGroup.wait();
        _httpserver.stop();
    }else{
        printf("Cloudisk Server start Failed");
    }
}

void CloudiskServer::loadModules(){
    loadStaticResourceModule();
    loadUserRegisterMoudle();
    loadUserLoginMoudle();
//    loadUserInfoModule();
//    loadFileQueryMoudle();
//    loadFileUploadMoudle();
//    loadFileDownloadMoudle();

}
void CloudiskServer::loadStaticResourceModule(){
    //收到的GET请求时，根据不同的URL，做不同的处理-这里的处理是返回文件
    _httpserver.GET("/user/signup",[](const HttpReq *,HttpResp *resp){
                    resp->File("static/view/signup.html");
                    });

    _httpserver.GET("/static/view/signin.html", [](const HttpReq *, HttpResp * resp){
                    resp->File("static/view/signin.html");
                    });

    _httpserver.GET("/static/view/home.html", [](const HttpReq *, HttpResp * resp){
                    resp->File("static/view/home.html");
                    });

    _httpserver.GET("/static/js/auth.js", [](const HttpReq *, HttpResp * resp){
                    resp->File("static/js/auth.js");
                    });

    _httpserver.GET("/static/img/avatar.jpeg", [](const HttpReq *, HttpResp * resp){
                    resp->File("static/img/avatar.jpeg");
                    });

    _httpserver.GET("/file/upload", [](const HttpReq *, HttpResp * resp){
                    resp->File("static/view/index.html");
                    });

    //当url匹配时，将静态文件的目录中的静态文件回复给客户端
    //css样式表等文件
    _httpserver.Static("/file/upload_files","static/view/upload_files");
}
void CloudiskServer::loadUserRegisterMoudle(){
    //接收用户的注册信息
    //执行流：框架收到了POST方法，并且url匹配完成。调用相对应的lambda表达式：
    //          lambda表达式处理的内容：获取用户信息，设置mysql任务，将mysql任务放入序列
    //          执行完mysql任务后，进入mysql任务的回调函数，检查mysql的响应，判断是否出错。
    _httpserver.POST("/user/signup",[](const HttpReq *req, HttpResp *resp,SeriesWork *series){
        if(req->content_type() == APPLICATION_URLENCODED){
            //接受urlencoded类型的请求体中的信息,以map键值对的方式存储
            auto formKV = req->form_kv();
            //获取用户名和密码
            string username = formKV["username"];
            string password = formKV["password"];
            //加密密码，将加密后的密码存储在MySQL
            string salt("12345678");
            string encodedPassword(crypt(password.c_str(),salt.c_str()));
            string mysqlurl("mysql://root:11111111@localhost");
            //创建一个mysql任务，向mysql服务器发起请求。在回调函数中，对mysql服务器的响应做处理
            auto mysqlTask = WFTaskFactory::create_mysql_task(mysqlurl,1,
                     [resp](WFMySQLTask *mysqltask){
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
                     resp->String("Signup Failed");
                     return;
                     }
                     //mysql数据库写入是否有问题，比如数据已存在
                     using namespace protocol;
                     MySQLResultCursor cursor(mysqlResp);
                     if(cursor.get_cursor_status() == MYSQL_STATUS_OK){
                         printf("Query OK. %llu row affected.\n",cursor.get_affected_rows());
                         resp->String("Success");
                     }else{
                         resp->String("Signup Failed");
                     }
                     });
            string sql("INSERT INTO cloudisk.tbl_user(user_name, user_pwd) VALUES('");
            sql += username + "','" + password + "')";
            mysqlTask->get_req()->set_query(sql);
            series->push_back(mysqlTask);
          }
    });

}
void CloudiskServer::loadUserLoginMoudle(){
    _httpserver.POST("/user/signin",[](const HttpReq *req, HttpResp *resp, SeriesWork *series){
         if(req->content_type() == APPLICATION_URLENCODED){
            auto formKV = req->form_kv();
            string username = formKV["username"];
            string password = formKV["password"];
            string salt("12345678");
            string encodedPassword(crypt(password.c_str(),salt.c_str()));
            cout << "mi wen:" << encodedPassword << endl;
            string mysqlurl("mysql://root:11111111@localhost");
            auto mysqlTask = WFTaskFactory::create_mysql_task(mysqlurl,1,[=](WFMySQLTask *mysqltask){
                 //0. 对任务的状态进行检测
                 int state = mysqltask->get_state();
                 int error = mysqltask->get_error();
                 if(state != WFT_STATE_SUCCESS) {
                       printf("%s\n", WFGlobal::get_error_string(state, error));
                       return;
                 }
                 //1. 检测SQL语句是否存在语法错误
                 auto mysqlResp = mysqltask->get_resp();
                 if(mysqlResp->get_packet_type() == MYSQL_PACKET_ERROR) {
                       printf("ERROR %d: %s\n", mysqlResp->get_error_code(),
                               mysqlResp->get_error_msg().c_str());
                       resp->String("Singup Failed");
                       return;
                 }
                 using namespace protocol;
                 MySQLResultCursor cursor(mysqlResp);
                //2. 成功写入数据库了
                //修改mysql内容的语句，返回插入是否成功
                 if(cursor.get_cursor_status() == MYSQL_STATUS_OK) {
                   printf("Query OK. %llu row affected.\n",cursor.get_affected_rows());
                   resp->String("Success");
                 }
                 else if(cursor.get_cursor_status() == MYSQL_STATUS_GET_RESULT){
                    //进行查找的语句，需要获取内容
                    vector<vector<MySQLCell>> matrix;
                    cursor.fetch_all(matrix);
                    string M = matrix[0][0].as_string();
                    cout << "M:" << M << endl;
                    if(encodedPassword == M){
                        //用户登陆信息正确，生成Token信息
                        Token token(username,salt);
                        string tokenStr = token.genToken();
                        //构造JSON对象，发送给client
                        using Json = nlohmann::json;
                        Json msg;
                        Json data;
                        data["Token"] = tokenStr;
                        data["Username"] = username;
                        data["Location"] = "/static/view/home.html";
                        msg["data"] = data;
                        resp->String(msg.dump());

                        auto nextTask = WFTaskFactory::create_mysql_task(mysqlurl,1,nullptr);
                        string sql("REPLACE INTO cloudisk.tbl_user_token(user_name, user_token)VALUES('");
                         sql += username + "', '" + tokenStr + "')";
                        cout << "sql : \n" << sql << endl;
                        nextTask->get_req()->set_query(sql);
                        series->push_back(nextTask);
                    }else{
                        resp->String("Login Failed");
                    }
                 }

            });
            string sql("select user_pwd from cloudisk.tbl_user where user_name = '");
            sql += username + "' limit 1";
            cout << "sql : \n" << sql << endl;
            mysqlTask->get_req()->set_query(sql);
            series->push_back(mysqlTask);
         }
    });

}
void CloudiskServer::loadUserInfoModule(){

}
void CloudiskServer::loadFileQueryMoudle(){}
void CloudiskServer::loadFileUploadMoudle(){}
void CloudiskServer::loadFileDownloadMoudle(){}
