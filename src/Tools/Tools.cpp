#include "Tools.h"

// 随机种子
//csprng rng;

void initRNG(csprng *rng) {
    char raw[100];
    octet RAW = {0, sizeof(raw), raw};
    unsigned long ran;
    time((time_t *) &ran);

    RAW.len = 100; // fake random seed source
    RAW.val[0] = ran;
    RAW.val[1] = ran >> 8;
    RAW.val[2] = ran >> 16;
    RAW.val[3] = ran >> 24;
    for (int i = 4; i < 100; i++)
        RAW.val[i] = i;

    CREATE_CSPRNG(rng, &RAW);
}


void randBig(BIG big, csprng &rng) {
    BIG mod;
    BIG_rcopy(mod, CURVE_Order);
    BIG_randtrunc(big, mod, 2 * CURVE_SECURITY_BLS12381, &rng);
}

ECP randECP(csprng &rng) {
    ECP ecp;
    ECP_generator(&ecp);
    BIG r;
    randBig(r, rng);
    ECP_mul(&ecp, r);
    return ecp;
}

ECP2 randECP2(csprng &rng) {
    ECP2 ecp2;
    ECP2_generator(&ecp2);
    BIG r;
    randBig(r, rng);
    ECP2_mul(&ecp2, r);
    return ecp2;
}

string charsToString(char *ch) {
    ostringstream oss; // 创建输出字符串流
    for (size_t i = 0; i < 48; ++i) {
        // 将字符转换为 unsigned char，然后打印为十六进制格式
        unsigned char ucharValue = static_cast<unsigned char>(ch[i]);
        oss << hex << setw(2) << setfill('0') << static_cast<int>(ucharValue);
    }
    return oss.str(); // 获取最终的字符串
}

mpz_class BIG_to_mpz(BIG big) {
    char ch[48];
    BIG_toBytes(ch, big);
    mpz_class t;
    t.set_str(charsToString(ch).c_str(), 16);
    return t;
}

void mpz_to_BIG(const mpz_class &t, BIG &big) {
    std::string hexStr = t.get_str(16);

    // 确保 hexStr 长度为 64 个字符（32 字节）
    if (hexStr.length() < 64) {
        hexStr.insert(0, 64 - hexStr.length(), '0'); // 在前部填充 '0'
    }
    // 将 hexStr 中的每两个字符（一个字节）转换成实际字节
    char ch[32] = {0};
    for (size_t i = 0; i < 32; ++i) {
        std::string byteStr = hexStr.substr(2 * i, 2);  // 每两个字符为一个字节
        ch[i] = static_cast<unsigned char>(strtol(byteStr.c_str(), nullptr, 16));
    }
    BIG_fromBytesLen(big, ch, 32);
}

void str_to_BIG(string hex_string, BIG &big) {
    // 确保 hexStr 长度为 96 个字符（48 字节）
    if (hex_string.length() < 96) {
        hex_string.insert(0, 96 - hex_string.length(), '0'); // 在前部填充 '0'
    }
    char byte_array[48];
    for (size_t i = 0; i < 48; i++) {
        sscanf(hex_string.substr(i * 2, 2).c_str(), "%2hhx", &byte_array[i]);
    }
    BIG_fromBytes(big, byte_array);
}


void ECP_mul(ECP &P1, const mpz_class &t) {
    BIG t1;
    mpz_to_BIG(t, t1);
    ECP_mul(&P1, t1);
}

void ECP2_mul(ECP2 &P2, const mpz_class &t) {
    BIG t1;
    mpz_to_BIG(t, t1);
    ECP2_mul(&P2, t1);
}

void initState(gmp_randstate_t &state) {
    gmp_randinit_default(state);// 下面使用当前时间作为种子,精确到纳秒，防止快速运行程序时出错
    gmp_randseed_ui(state, duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count());
}

mpz_class rand_mpz(gmp_randstate_t state) {
    mpz_class res;
    mpz_class max_value = 0x73EDA753299D7D483339D80809A1D80553BDA402FFFE5BFEFFFFFFFF00000001_mpz;  // 设置最大值为 椭圆曲线阶-1
    mpz_urandomm(res.get_mpz_t(), state, max_value.get_mpz_t());
    return res+1;// 随机数生成后，需要+1，防止 res = 0;
}


