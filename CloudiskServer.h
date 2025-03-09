#ifndef __ClouddiskServer_H__
#define __ClouddiskServer_H__

#include <workflow/WFFacilities.h>
#include <wfrest/HttpServer.h>

class CloudiskServer{
public:
    CloudiskServer(int cnt)
        :_waitGroup(cnt)
    {}

    ~CloudiskServer(){}

    void start(unsigned short port);
    void loadModules();

private:
    //加载静态资源
    void loadStaticResourceModule();
    //加载注册模块
    void loadUserRegisterMoudle();
    //加载登陆模块
    void loadUserLoginMoudle();
    //加载用户信息模块
    void loadUserInfoModule();
    //加载文件查询模块
    void loadFileQueryMoudle();
    //加载文件上传模块
    void loadFileUploadMoudle();
    //加载文件下载模块
    void loadFileDownloadMoudle();

private:
    WFFacilities::WaitGroup _waitGroup;
    wfrest::HttpServer _httpserver;
};

#endif
