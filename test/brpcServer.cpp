//#include <gflags/gflags.h>
//#include "butil/logging.h"
//#include "brpc/server.h"
//#include "json2pb/pb_to_json.h"
//#include "output/echo.pb.h"
//
//using namespace std;
//
//DEFINE_bool(echo_attachment, true, "Echo attachment as well");
//DEFINE_int32(port, 8000, "TCP Port of this server");
//DEFINE_string(listen_addr, "", "Server listen address, may be IPV4/IPV6/UDS."
//                               " If this is set, the flag port will be ignored");
//DEFINE_int32(idle_timeout_s, -1, "Connection will be closed if there is no "
//                                 "read/write operations during the last `idle_timeout_s'");
//
//// 声明命名空间 example
//// EchoServiceImpl 继承了 EchoService , 通过继承生成的 EchoService 来实现 bRPC 服务
//namespace example {
//    class EchoServiceImpl : public EchoService {
//    public:  // //构造函数和析构函数 EchoServiceImpl() {}  ; virtual ~EchoServiceImpl() {}
//        // proto文件中定义了服务Echo, 这里给出 Echo 方法的实现. (接收客户端请求并返回响应)
//        virtual void Echo(google::protobuf::RpcController *cntl_base,
//                          const EchoRequest *request, // proto文件中定义的,客户端请求的消息
//                          EchoResponse *response,     // proto文件中定义的,服务端响应的消息
//                          google::protobuf::Closure *done) {
//
//            // 创建 ClosureGuard 对象，确保在处理完请求后自动调用 done->Run()，标记完成,同步调用时才会使用
//            brpc::ClosureGuard done_guard(done);
//
//            // 将 cntl_base 转换为 bRPC 的 Controller 以访问控制信息
//            brpc::Controller *cntl = static_cast<brpc::Controller *>(cntl_base);
//
//            // 设置在响应发送后调用的回调函数 CallAfterRpc,std::bind 用于绑定 CallAfterRpc 函数到 after_rpc_resp_fn 回调中
//            cntl->set_after_rpc_resp_fn(bind(&EchoServiceImpl::CallAfterRpc,
//                                             placeholders::_1, placeholders::_2, placeholders::_3)); // 这里表示回调函数有三个参数
//
//            // ---------------------------------------------------------------------------------------------------------
//            // 记录请求的详细信息：日志 ID、客户端地址、服务器地址、请求内容和请求附件
//            LOG(INFO) << "Received request[log_id=" << cntl->log_id()
//                      << "] from " << cntl->remote_side()
//                      << " to " << cntl->local_side()
//                      << ": " << request->message()
//                      << " (attached=" << cntl->request_attachment() << ")";
//
//            // 设置响应内容，将请求中的 message 字段内容复制到响应消息中
//            response->set_message(request->message());
//            // ---------------------------------------------------------------------------------------------------------
//
//            // 可选：设置响应的压缩方式，如 GZIP（注释掉未启用，启用前需要评估性能）
//            // cntl->set_response_compress_type(brpc::COMPRESS_TYPE_GZIP);
//
//            // 如果 FLAGS_echo_attachment 为 true，将请求的附件直接传输到响应附件中
//            if (FLAGS_echo_attachment) {
//                cntl->response_attachment().append(cntl->request_attachment());
//            }
//        }
//
//
//        // CallAfterRpc 静态回调函数：在响应发送后执行，用于记录请求和响应内容的 JSON 形式
//        static void CallAfterRpc(brpc::Controller *cntl,
//                                 const google::protobuf::Message *req,
//                                 const google::protobuf::Message *res) {
//            // 将请求和响应转换为 JSON 格式的字符串，方便记录和调试
//            string req_str;
//            string res_str;
//            json2pb::ProtoMessageToJson(*req, &req_str, NULL);
//            json2pb::ProtoMessageToJson(*res, &res_str, NULL);
//
//            // 记录请求和响应的 JSON 格式内容
//            LOG(INFO) << "req:" << req_str
//                      << " res:" << res_str;
//        }
//
//    };
//
//
//    class MutiParamsServiceImpl : public MutiParamsService {
//    public:
//        MutiParamsServiceImpl() {}
//
//        virtual ~MutiParamsServiceImpl() {}
//
//        void MutiParams(::google::protobuf::RpcController *cntl_base,
//                        const ::example::MutiParamsRequest *request,
//                        ::example::MutiParamsResponse *response,
//                        ::google::protobuf::Closure *done) {
//            // 创建 ClosureGuard 对象，确保在处理完请求后自动调用 done->Run()，标记完成,同步调用时才会使用
//            brpc::ClosureGuard done_guard(done);
//
//            // 将 cntl_base 转换为 bRPC 的 Controller 以访问控制信息
//            brpc::Controller *cntl = static_cast<brpc::Controller *>(cntl_base);
//
//            // 设置在响应发送后调用的回调函数 CallAfterRpc,std::bind 用于绑定 CallAfterRpc 函数到 after_rpc_resp_fn 回调中
//            cntl->set_after_rpc_resp_fn(bind(&MutiParamsServiceImpl::MutiCallAfterRpc,
//                                             placeholders::_1, placeholders::_2,
//                                             placeholders::_3)); // 这里表示回调函数有三个参数
//
//            // ---------------------------------------------------------------------------------------------------------
//            // 记录请求的详细信息：日志 ID、客户端地址、服务器地址、请求内容和请求附件
//            LOG(INFO) << "Received request[log_id=" << cntl->log_id()
//                      << "] from " << cntl->remote_side()
//                      << " to " << cntl->local_side()
//                      << ": " << request->param1() << "," << request->param2()
//                      << " (attached=" << cntl->request_attachment() << ")";
//
//            // 设置响应内容，将请求中的 message 字段内容复制到响应消息中
//            response->set_param1(request->param1());
//            response->set_param2(request->param2());
//            // ---------------------------------------------------------------------------------------------------------
//
//            // 如果 FLAGS_echo_attachment 为 true，将请求的附件直接传输到响应附件中
//            if (FLAGS_echo_attachment) {
//                cntl->response_attachment().append(cntl->request_attachment());
//            }
//        }
//
//        // CallAfterRpc 静态回调函数：在响应发送后执行，用于记录请求和响应内容的 JSON 形式
//        static void MutiCallAfterRpc(brpc::Controller *cntl,
//                                     const google::protobuf::Message *req,
//                                     const google::protobuf::Message *res) {
//            // 将请求和响应转换为 JSON 格式的字符串，方便记录和调试
//            string req_str;
//            string res_str;
//            json2pb::ProtoMessageToJson(*req, &req_str, NULL);
//            json2pb::ProtoMessageToJson(*res, &res_str, NULL);
//
//            // 记录请求和响应的 JSON 格式内容
//            LOG(INFO) << "MutiReq:" << req_str
//                      << " MutiRes:" << res_str;
//        }
//
//
//    };  // namespace example
//}
//
//// -------------------------------------------------------------------------------------------------------------------
//
//int startServer(int argc, char *argv[]) {
//    // 解析命令行参数，使用 gflags 提供的 ParseCommandLineFlags 函数解析参数
//    google::ParseCommandLineFlags(&argc, &argv, true);
//
//    // 创建 bRPC 服务器实例。一般来说，一个进程只需要一个 Server 实例
//    brpc::Server server;
//
//    // 创建服务实例，并将定义的 EchoServiceImpl 服务对象添加到服务器中，注意第二个参数：SERVER_DOESNT_OWN_SERVICE
//    // 表示服务对象不由服务器管理，因为它在栈上分配。如果想由服务器管理，可以用 SERVER_OWNS_SERVICE
//    example::EchoServiceImpl echo_service_impl;
//    example::MutiParamsServiceImpl muti_service_impl;
//    if (server.AddService(&echo_service_impl, brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
//        LOG(ERROR) << "Fail to add service";  // 添加服务失败时记录错误日志
//        return -1;
//    }
//    if (server.AddService(&muti_service_impl, brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
//        LOG(ERROR) << "Fail to add service";  // 添加服务失败时记录错误日志
//        return -1;
//    }
//
//    // 设置服务器监听的地址
//    butil::EndPoint point;
//    if (!FLAGS_listen_addr.empty()) {  // 检查是否指定了监听地址
//        // 如果指定了监听地址，将其转换为端点格式
//        if (butil::str2endpoint(FLAGS_listen_addr.c_str(), &point) < 0) {
//            LOG(ERROR) << "Invalid listen address:" << FLAGS_listen_addr;  // 无效地址记录错误日志
//            return -1;
//        }
//    } else {
//        // 如果没有指定地址，使用通配地址和端口 FLAGS_port
//        point = butil::EndPoint(butil::IP_ANY, FLAGS_port);
//    }
//
//    // 启动服务器
//    brpc::ServerOptions options;
//    options.idle_timeout_sec = FLAGS_idle_timeout_s;  // 设置空闲超时时间
//    if (server.Start(point, &options) != 0) {  // 尝试启动服务器
//        LOG(ERROR) << "Fail to start EchoServer";  // 启动失败时记录错误日志
//        return -1;
//    }
//
//    // 使服务器持续运行，直到接收到退出信号（如 Ctrl-C）
//    server.RunUntilAskedToQuit();
//
//    return 1;
//}
//
////int main(int argc, char *argv[]) {
////    startServer(argc,argv);
////    return 0;
////}
