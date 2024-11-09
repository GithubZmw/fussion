//#include <brpc/channel.h>
//// A client sending requests to server every 1 second.
//
//#include <gflags/gflags.h>
//#include "butil/logging.h"
//#include "butil/time.h"
//#include "brpc/channel.h"
//#include "output/echo.pb.h"
//
//
//DEFINE_string(attachment, "", "Carry this along with requests");
//DEFINE_string(protocol, "baidu_std", "Protocol type. Defined in src/brpc/options.proto");
//DEFINE_string(connection_type, "", "Connection type. Available values: single, pooled, short");
//DEFINE_string(server, "0.0.0.0:8000", "IP Address of server");
//DEFINE_string(load_balancer, "", "The algorithm for load balancing");
//DEFINE_int32(timeout_ms, 100, "RPC timeout in milliseconds");
//DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)");
//DEFINE_int32(interval_ms, 1000, "Milliseconds between consecutive requests");
//
//
//int startUAV(int argc, char* argv[]){
//    // 解析命令行参数，使用 gflags 提供的 ParseCommandLineFlags 函数解析参数
//    google::ParseCommandLineFlags(&argc, &argv, true);
//
//    // 定义并初始化 Channel ，NULL 表示使用默认选项。Channel 表示与服务器的通信通道，它是线程安全的，可以在程序中的所有线程间共享
//    brpc::Channel channel;
//    brpc::ChannelOptions options;
//    options.protocol = FLAGS_protocol;              // 通信协议
//    options.connection_type = FLAGS_connection_type; // 连接类型
//    options.timeout_ms = FLAGS_timeout_ms;           // 超时时间（毫秒）
//    options.max_retry = FLAGS_max_retry;             // 最大重试次数
//
//
//    // 尝试初始化 Channel，若失败则输出错误日志并返回 -1
//    if (channel.Init(FLAGS_server.c_str(), FLAGS_load_balancer.c_str(), &options) != 0) {
//        LOG(ERROR) << "Fail to initialize channel";
//        return -1;
//    }
//
//    // 通常不直接调用 Channel，而是构造一个封装它的 stub Service ， stub 也是线程安全的，可在所有线程间共享
//    example::EchoService_Stub stub(&channel);
//
//    // 每隔 1 秒发送一个请求并等待响应 ---------------------------------------------------------------------------------------
//    int log_id = 0;
//    while (!brpc::IsAskedToQuit()) {
//        // We will receive response synchronously, safe to put variables on stack.
//        example::EchoRequest request;
//        example::EchoResponse response;
//        brpc::Controller cntl;
//
//        request.set_message("hello world");
//
//        cntl.set_log_id(log_id ++);  // set by user
//        // Set attachment which is wired to network directly instead of being serialized into protobuf messages.
//        cntl.request_attachment().append(FLAGS_attachment);
//
//        // Because done'(last parameter) is NULL, this function waits until,the response comes back or error occurs(including timedout).
//        stub.Echo(&cntl, &request, &response, NULL);
//        // 检查是否成功接收到响应
//        if (!cntl.Failed()) {
//            LOG(INFO) << "Received response from " << cntl.remote_side()
//                      << " to " << cntl.local_side()
//                      << ": " << response.message() << " (attached="
//                      << cntl.response_attachment() << ")"
//                      << " latency=" << cntl.latency_us() << "us";
//        } else {
//            LOG(WARNING) << cntl.ErrorText();
//        }
//
//
//
//        example::MutiParamsRequest MutiRequest;
//        example::MutiParamsResponse MutiResponse;
//        brpc::Controller cntl2;
//        cntl2.set_log_id(log_id ++);  // set by user
//        // 通常不直接调用 Channel，而是构造一个封装它的 stub Service ， stub 也是线程安全的，可在所有线程间共享
//        example::MutiParamsService_Stub MutiStub(&channel);
//        MutiRequest.set_param1("I am Tom");
//        MutiRequest.set_param2("This is a big house");
//        MutiStub.MutiParams(&cntl2, &MutiRequest, &MutiResponse, NULL);
//        // 检查是否成功接收到响应
//        if (!cntl2.Failed()) {
//            LOG(INFO) << "Received MutiResponse: "
//                      << ": " << MutiResponse.param1()
//                      << ": " << MutiResponse.param2()
//                      << " (attached="
//                      << cntl2.response_attachment() << ")"
//                      << " latency=" << cntl2.latency_us() << "us";
//        } else {
//            LOG(WARNING) << cntl2.ErrorText();
//        }
//
//        usleep(FLAGS_interval_ms * 1000L); // 按指定的间隔发送下一个请求-------------------------------------------
//    }
//    // 当接收到退出信号时，客户端退出
//    LOG(INFO) << "EchoClient is going to quit";
//
//    return 1;
//}
//
////int main(int argc, char* argv[]) {
////    startUAV(argc,argv);
////    return 0;
////}