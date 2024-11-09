#include "Tools.h"

//// -------------------------------- 2. 方案涉及实体的定义 ----------------------------------------

// 无人机中的簇头节点
typedef struct {
    mpz_class ID;
    mpz_class alpha;
}UAV_h;


// 无人机群组中的普通无人机，c1,c2是无人机的份额重构密钥
typedef struct {
    vector<mpz_class> c1;
    vector<mpz_class> c2;
    mpz_class ID;
}UAV;

// -------------------------------- 3. 参数和通信消息定义 ----------------------------------------

// 系统公共参数
typedef struct {
    int n;
    int tm;
    mpz_class q;
    ECP2 P2;
    mpz_class g;
    mpz_class beta;
    vector<ECP2> PK;
}Params;

typedef struct {
    mpz_class cj;
    ECP sig;
    mpz_class ID;
}parSig;


typedef struct {
    vector<mpz_class> aux;
    vector<ECP> sig;
    vector<mpz_class> IDs;
}Sigma;



vector<mpz_class> getFactors();
bool isCoprime(mpz_class x, vector<mpz_class> factors);
mpz_class hashToCoprime(mpz_class beHashed, mpz_class module, vector<mpz_class> factors);
Params Setup(mpz_class &alpha, int n, int tm);
vector<ECP2> getPK(vector<mpz_class> b);
UAV getUAV(Params pp, vector<mpz_class> d, vector<mpz_class> b, mpz_class id);
vector<UAV> KeyGen(Params &params, mpz_class alpha, UAV_h &uavH);
parSig Sign(Params pp, UAV uav, int t, mpz_class M);
vector<parSig> collectSig(Params pp, vector<UAV> UAVs, int t, mpz_class M);
Sigma AggSig(vector<parSig> parSigs, Params pp, UAV_h uavH, mpz_class PK_v);
vector<mpz_class> getPi_0(Params pp, vector<mpz_class> ID, int t);
int Verify(Sigma sigma, mpz_class sk_v, Params pp, mpz_class M, int t);