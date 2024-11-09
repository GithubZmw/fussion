#ifndef TOOLS_H
#define TOOLS_H
#include <iostream>
#include <pair_BLS12381.h>
#include <bls_BLS12381.h>
#include <randapi.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>
#include <sys/time.h>
#include "gmpxx.h"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <random>


using namespace B384_58;
using namespace BLS12381;
using namespace core;
using namespace std;
using namespace chrono;

/**
 * 初始化随机种子，用于生成随机数
 * @param rng 待随机化的种子
 */
void initRNG(csprng *rng);
/**
 * 生成一个Fq有限域上的256bit的随机数，其中q表示BLS12381曲线的阶.
 * 注意：执行这个操作之前必须先执行initRNG函数去初始化随机种子
 * @param big 生成的随机数会赋值给这个变量
 * @param rng 生成随机数使用的随机种子，它需要先被初始化（使用initRNG）
 */
void randBig(BIG big,csprng &rng);
/**
 * 生成一个随机的BLS12381曲线上，G1群上的点
 * @param ecp 生成的G1结果
 */
ECP randECP(csprng &rng);
/**
 * 生成一个随机的BLS12381曲线上，G2群上的点
 * @param ecp 生成的G2结果
 */
ECP2 randECP2(csprng &rng);
/**
 * 将一个char*类型的数据转换成string类型
 * @param ch 待转换的char*类型数据
 * @return 返回转换后的string类型数据
 */
string charsToString(char *ch);
/**
 * 将一个BIG类型的大整数转换成mpz_class类型的大整数
 * @param big 待转换的BIG类型大整数
 * @return 返回转换后的mpz_class类型大整数
 */
mpz_class BIG_to_mpz(BIG big);
/**
 * 将一个mpz_class类型的大整数转换成BIG类型的大整数
 * @param t 待转换的mpz_class类型大整数
 * @param big 转换后的BIG类型大整数
 */
void mpz_to_BIG(const mpz_class& t ,BIG& big);
/**
 * 将一个string类型的字符串转换成mpz_class类型的大整数
 * @param hex_string 待转换的字符串
 * @param big 返回转换后的BIG类型大整数
 */
void str_to_BIG(string hex_string, BIG &big);
/**
 * 椭圆曲线上的乘法，这里的常数使用mpz_class
 * @param P1 椭圆曲线上的点
 * @param t 要乘的数
 */
void ECP_mul(ECP& P1, const mpz_class& t);
/**
 * 椭圆曲线上的乘法，这里的常数使用BIG
 * @param P2 椭圆曲线上的点
 * @param t 要乘的数
 */
void ECP2_mul(ECP2& P2, const mpz_class& t);
/**
 * 初始化gmp库中的随机数生成器
 * @param state 需要初始化的随机数生成器
 */
void initState(gmp_randstate_t& state);
/**
 * 生成一个随机的mpz_class类型的大整数
 * @param bits 指定所生成大整数的bit位数
 * @param state 随机数发生器
 * @return 返回一个随机的mpz_class类型的大整数
 */
mpz_class rand_mpz(gmp_randstate_t state);
/**
 * 计算一个 base^exp % mod 的值，结果存储在res中
 * @param base 底数
 * @param exp 指数
 * @param mod 模数
 * @return 返回大整数模幂的结果
 */
mpz_class pow_mpz(const mpz_class& base, const mpz_class& exp, const mpz_class& mod);
/**
 * 计算一个 a 在模 m 下的乘法逆元，结果存储在 res 中
 * @param a 求逆元的大整数
 * @param m 模数
 * @return 返回大整数模乘法逆元的结果，如果a在模m下没有乘法逆元，则返回0
 */
mpz_class invert_mpz(const mpz_class& a, const mpz_class& m);
/**
 * 以十六进制输出mpz_t类型的大整数，包含一个换行 *
 * @param mpz 带查看的大整数
 */
void show_mpz(mpz_t mpz);
/**
 * 将一个octet哈希成一个256bit的大整数，函数hashToZp256需要用到这个函数
 * @param num 哈希结果
 * @param ct 被哈希的octet
 * @param q 哈希结果需要模上的椭圆曲线的阶
 */
void hashZp256(BIG res, octet *ct,BIG q);
/**
 * 将一个BIG类型的大整数哈希成一个256bit的大整数
 * @param res 哈希结果
 * @param beHashed 被哈希的大整数
 * @param q 将哈希结果模上椭圆曲线的阶
 */
void hashToZp256(BIG res, BIG beHashed, BIG q);
/**
 * 将一个mpz_class类型的大整数哈希成一个256bit的大整数
 * @param res 哈希结果
 * @param beHashed 被哈希的大整数
 * @param q 将哈希结果模上椭圆曲线的阶
 */
mpz_class hashToZp256(mpz_class beHashed, mpz_class q);
/**
 * 将一个256bit的大整数哈希成一个G1群上的点
 * @param big 需要哈希的数
 * @param q 哈希结果需要模上的椭圆曲线的阶
 */
ECP hashToPoint(BIG big,BIG q);
/**
 * 将一个256bit的大整数哈希成一个G2群上的点
 * @param big 需要哈希的数
 * @param q 哈希结果需要模上的椭圆曲线的阶
 */
ECP hashToPoint(mpz_class big,mpz_class q);
/**
 * 双线性映射
 * @param alpha1 G1上的元素
 * @param alpha2 G2上的元素
 * @return 返回双线性映射的结果GT上的元素
 */
FP12 e(ECP P1, ECP2 P2);
/**
 * 求一个大整数a在模m下的乘法逆元，结果存储在res中
 * @param res 存储乘法逆元
 * @param a 求逆元的大整数
 * @param m 模数
 */
void BIG_inv(BIG &res, const BIG a,const BIG m);
/**
 * 以十六进制输出BIG类型的大整数，包含一个换行
 * @param big 待查看的大整数
 */
void showBIG(BIG big);
/**
 * 以十六进制输出FP12类型的BLS12381椭圆曲线GT上点，包含一个换行
 * @param big 待查看的GT上的元素
 */
void showFP12(FP12 fp12);
/**
 * 输出一个分割线，调试时候使用
 * @param text 分割线上的文字内容
 */
void printLine(const string& text);


#endif