#include "main.h"
#define clear 1

int main1()
{
    if (clear)
    {
        ofstream f1("20221103.txt", ios::out);
        f1.close();
        ofstream f2("20221104.txt", ios::out);
        f2.close();
    }
    time_t now_time = time(NULL);
    string text_name = to_string(now_time) + ".txt";
    //目标
    auto T = initizeT(30);
    //雷达
    auto L = initizeL(4);
    //指控
    auto Z = initizeZ(4);
    //火力
    auto H = initizeH(8);
    for (int i = 1; i < 10; i++)
    {
        for (int j = 1; j < 10 - i; j++)
        {
            for (int k = 1; k < 10 - i - j; k++)
            {
                for (int l = 1; l < 10 - i - j - k; l++)
                {
                    int m = 10 - i - j - k - l;
                    vector<double> w = {(double)i, (double)j, (double)k, (double)l, (double)m};
                    LinkCalculation wbx(T, L, Z, H);
                    double ans = Jc(wbx, w);
                    ofstream f1(text_name, ios::app);
                    f1 << w[0] << " " << w[1] << " " << w[2] << " " << w[3] << " " << w[4] << " ";
                    f1 << ans << endl;
                    f1.close();
                }
            }
        }
    }
    return 0;
}

// 撤链的逻辑没写 why
// 雷达位置 x,y,z
// prob
// 没测过
// 筹划
// 小问题
int main()
{
    for (int i = 0; i < 1; i++)
        main1();
    return 0;
}
