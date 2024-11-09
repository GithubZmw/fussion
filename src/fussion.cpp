#include "fussion.h"
#include <openssl/rand.h>

csprng rng;
gmp_randstate_t state_gmp;

auto seed = time(NULL);

#define DEBUG 1

vector<mpz_class> getFactors() {
    vector<mpz_class> factors;
    int addends[] = {2, 3, 11, 19, 10177, 125527, 859267, 906349, 2508409, 2529403, 52437899, 254760293};
    mpz_class factor;
    for (int i = 0; i < 12; ++i) {
        factor = addends[i];
        factors.push_back(factor);
    }
    return factors;
}

bool isCoprime(mpz_class x, vector<mpz_class> factors) {
    for (int i = 0; i < factors.size(); ++i)
        if (x % factors[i] == 0)
            return false;
    return true;
}

mpz_class hashToCoprime(mpz_class beHashed, mpz_class module, vector<mpz_class> factors) {
    mpz_class hash = hashToZp256(beHashed, module);
    mpz_class inc = 254760293;
    while (!isCoprime(hash, factors)) {
        hash += inc;
    }
    return hash;
}


Params Setup(mpz_class &alpha, int n, int tm) {
    initState(state_gmp);
    Params pp;
    pp.n = n;
    pp.tm = tm;
    BIG q;
    BIG_rcopy(q, CURVE_Order);
    pp.q = BIG_to_mpz(q);
    ECP2_generator(&pp.P2);
    pp.g = rand_mpz(state_gmp);
    alpha = rand_mpz(state_gmp);
    pp.beta = pow_mpz(pp.g, alpha, pp.q);
    return pp;
}


vector<ECP2> getPK(vector<mpz_class> b) {
    vector<ECP2> PK;
    ECP2 A;
    ECP2_generator(&A);
    int i = 0;
    do {
        ECP2_mul(A, b[i]);
        PK.push_back(A);
    } while (++i < b.size());
    return PK;
}


UAV getUAV(Params pp, vector<mpz_class> d, vector<mpz_class> b, mpz_class id) {

    UAV uav;
    mpz_class fij;
    for (int i = 0; i < b.size(); ++i) {
        fij = ((d[i] * id) + b[i]) % pp.q;// 这里得计算没有问题
        // ElGamal加密
        mpz_class c1, u, beta_u;
        u = rand_mpz(state_gmp);
        c1 = pow_mpz(pp.g, u, pp.q);
        beta_u = pow_mpz(pp.beta, u, pp.q);
        beta_u = (beta_u * fij) % pp.q;
        uav.c1.push_back(c1);
        uav.c2.push_back(beta_u);
    }
    uav.ID = id;
    return uav;
}


vector<UAV> KeyGen(Params &params, mpz_class alpha, UAV_h &uavH) {
    // 1. 初始化tm个随机数
    vector<mpz_class> b, d;
    int tm = params.tm;
    for (int i = 0; i < tm - 1; ++i) { // 阈值 tm 需要 tm-1 次多项式
        b.push_back(rand_mpz(state_gmp));
        d.push_back(rand_mpz(state_gmp));
    }

    // 2. 初始化tm个ECP2，作为系统主公钥
    params.PK = getPK(b);

    // 3. 假设UAVj的身份为IDj,首先初始化它们的身份ID,求每个无人机UAVj对应的"份额重构密钥并求得它的密文形式K[i]，最终得到所有UAV
    vector<UAV> UAVs;
    for (int j = 0; j < params.n; ++j) {
        UAVs.push_back(getUAV(params, d, b, rand_mpz(state_gmp)));
    }

    // 4. 初始化簇头UAV_h ,不失一般性，这里直接将第一个无人机作为簇头
    uavH.alpha = alpha;
    uavH.ID = rand_mpz(state_gmp);

    return UAVs;
}

