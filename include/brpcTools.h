#include "Tools.h"

/**
 * 将 mpz_class 转换为 std::string
 * @param value 待转化的mpz_class
 * @return 转化后的字符串
 */
std::string mpz_to_str(const mpz_class& value) ;

/**
 * 将字符串转化为mpz_class
 * @param str 待转化的字符串
 * @return 转化后的mpz_class
 */
mpz_class str_to_mpz(const string& str);
/**
 * 将 ECP 点转换为 std::string
 * @param ecp 待转化的 ECP 点
 * @return 转化后的字符串
 */
std::string ECP_to_str(ECP ecp);

/**
 * 将字符串转化为 ECP 点
 * @param str  待转化的字符串
 * @return 转化后的 ECP 点
 */
ECP str_to_ECP(const std::string& str);

/**
 * 将 ECP2 点转换为 std::string
 * @param ecp2  待转化的 ECP2 点
 * @param compressed 是否压缩
 * @return 转化后的字符串
 */
std::string ECP2_to_str(ECP2 ecp2, bool compressed = true);
/**
 * 将字符串转化为 ECP2 点
 * @param hex_string 待转化的字符串
 * @return  转化后的 ECP2 点
 */
ECP2 str_to_ECP2(const std::string& hex_string) ;
/**
 * 将 FP12 转换为 std::string
 * @param fp12 待转化的 FP12
 * @return 转化后的字符串
 */
std::string FP12_to_str(FP12 fp12);
/**
 * 将字符串转化为 FP12 * * * *
 * @param hex_string 待转化的字符串
 * @return 转化后的 FP12
 */
FP12 str_to_FP12(const std::string& hex_string);

string mpzArr_to_str(vector<mpz_class> mpzs);
vector<mpz_class> str_to_mpzArr(string str);
string ECPArr_to_str(vector<ECP> ecps);
vector<ECP> str_to_ECPArr(string str);
std::string ECP2Arr_to_str(vector<ECP2> ecp2s, bool compressed = true);
vector<ECP2> str_to_ECP2Arr(const std::string& str);