mpz_class pow_mpz(const mpz_class &base, const mpz_class &exp, const mpz_class &mod) {
    mpz_class res;
    mpz_powm(res.get_mpz_t(), base.get_mpz_t(), exp.get_mpz_t(), mod.get_mpz_t());
    return res;
}

mpz_class invert_mpz(const mpz_class &a, const mpz_class &m) {
    mpz_class res;
    mpz_invert(res.get_mpz_t(), a.get_mpz_t(), m.get_mpz_t());
    return res;
}

void show_mpz(mpz_t mpz) {
    gmp_printf("%Zx", mpz);
    cout << endl;
}


void hashZp256(BIG res, octet *ct, BIG q) {
    hash256 h;
    // 数组长度设为48，由于每一位char用两个十六进制的数字表示【可以表示256个字符，刚好表示ASCII表】，则将其转化为为BIG后，数字长度为96，每个16进制用4个bit表示，故96*4=384
    char hashstr[48];
    memset(hashstr, 0, 48);//逐字节初始化charstr
    // 哈希函数三步走
    HASH256_init(&h);
    for (int j = 0; j < ct->max; j++) {
        HASH256_process(&h, ct->val[j]);
    }
    HASH256_hash(&h, hashstr);
    // 将得到的结果转化为有限域Fq上的元素
    BIG_fromBytesLen(res, hashstr, 48);
    BIG_mod(res, q);
}

void hashToZp256(BIG res, BIG beHashed, BIG q) {
    char idChar[48];
    BIG_toBytes(idChar, beHashed);
    octet id_i_oc;
    id_i_oc.max = 48;
    id_i_oc.val = idChar;
    hashZp256(res, &id_i_oc, q);
}

mpz_class hashToZp256(mpz_class beHashed, mpz_class q) {
    mpz_class res;
    BIG res_b, beHashed_b, module_b;
    mpz_to_BIG(res, res_b);
    mpz_to_BIG(beHashed, beHashed_b);
    mpz_to_BIG(q, module_b);
    hashToZp256(res_b, beHashed_b, module_b);
    return BIG_to_mpz(res_b);
}

ECP hashToPoint(BIG big, BIG q) {
    BIG hash;
    hashToZp256(hash, big, q);
    ECP res;
    ECP_generator(&res);
    ECP_mul(&res, hash);
    return res;
}

ECP hashToPoint(mpz_class big, mpz_class q) {
    BIG tb, tq;
    mpz_to_BIG(big, tb);
    mpz_to_BIG(q, tq);
    return hashToPoint(tb, tq);
}


FP12 e(ECP P1, ECP2 P2) {
    FP12 temp1;
    PAIR_ate(&temp1, &P2, &P1);
    PAIR_fexp(&temp1);
    FP12_reduce(&temp1);
    if (FP12_isunity(&temp1) || FP12_iszilch(&temp1)) {
        printf("pairing error [temp1]\n");
    }
    return temp1;
}


void BIG_inv(BIG &res, const BIG a, const BIG m) {
    BIG m0, x0, x1, one, a_back, module;
    // 此处大费周折复制变量是为了防止求逆元时改变了参数a,m的值
    BIG_rcopy(a_back, a);
    BIG_rcopy(module, m);
    BIG_rcopy(m0, m);
    BIG_one(one);
    // 1. 初始化参数
    BIG_zero(x0);
    BIG_one(x1);
    BIG q, temp;
    while (BIG_comp(a_back, one) > 0) {
        // q 是 a 和 m 的商
        BIG_copy(q, a_back);
        BIG_sdiv(q, module);
        // 更新 a = m ; 并更新 m = a % m;
        BIG_copy(temp, module);
        BIG_copy(module, a_back);
        BIG_mod(module, temp);
        BIG_copy(a_back, temp);
        // 更新 x1 = x0 ; x0 = x1 - q * x0
        BIG_copy(temp, x0);
        BIG_modmul(x0, q, x0, m0);
        BIG_modneg(x0, x0, m0);
        BIG_modadd(x0, x0, x1, m0);
        BIG_copy(x1, temp);
    }
    BIG_copy(res, x1);
}

void showBIG(BIG big) {
    BIG_output(big);
    cout << endl;
}

void showFP12(FP12 fp12) {
    FP12_output(&fp12);
    cout << endl;
}

void printLine(const string &text) {
    cout << "--------------------------------------------------\t " + text +
            " \t--------------------------------------------------" << endl;
}


//int main(){
//    testTools();
//    return 0;
//}