parSig Sign(Params pp, UAV uav, int t, mpz_class M) {

    // 1. 无人机Uj利用自己的"份额重构密钥"计算阈值 t 对应的份额 (cj,sj)
    mpz_class cj = 1, sj = 1;
    for (int i = 0; i < t - 1; ++i) { // t-1 次多项式
        cj = (cj * uav.c1[i]) % pp.q;
        sj = (sj * uav.c2[i]) % pp.q;
    }

    // 2. 计算BLS部分阈值签名
    ECP Hm = hashToPoint(M, pp.q);

    ECP sigma;
    ECP_copy(&sigma, &Hm);
    ECP_mul(sigma, sj);//si*H(m) 没问题

    // 3. 返回结果
    parSig res;
    res.ID = uav.ID;
    res.cj = cj;
    ECP_copy(&res.sig, &sigma);

    return res;
}


vector<parSig> collectSig(Params pp, vector<UAV> UAVs, int t, mpz_class M) {
    vector<parSig> sigmas;
    parSig sigma;
    for (int i = 0; i < t; ++i) {
        sigma = Sign(pp, UAVs[i], t, M);
        sigmas.push_back(sigma);
    }
    return sigmas;
}

Sigma AggSig(vector<parSig> parSigs, Params pp, UAV_h uavH, mpz_class PK_v) {
    initState(state_gmp);
    Sigma sigma;
    // 4. 簇头使用自己的重加密密钥对阈值签名进行转化，得到接收者能够验证的阈值签名
    // 4.2 根据接收方的公钥生成相对应的重加密密钥 rk = alpha / H(PK_v^alpha)
    mpz_class rk = pow_mpz(PK_v, uavH.alpha, pp.q);
    mpz_class lambda_q = pp.q - 1;
    vector<mpz_class> factors = getFactors();
    rk = hashToCoprime(rk, lambda_q, factors);
    rk = invert_mpz(rk, lambda_q);
    rk = (uavH.alpha * rk) % lambda_q;// 这里不能对 q 取模，否则将会导致后面的验证不通过。

    // 4.3 使用重加密密钥对每个无人机生成的密文进行处理
    mpz_class e = rand_mpz(state_gmp);
    mpz_class ge = pow_mpz(pp.g, e, pp.q);
    mpz_class beta_e = pow_mpz(pp.beta, e, pp.q);
    mpz_class aux_i;
    ECP sig_i;
    for (int i = 0; i < parSigs.size(); ++i) {
        // 重加密辅助解密数据, aux = (cj * g^e)^rk
        aux_i = (parSigs[i].cj * ge) % pp.q;
        aux_i = pow_mpz(aux_i, rk, pp.q);
        sigma.aux.push_back(aux_i);

        // 重加密签名 σ = σ * β^e
        ECP_copy(&sig_i, &parSigs[i].sig);
        ECP_mul(sig_i, beta_e);
        sigma.sig.push_back(sig_i);
        sigma.IDs.push_back(parSigs[i].ID);
    }

    return sigma;
}


vector<mpz_class> getPi_0(Params pp, vector<mpz_class> ID, int t) {
    vector<mpz_class> Pis;

    // 1. 预计算 prodX = x1 * x2 * ... * xt，减少开销
    mpz_class prodX = 1;
    for (int i = 0; i < t; ++i) {
        prodX = (prodX * ID[i]) % pp.q;
    }
    mpz_class denominator, numerator;
    // 2. 生成多项式 pk(x) 的 Pi(0)
    for (int i = 0; i < t; i++) {
        denominator = 1;
        for (int j = 0; j < t; j++) {
            if (j != i) {
                // 分母部分: Π (ID_j - ID_i)
                mpz_class temp = ((ID[j] - ID[i]) + pp.q) % pp.q;
                denominator = (denominator * temp) % pp.q;
            }
        }
        // 从分子中除去当前 UAV[i].ID
        numerator = (prodX * invert_mpz(ID[i], pp.q)) % pp.q;

        // 计算 Pi(0) 并存储
        mpz_class inv_den = invert_mpz(denominator, pp.q); // 求分母的逆元
        mpz_class Pi_0 = (numerator * inv_den) % pp.q;
        Pis.push_back(Pi_0);
    }
    return Pis;
}

