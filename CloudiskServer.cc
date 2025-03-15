#include "CloudiskServer.h"
#include "Token.h"
#include "Hash.h"
#include "server_rpc/signup.pb.h"
#include "unixHeader.h"

#include <csignal>
#include <fcntl.h>
#include <nlohmann/detail/meta/type_traits.hpp>
#include <nlohmann/json_fwd.hpp>
#include <srpc/rpc_context.h>
#include <string>
#include <unistd.h>
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

#include "Oss.h"
#include "Rabbitmq.h"

#include "signup.srpc.h"
#include "workflow/WFFacilities.h"
#include <workflow/Communicator.h>

using namespace wfrest;
using namespace srpc;
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
    loadUserInfoModule();
    loadFileQueryMoudle();
    loadFileUploadMoudle();
    loadFileDownloadMoudle();

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
    _httpserver.POST("/user/signup",[](const HttpReq *req, HttpResp *resp,SeriesWork *series){
                     if(req->content_type() == APPLICATION_URLENCODED){
                     //接受urlencoded类型的请求体中的信息,以map键值对的方式存储
                     auto formKV = req->form_kv();
                     //获取用户名和密码
                     string username = formKV["username"];
                     string password = formKV["password"];
                     //rpc调用
                     GOOGLE_PROTOBUF_VERIFY_VERSION;
                     const char *ip = "127.0.0.1";                                         
                     unsigned short port = 12345;

                     //client 是服务端在客户端的代理
                     UserService::SRPCClient client(ip, port);

                     //ReqSignup是函数参数
                     ReqSignup req;
                     req.set_username(username);
                     req.set_password(password);

                     auto task = client.create_Signup_task([resp](RespSignup *response, srpc::RPCContext *context){
                                                           if(response->code() == 100){
                                                           resp->String("SUCCESS");
                                                           }else{
                                                           resp->String("Signup Failed");
                                                           }
                                                           });
                     //将rpc函数的参数放进去
                     task->serialize_input(&req);
                     series->push_back(task);
                     }
    });
}


    // void CloudiskServer::loadUserRegisterMoudle(){
    //     //接收用户的注册信息
    //     //执行流：框架收到了POST方法，并且url匹配完成。调用相对应的lambda表达式：
    //     //          lambda表达式处理的内容：获取用户信息，设置mysql任务，将mysql任务放入序列
    //     //          执行完mysql任务后，进入mysql任务的回调函数，检查mysql的响应，判断是否出错。
    //     _httpserver.POST("/user/signup",[](const HttpReq *req, HttpResp *resp,SeriesWork *series){
    //         if(req->content_type() == APPLICATION_URLENCODED){
    //             //接受urlencoded类型的请求体中的信息,以map键值对的方式存储
    //             auto formKV = req->form_kv();
    //             //获取用户名和密码
    //             string username = formKV["username"];
    //             string password = formKV["password"];
    //             //加密密码，将加密后的密码存储在MySQL
    //             string salt("12345678");
    //             string encodedPassword(crypt(password.c_str(),salt.c_str()));
    //             string mysqlurl("mysql://root:11111111@localhost");
    //             //创建一个mysql任务，向mysql服务器发起请求。在回调函数中，对mysql服务器的响应做处理
    //             auto mysqlTask = WFTaskFactory::create_mysql_task(mysqlurl,1,
    //                      [resp](WFMySQLTask *mysqltask){
    //                      //检测任务状态,即workflow是否有问题
    //                      int state = mysqltask->get_state();
    //                      int error = mysqltask->get_error();
    //                      if(state != WFT_STATE_SUCCESS){
    //                      printf("%s\n",WFGlobal::get_error_string(state,error));
    //                      return;
    //                      }
    //                      //检测SQL语句是否存在语法错误,即server发送的消息是否有问题
    //                      //如果是语法错误，需要返回提示信息给服务器,用于跟踪问题
    //                      auto mysqlResp = mysqltask->get_resp();
    //                      if(mysqlResp->get_packet_type() == MYSQL_PACKET_ERROR){
    //                      printf("ERROR %d: %s\n", mysqlResp->get_error_code(),
    //                             mysqlResp->get_error_msg().c_str());
    //                      resp->String("Signup Failed");
    //                      return;
    //                      }
    //                      //mysql数据库写入是否有问题，比如数据已存在
    //                      using namespace protocol;
    //                      MySQLResultCursor cursor(mysqlResp);
    //                      if(cursor.get_cursor_status() == MYSQL_STATUS_OK){
    //                          printf("Query OK. %llu row affected.\n",cursor.get_affected_rows());
    //                          resp->String("Success");
    //                      }else{
    //                          resp->String("Signup Failed");
    //                      }
    //                      });
    //             string sql("INSERT INTO cloudisk.tbl_user(user_name, user_pwd) VALUES('");
    //             sql += username + "','" + password + "')";
    //             mysqlTask->get_req()->set_query(sql);
    //             series->push_back(mysqlTask);
    //           }
    //     });
    // }

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
        _httpserver.GET("/user/info",[](const HttpReq *req, HttpResp *resp, SeriesWork *series){
                        //1.从请求中读取用户名、Token
                        string username = req->query("username");
                        string password = req->query("token");
                        //2.检查Token是否有效
                        //略
                        //3.根据用户名，去mysql查询用户信息
                        string mysqlurl("mysql://root:11111111@localhost");
                        auto mysqlTask = WFTaskFactory::create_mysql_task(mysqlurl,1,[=](WFMySQLTask *mysqltask){
                                                                          //检测任务状态、检测mysql语法、检测执行sql后返回的信息
                                                                          //略
                                                                          using namespace protocol;
                                                                          auto mysqlResp = mysqltask->get_resp();
                                                                          MySQLResultCursor cursor(mysqlResp);
                                                                          if(cursor.get_cursor_status() == MYSQL_STATUS_GET_RESULT){
                                                                          vector<vector<MySQLCell>> matrix;
                                                                          cursor.fetch_all(matrix);
                                                                          string signupAt = matrix[0][0].as_string();
                                                                          using Json = nlohmann::json;
                                                                          Json msg;
                                                                          Json data;
                                                                          data["Username"] = username;
                                                                          data["SignupAt"] = signupAt;
                                                                          msg["data"] = data;
                                                                          resp->String(msg.dump());
                                                                          }else{
                                                                          resp->String("error");
                                                                          }
                                                                          });

                        string sql("select signup_at from cloudisk.tbl_user where user_name = '");
                        sql += username + "'";
                        mysqlTask->get_req()->set_query(sql);
                        series->push_back(mysqlTask);
        });

    }
    void CloudiskServer::loadFileQueryMoudle(){
        _httpserver.POST("/file/query", [](const HttpReq *req, HttpResp * resp, SeriesWork * series){
                         //1. 解析请求: 查询词
                         string username = req->query("username");
                         string tokenStr = req->query("token");
                         cout << "username:" << username << endl;
                         cout << "token:" << tokenStr << endl;
                         //2. 解析请求： 消息体
                         string limitCnt = req->form_kv()["limit"];

                         string mysqlurl("mysql://root:11111111@localhost");
                         auto mysqlTask = WFTaskFactory::create_mysql_task(mysqlurl, 1, 
                                                                           [=](WFMySQLTask * mysqltask){
                                                                           //...检测
                                                                           using namespace protocol;
                                                                           auto mysqlResp = mysqltask->get_resp();
                                                                           MySQLResultCursor cursor(mysqlResp);
                                                                           if(cursor.get_cursor_status() == MYSQL_STATUS_GET_RESULT) {
                                                                           //读操作,获取用户的
                                                                           vector<vector<MySQLCell>> matrix;
                                                                           cursor.fetch_all(matrix);
                                                                           using Json = nlohmann::json;
                                                                           Json msgArr;
                                                                           for(size_t i = 0; i < matrix.size(); ++i) {
                                                                           Json row;
                                                                           row["FileHash"] = matrix[i][0].as_string();
                                                                           row["FileName"] = matrix[i][1].as_string();
                                                                           row["FileSize"] = matrix[i][2].as_ulonglong();
                                                                           row["UploadAt"] = matrix[i][3].as_datetime();
                                                                           row["LastUpdated"] = matrix[i][4].as_datetime();
                                                                           msgArr.push_back(row);//在数组中添加一个元素,使用push_back即可
                                                                           }
                                                                           resp->String(msgArr.dump());
                                                                           } else {
                                                                               //没有读取到正确的信息
                                                                               resp->String("error");
                                                                           }
                                                                           });
                         string sql("select file_sha1, file_name, file_size, upload_at, last_update from cloudisk.tbl_user_file where user_name = '");
                         sql += username + "' limit " + limitCnt;
                         cout << "\nsql:\n"  << sql << endl;
                         mysqlTask->get_req()->set_query(sql);
                         series->push_back(mysqlTask);
        });   
    }
    void CloudiskServer::loadFileUploadMoudle(){
        _httpserver.POST("/file/upload",[](const HttpReq *req, HttpResp *resp, SeriesWork *series){
                         //解析请求
                         string username = req->query("username");
                         string tokenStr = req->query("token");
                         cout << "username:" << username << endl;
                         cout << "token:" << tokenStr << endl;
                         //验证token
                         //略
                         //解析消息体
                         if(req->content_type() == MULTIPART_FORM_DATA){
                         auto form = req->form();
                         string filename = form["file"].first;
                         std::cout << filename << std::endl;
                         string content = form["file"].second;
                         mkdir("tmp",0755);
                         string filepath = "tmp/" + filename;
                         int fd = open(filepath.c_str(),O_CREAT|O_RDWR,0664);
                         if(fd < 0){
                         perror("open");
                         return;
                         }
                         write(fd,content.c_str(),content.size());
                         close(fd);
                         resp->String("upload Success");

                         //生成SHA1值
                         Hash hash(filepath);
                         string filehash = hash.sha1();
                         cout << filehash << endl;
                         //写入数据库
                         string mysqlurl("mysql://root:11111111@localhost");
                         auto mysqlTask = WFTaskFactory::create_mysql_task(mysqlurl,1,nullptr);
                         string sql("INSERT INTO cloudisk.tbl_user_file(user_name,file_sha1,file_size,file_name)VALUES('");
                         sql += username + "','" + filehash + "'," + std::to_string(content.size()) + ",'" + filename + "')";
                         cout << "sql = " << sql << endl;
                         mysqlTask->get_req()->set_query(sql);
                         series->push_back(mysqlTask);

                         //备份到阿里云
                         // Oss oss;
                         // string remote_path = "tmp/" + filename;
                         // oss.putFile(remote_path,filepath);

                         //将备份的文件信息，发给消息队列
                         Rabbitmq mq;
                         Json msg;
                         msg["filepath"] = filepath;
                         mq.publish(msg.dump());
                         }
        });
    }
    void CloudiskServer::loadFileDownloadMoudle(){
        _httpserver.GET("/file/downloadurl", [](const HttpReq *req, HttpResp * resp){
                        string filename = req->query("filename");
                        cout << "filename: " << filename << endl;




                        //将下载业务从服务器中分离出去，之后只需要产生一个下载链接就可以了
                        //这要求我们还需要去部署一个下载服务器
                        //string downloadURL = "http://10.211.55.4:8080/" + filename;
                        string remotePath = "tmp/" + filename;
                        Oss oss;
                        string downloadURL = oss.getFileUrl(remotePath);
                        resp->String(downloadURL);
                        });
    }
