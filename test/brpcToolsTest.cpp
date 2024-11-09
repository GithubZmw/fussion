#include "brpcTools.h"

gmp_randstate_t state_brpc;

void testECP() {
    bool success = true;
    initState(state_brpc);
    ECP ecp2;
    ECP_generator(&ecp2);
    for (int i = 0; i < 1000; ++i) {
        mpz_class r = rand_mpz(state_brpc);
        ECP_mul(ecp2, r);
        string ecpStr = ECP_to_str(ecp2);
        ECP ecp1 = str_to_ECP(ecpStr);
        if (ECP_equals(&ecp2, &ecp1) == 0) {
            ECP_output(&ecp2);
            ECP_output(&ecp1);
            success = false;
        }
    }
    string res = success ? "success" : "failed";
    cout << "test finished -> " << res << endl;
}


void testECP2() {
    bool success = true;
    initState(state_brpc);
    ECP2 ecp2;
    ECP2_generator(&ecp2);
    for (int i = 0; i < 1000; ++i) {
        mpz_class r = rand_mpz(state_brpc);
        ECP2_mul(ecp2, r);
        bool compress = (i % 2 == 0);
        string ecpStr = ECP2_to_str(ecp2, compress);
        ECP2 ecp1 = str_to_ECP2(ecpStr);
        if (ECP2_equals(&ecp2, &ecp1) == 0) {
            ECP2_output(&ecp2);
            ECP2_output(&ecp1);
            success = false;
        }
    }
    string res = success ? "success" : "failed";
    cout << "test finished -> " << res << endl;
}


void testFP12() {
    bool success = true;
    initState(state_brpc);
    ECP P1;
    ECP_generator(&P1);
    ECP2 P2;
    ECP2_generator(&P2);
    FP12 fp12 = e(P1, P2);
    for (int i = 0; i < 1000; ++i) {
        mpz_class r = rand_mpz(state_brpc);
        ECP_mul(P1, r);
        ECP2_mul(P2, r);
        fp12 = e(P1, P2);
        string ecpStr = FP12_to_str(fp12);
        FP12 fp121 = str_to_FP12(ecpStr);
        if (FP12_equals(&fp12, &fp121) == 0) {
            showFP12(fp12);
            showFP12(fp121);
            success = false;
        }
    }
    string res = success ? "success" : "failed";
    cout << "test finished -> " << res << endl;
}

void testStrAndMpzArr() {
    bool success = true;
    initState(state_brpc);
    vector<mpz_class> arr = {1, 2, 3, 4, 5};
    for (int k = 0; k < 1000; k++) {
        for (int i = 0; i < arr.size(); ++i) {
            arr[i] = rand_mpz(state_brpc);
        }
        string str = mpzArr_to_str(arr);
        vector<mpz_class> arr1 = str_to_mpzArr(str);
        for (int i = 0; i < arr1.size(); ++i) {
            if (arr[i] != arr1[i]) {
                success = false;
                printLine("出现错误");
                show_mpz(arr[i].get_mpz_t());
                show_mpz(arr1[i].get_mpz_t());
            }
        }
    }
    string res = success ? "success" : "failed";
    cout << "test finished -> " << res << endl;
}

void testStrAndECPArr() {
    bool success = true;
    initState(state_brpc);
    vector<ECP> arr;
    ECP P;
    ECP_generator(&P);
    for (int k = 0; k < 1000; k++) {
        for (int i = 0; i < 5; ++i) {
            mpz_class r = rand_mpz(state_brpc);
            ECP_mul(P, r);
            arr.push_back(P);
        }
        string str = ECPArr_to_str(arr);
        vector<ECP> arr1 = str_to_ECPArr(str);
        for (int i = 0; i < arr.size(); ++i) {
            if (!ECP_equals(&arr[i],&arr1[i])) {
                success = false;
                printLine("出现错误");
                ECP_output(&arr[i]);
                ECP_output(&arr1[i]);
            }
        }
    }
    string res = success ? "success" : "failed";
    cout << "test finished -> " << res << endl;
}

void testStrAndECP2Arr() {
    bool success = true;
    initState(state_brpc);
    vector<ECP2> arr;
    ECP2 P2;
    ECP2_generator(&P2);
    for (int k = 0; k < 1; k++) {
        for (int i = 0; i < 5; ++i) {
            mpz_class r = rand_mpz(state_brpc);
            ECP2_mul(P2, r);
            arr.push_back(P2);
        }
        string str = ECP2Arr_to_str(arr);
        vector<ECP2> arr1 = str_to_ECP2Arr(str);
        for (int i = 0; i < arr.size(); ++i) {
            if (!ECP2_equals(&arr[i],&arr1[i])) {
                success = false;
                printLine("出现错误");
                ECP2_output(&arr[i]);
                ECP2_output(&arr1[i]);
            }
        }
    }
    string res = success ? "success" : "failed";
    cout << "test finished -> " << res << endl;
}


//int main() {
//    testECP();
//    testECP2();
//    testFP12();
//    testStrAndMpzArr();
//    testStrAndECPArr();
//    testStrAndECP2Arr();
//    return 0;
//}