vector<mpz_class> getPi_0(Params pp, Sigma UAVs, int t) {
    vector<mpz_class> Pis;

    // 生成多项式 pk(x) 的 Pi(0)
    for (int i = 0; i < t; i++) {
        // 初始化分子和分母
        mpz_class numerator = 1;
        mpz_class denominator = 1;

        // 计算分子和分母
        for (int j = 0; j < t; j++) {
            if (j != i) {
                // 分子部分: Π (0 - ID_j) mod q，相当于 -ID_j mod q
                mpz_class temp_numerator = (-UAVs.IDs[j] + pp.q) % pp.q;
                numerator = (numerator * temp_numerator) % pp.q;

                // 分母部分: Π (ID_i - ID_j) mod q
                mpz_class temp_denominator = (UAVs.IDs[i] - UAVs.IDs[j] + pp.q) % pp.q;
                denominator = (denominator * temp_denominator) % pp.q;
            }
        }

        // 计算 Pi(0) 并存储
        mpz_class inv_den = invert_mpz(denominator, pp.q); // 求分母的逆元
        mpz_class Pi_0 = (numerator * inv_den) % pp.q;
        Pis.push_back(Pi_0);
    }

    return Pis;
}


int Verify(Sigma sigma, mpz_class sk_v, Params pp, mpz_class M, int t) {

    // 1. 求Pi(0) (i=0,1,...,t-1)
    vector<mpz_class> Pis = getPi_0(pp, sigma.IDs, t);

    // 2 计算 H(beta^sk_v)
    mpz_class temp, hash;
    temp = pow_mpz(pp.beta, sk_v, pp.q);
    hash = hashToCoprime(temp, pp.q - 1, getFactors());
    // 3. 计算ki
    vector<mpz_class> k;
    for (int i = 0; i < t; ++i) {
        temp = pow_mpz(sigma.aux[i], hash, pp.q);
        temp = invert_mpz(temp, pp.q);
        temp = (Pis[i] * temp) % pp.q;
        k.push_back(temp);
    }
    // 3. 计算签名s
    ECP s;
    ECP_copy(&s, &sigma.sig[0]);
    ECP_mul(s, k[0]);
    // 3.2 计算累加后的s
    for (int i = 1; i < t; ++i) { // 这里t-1次多项式，所以t个点
        ECP_mul(sigma.sig[i], k[i]);
        ECP_add(&s, &sigma.sig[i]);
    }
    // 4. 验证签名
    FP12 left = e(s, pp.P2);
    ECP Hm = hashToPoint(M, pp.q);
    FP12 right = e(Hm, pp.PK[t - 2]);// 这里验证使用t-1次多项式对应的公钥，由于PK下标从0开始，因此是PK[t-2]
    return FP12_equals(&left, &right);
}


int fussion() {
    initState(state_gmp);
    initRNG(&rng);
    // 初始化参数
    mpz_class alpha, M = 123456789;
    int n = 2, tm = 2;
    // setup
    Params pp = Setup(alpha, n, tm);
    UAV_h uavH;
    // KeyGen
    vector<UAV> UAVs = KeyGen(pp, alpha, uavH);
    // collect signatures
    int t = tm / 2 + 1;
    vector<parSig> sigmas = collectSig(pp, UAVs, t, M);
    // AggSig
    mpz_class sk_v = rand_mpz(state_gmp);
    mpz_class PK_v = pow_mpz(pp.g, sk_v, pp.q);

    Sigma sig = AggSig(sigmas, pp, uavH, PK_v);
    // Verify
    int pass = Verify(sig, sk_v, pp, M, t);

//    cout << "验证结果：" << pass << endl;
    return pass;
}

#include <unistd.h>

void test() {
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        if (fussion() != 1) {
            cout << "本次验证失败 sum=" << sum++ << "  &&  i=" << i << "  &&  seed=" << seed << endl;

//            // 遍历 map
//            std::cout << "All fruits:" << std::endl;
//            for (const auto &pair : numbers) {
//                mpz_class value = pair.second;
//                gmp_printf("%Zx", value.get_mpz_t());
//                cout << "\t\t:" << pair.first  << endl;
//            }
//            cout << endl;
            sleep(1);
        }
    }
    cout << "失败次数：" << sum << endl;

}


//int main() {
////    test();
//    cout << fussion() << endl;
//    return 0;
//}

