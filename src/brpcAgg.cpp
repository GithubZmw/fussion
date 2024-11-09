#include <gflags/gflags.h>
#include "butil/logging.h"
#include "brpc/channel.h"
#include "brpc/server.h"
#include "json2pb/pb_to_json.h"
#include "output/UAV.pb.h"
#include "output/Agg.pb.h"
#include "output/TA.pb.h"
#include "fussion.h"
#include "brpcTools.h"
#include "Tools.h"


//
DECLARE_string(attachment);
DECLARE_string(protocol);
DECLARE_string(connection_type);
DECLARE_int32(timeout_ms);
DECLARE_int32(max_retry);
DECLARE_int32(interval_ms);
//
DECLARE_string(serverTA);
DEFINE_string(serverUAV, "0.0.0.0:8001", "IP Address of server");
DECLARE_string(load_balancer);
//
DECLARE_bool(echo_attachment);
DEFINE_int32(port_agg, 8001, "TCP Port of this server");
DECLARE_string(listen_addr);
DECLARE_int32(idle_timeout_s);

gmp_randstate_t state_Agg;
Params pp_AGG;
UAV_h uavH;
mpz_class M_AGG;
int t_AGG;
vector<parSig> sigmas;

/*
 * `TranServiceImpl` implements the `TranService` interface. When this service is invoked,
 * the aggregator processes and transforms the signatures collected from the UAVs,
 * generating a consolidated signature that the verifier can authenticate.
 */
namespace AGGproto {
    class TranServiceImpl : public TranService {
    public:
        virtual void Tran(google::protobuf::RpcController *cntl_base,
                          const verifyRequest *request,
                          verifyResponse *response,
                          google::protobuf::Closure *done) {

            brpc::ClosureGuard done_guard(done);
            brpc::Controller *cntl = static_cast<brpc::Controller *>(cntl_base);
            cntl->set_after_rpc_resp_fn(bind(TranServiceImpl::CallAfterRpc,
                                             placeholders::_1, placeholders::_2, placeholders::_3));
            // ---------------------------------------------------------------------------------------------------------
            /* print request message
             LOG(INFO) << "Received request=" << cntl->log_id() << "] from " << cntl->remote_side() << " to "
                      << cntl->local_side() << endl << "\tTran服务请求:"
                      << endl << "\t\tPK_v=" << request->pk_v();
            */

            // Set the transformed signatures collected by the UAV into response.
            Sigma sig = AggSig(sigmas, pp_AGG, uavH, str_to_mpz(request->pk_v()));
            response->set_auxs(mpzArr_to_str(sig.aux));
            response->set_sigs(ECPArr_to_str(sig.sig));
            response->set_ids(mpzArr_to_str(sig.IDs));
            // ---------------------------------------------------------------------------------------------------------
        }

        static void CallAfterRpc(brpc::Controller *cntl,
                                 const google::protobuf::Message *req,
                                 const google::protobuf::Message *res) {
            string req_str;
            string res_str;
            json2pb::ProtoMessageToJson(*req, &req_str, NULL);
            json2pb::ProtoMessageToJson(*res, &res_str, NULL);
            LOG(INFO) << endl << "\tTran服务响应:" << endl
                      << "\t\treq:" << req_str << endl
                      << "\t\tres:" << res_str;
        }
    };
}// namespace AGGproto


/**
 * @brief The aggregator, i.e., the UAV cluster head, retrieves the public parameters (pp) and its private key (sk).
 * @param argc Number of parameters
 * @param argv List of parameters
 * @return Returns 1 if the information is successfully retrieved, otherwise returns 0
 */
int getPpAndSk(int argc, char *argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);

    brpc::Channel channel;
    brpc::ChannelOptions options;
    options.protocol = FLAGS_protocol;              // 通信协议
    options.connection_type = FLAGS_connection_type; // 连接类型
    options.timeout_ms = FLAGS_timeout_ms;           // 超时时间（毫秒）
    options.max_retry = FLAGS_max_retry;             // 最大重试次数

    if (channel.Init(FLAGS_serverTA.c_str(), FLAGS_load_balancer.c_str(), &options) != 0) {
        LOG(ERROR) << "Fail to initialize channel";// 尝试连接TA，若失败则输出错误日志并返回 -1
        return -1;
    }

    TAproto::RegisterService_Stub stubRegister(&channel);
    if (!brpc::IsAskedToQuit()) {
        // We will receive response synchronously, safe to put variables on stack.
        TAproto::UAVIDRequest request;
        TAproto::UAVIDResponse response;
        brpc::Controller cntl;

        // 1. 生成无人机的ID并设置到请求中，这里特殊的ID表示他是簇头，也就是聚合器
        initState(state_Agg);
        mpz_class id = 0x6666666666666666666666666666666666666666666666666666666666666666_mpz;
        request.set_uav_id(mpz_to_str(id));
        cntl.request_attachment().append(FLAGS_attachment);
        stubRegister.Register(&cntl, &request, &response, NULL); // 执行注册服务
        // 检查是否成功接收到响应
        if (!cntl.Failed()) {
            LOG(INFO) << "response from " << cntl.remote_side() << endl
                      << "\tresponse: " << "{ n=" << response.params().n() << endl
                      << "\t\ttm=" << response.params().tm() << endl
                      << "\t\tq=" << response.params().q() << endl
                      << "\t\tg=" << response.params().g() << endl
                      << "\t\tbeta=" << response.params().beta() << endl
                      << "\t\tP2=" << response.params().p2() << "}" << endl
                      << "\t\tcj = {" << response.cj() << "}" << endl
                      << "\t\tsj = {" << response.sj() << "}" << endl
                      << "\t\t{M=" << response.m() << endl
                      << "\t\tt=" << response.t() << "}" << endl
                      << " ---->> latency=" << cntl.latency_us() << "us";


            // save public parameters and private key
            pp_AGG.n = response.params().n();
            pp_AGG.tm = response.params().tm();
            pp_AGG.q = str_to_mpz(response.params().q());
            pp_AGG.g = str_to_mpz(response.params().g());
            pp_AGG.beta = str_to_mpz(response.params().beta());
            pp_AGG.P2 = str_to_ECP2(response.params().p2());
            pp_AGG.PK = str_to_ECP2Arr(response.params().pk());
            uavH.ID = id;
            uavH.alpha = str_to_mpz(response.sj());
            M_AGG = str_to_mpz(response.m());
            t_AGG = response.t();
        } else {
            LOG(WARNING) << cntl.ErrorText();
        }
    }
    return 1;
}

