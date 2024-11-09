#include <gflags/gflags.h>
#include "butil/logging.h"
#include "brpc/server.h"
#include "json2pb/pb_to_json.h"
#include "output/TA.pb.h"
#include "fussion.h"
#include "brpcTools.h"

using namespace std;

DEFINE_bool(echo_attachment, true, "Echo attachment as well");
DEFINE_int32(port_ta, 8000, "TCP Port of this server");
DEFINE_string(listen_addr, "", "Server listen address, may be IPV4/IPV6/UDS."
                               " If this is set, the flag port will be ignored");
DEFINE_int32(idle_timeout_s, -1, "Connection will be closed if there is no "
                                 "read/write operations during the last `idle_timeout_s'");

gmp_randstate_t state_TA;
Params pp;
mpz_class sk;//簇头的私钥
vector<mpz_class> d;
vector<mpz_class> b;
mpz_class M;
int t;


/*
 * The defined proto file has been compiled, generating an interface class.
 * Here, we provide concrete implementations for the functions within this interface class to define the required business logic.
 */
namespace TAproto {
    using namespace brpc;

    class RegisterServiceImpl : public RegisterService {
    public:
        virtual void Register(google::protobuf::RpcController *cntl_base,
                              const UAVIDRequest *request,
                              UAVIDResponse *response,
                              google::protobuf::Closure *done) {

            ClosureGuard done_guard(done);
            Controller *cntl = static_cast<brpc::Controller *>(cntl_base);
            cntl->set_after_rpc_resp_fn(bind(&RegisterServiceImpl::CallAfterRpc,
                                             placeholders::_1, placeholders::_2, placeholders::_3));

            // ---------------------------------------------------------------------------------------------------------
            /* print request info , for debugging
            LOG(INFO) << "Received request[log_id=" << cntl->log_id() << "] from " << cntl->remote_side()
                      << " to " << cntl->local_side() << endl << "\t注册服务请求:" << endl
                      << "\tUAV_ID = " << request->uav_id() << endl;
             */

            // TA issues public parameters to entities in the system.
            mpz_class id = str_to_mpz(request->uav_id());
            response->mutable_params()->set_n(pp.n);
            response->mutable_params()->set_tm(pp.tm);
            response->mutable_params()->set_q(mpz_to_str(pp.q));
            response->mutable_params()->set_p2(ECP2_to_str(pp.P2));
            response->mutable_params()->set_g(mpz_to_str(pp.g));
            response->mutable_params()->set_beta(mpz_to_str(pp.beta));
            response->mutable_params()->set_pk(ECP2Arr_to_str(pp.PK));
            response->set_m(mpz_to_str(M));
            response->set_t(t);
            /*
             * Generate a key for the UAV by executing KeyGen, create a signed message, and set the threshold t.
             * If the registered entity is a cluster head, issue it a signature transformation key.
             * Here, we use a unique identity ID to identify the cluster head
             */
            string isUAVh = "6666666666666666666666666666666666666666666666666666666666666666";
            if (request->uav_id() != isUAVh) {
                UAV uav = getUAV(pp, d, b, id);
                response->set_cj(mpzArr_to_str(uav.c1));
                response->set_sj(mpzArr_to_str(uav.c2));
            } else {
                response->set_sj(mpz_to_str(sk));//store sk
            }
            // ---------------------------------------------------------------------------------------------------------
        }

        /*
         * `CallAfterRpc` static callback function: executed after the response is sent, used to log the request
         * and response content in JSON format for easier debugging.
         */
        static void CallAfterRpc(brpc::Controller *cntl,
                                 const google::protobuf::Message *req,
                                 const google::protobuf::Message *res) {
            string req_str;
            string res_str;
            json2pb::ProtoMessageToJson(*req, &req_str, NULL);
            json2pb::ProtoMessageToJson(*res, &res_str, NULL);

            LOG(INFO) << endl << "\t注册服务响应:" << endl
                      << "\t\treq:" << req_str << endl
                      << "\t\tres:" << res_str;
        }
    };
}// namespace UAVproto

// -------------------------------------------------------------------------------------------------------------------

/**
 * @brief Starts the server responsible for handling registration requests from other entities.
 * @param argc Number of command-line arguments
 * @param argv Command-line arguments
 * @return Returns 1 if the server starts successfully, otherwise returns -1
 */
int startServerTA(int argc, char *argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    brpc::Server server;
    TAproto::RegisterServiceImpl register_service_impl;
    if (server.AddService(&register_service_impl, brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        LOG(ERROR) << "Fail to add register_service service";  // 添加服务失败时记录错误日志
        return -1;
    }
    butil::EndPoint point;
    if (!FLAGS_listen_addr.empty()) {
        if (butil::str2endpoint(FLAGS_listen_addr.c_str(), &point) < 0) {
            LOG(ERROR) << "Invalid listen address:" << FLAGS_listen_addr;  // 无效地址记录错误日志
            return -1;
        }
    } else {
        point = butil::EndPoint(butil::IP_ANY, FLAGS_port_ta);
    }
    // 启动服务器
    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_s;
    if (server.Start(point, &options) != 0) {
        LOG(ERROR) << "Fail to start EchoServer";
        return -1;
    }
    // 使服务器持续运行，直到接收到退出信号（如 Ctrl-C）
    server.RunUntilAskedToQuit();
    return 1;
}
/**
 * @brief Initializes the system parameters and generates the public key.
 */
void initParams() {
    initState(state_TA);
    int n = 52, tm = 20;
    pp = Setup(sk, n, tm);
    for (int i = 0; i < tm - 1; ++i) { // 阈值 tm 需要 tm-1 次多项式
        b.push_back(rand_mpz(state_TA));
        d.push_back(rand_mpz(state_TA));
    }
    pp.PK = getPK(b);
    t = tm / 2 + 1;
    M = 123456789;
}


//int main(int argc, char *argv[]) {
//    initParams();// 初始化系统公钥
//    startServerTA(argc, argv);// 启动服务器
//    return 0;
//}