/**
 * @brief The aggregator, i.e., the UAV cluster head, retrieves the signatures of the UAVs in the cluster. *
 * @param argc Number of command line arguments
 * @param argv Command line arguments
 * @return  0 on success, -1 on failure
 */
int collectParSigs(int argc, char *argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    brpc::Channel channel;
    brpc::ChannelOptions options;
    options.protocol = FLAGS_protocol;
    options.connection_type = FLAGS_connection_type;
    options.timeout_ms = FLAGS_timeout_ms;
    options.max_retry = FLAGS_max_retry;

    /*
     * Collect multiple signatures from various UAVs and store them in a variable for subsequent transformation (Tran).
     * To achieve this, connections to multiple UAVs are established, starting with the first UAV on port 8002,
     * with each subsequent UAV assigned to incrementally higher ports.
     */
    std::string ip = "0.0.0.0";
    int base_port = 8002;
    int num_iterations = t_AGG;

    for (int i = 0; i < num_iterations; ++i) {
        int port = base_port + i; // 计算当前端口号
        std::string server_address = ip + ":" + std::to_string(port);
        if (channel.Init(server_address.c_str(), FLAGS_load_balancer.c_str(), &options) != 0) {
            LOG(ERROR) << "Fail to initialize channel for " << server_address;
            return -1;
        }
        UAVproto::SignService_Stub stubSign(&channel);
        if (!brpc::IsAskedToQuit()) {
            // We will receive response synchronously, safe to put variables on stack.
            UAVproto::SignRequest request;
            UAVproto::SignResponse response;
            brpc::Controller cntl;
            initState(state_Agg);
            request.set_m(mpz_to_str(M_AGG));
            request.set_t(t_AGG);
            cntl.request_attachment().append(FLAGS_attachment);
            stubSign.sign(&cntl, &request, &response, NULL); // 执行注册服务
            // 检查是否成功接收到响应
            if (!cntl.Failed()) {
                LOG(INFO) << "response from " << cntl.remote_side() << endl
                          << "\tresponse: ID=" << response.id() << endl
                          << "\t\tparSig=" << response.sig() << endl
                          << "\t\tcj=" << response.cj() << endl
                          << " ---->> latency=" << cntl.latency_us() << "us";

                // Store the collected UAV signatures in an vector for subsequent transformation (Tran) operations.
                parSig sigma;
                sigma.cj = str_to_mpz(response.cj());
                sigma.ID = str_to_mpz(response.id());
                sigma.sig = str_to_ECP(response.sig());
                sigmas.push_back(sigma);
            } else {
                LOG(WARNING) << cntl.ErrorText();
            }
        }
    }

    return 1;
}

/**
 * @brief Starts the Tran service, which transforms UAV signatures into signatures that can be verified by the verifier.
 * @param argc Number of command-line arguments
 * @param argv Command-line arguments
 * @return Returns 0 on success, -1 on failure
 */
int startServerTran(int argc, char *argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    brpc::Server server;
    AGGproto::TranServiceImpl tran_service_impl;
    if (server.AddService(&tran_service_impl, brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        LOG(ERROR) << "Fail to add tran_service_impl service";  // 添加服务失败时记录错误日志
        return -1;
    }
    butil::EndPoint point;
    if (!FLAGS_listen_addr.empty()) {
        if (butil::str2endpoint(FLAGS_listen_addr.c_str(), &point) < 0) {
            LOG(ERROR) << "Invalid listen address:" << FLAGS_listen_addr;  // 无效地址记录错误日志
            return -1;
        }
    } else {
        point = butil::EndPoint(butil::IP_ANY, FLAGS_port_agg);
    }

    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_s;  // 设置空闲超时时间
    if (server.Start(point, &options) != 0) {  // 尝试启动服务器
        LOG(ERROR) << "Fail to start TranServer";  // 启动失败时记录错误日志
        return -1;
    }

    server.RunUntilAskedToQuit();
    return 1;
}


//int main(int argc, char *argv[]) {
//    getPpAndSk(argc, argv);
//    collectParSigs(argc, argv);
//    startServerTran(argc, argv);// 启动服务器
//    return 0;
//}
