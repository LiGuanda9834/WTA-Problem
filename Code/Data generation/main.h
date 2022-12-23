#include "LinkCalculation.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
typedef priority_queue<Target, vector<Target>, Target_cmp> pqT;
pqT initizeT(int num)
{
    pqT res;
    int threat_min = 0;
    int threat_max = 100;
    int x_min = -120000;
    int x_max = -30000;
    int y_min = 30000;
    int y_max = 300000;
    int z_min = 1000;
    int z_max = 25000;
    int Px_min = -15000;
    int Px_max = 15000;
    int Py_min = -20000;
    int Py_max = 20000;
    int v_min = 250;
    int v_max = 800;

    srand((unsigned)time(NULL));
    for (int i = 1; i <= num; i++)
    {
        double x = (double)((rand() % (x_max - x_min)) + x_min + 1);
        double y = (double)((rand() % (y_max - y_min)) + y_min + 1);
        double z = (double)((rand() % (z_max - z_min)) + z_min + 1);
        double Px = (double)((rand() % (Px_max - Px_min)) + Px_min + 1);
        double Py = (double)((rand() % (Py_max - Py_min)) + Py_min + 1);
        double v = (double)((rand() % (v_max - v_min)) + v_min + 1);
        double PR = sqrt(Px * Px + Py * Py);
        int xp = 5;
        if (PR > 5000)
            xp = 4;
        if (PR > 10000)
            xp = 3;
        if (PR > 15000)
            xp = 2;
        if (PR > 20000)
            xp = 1;
        int xs = 5;
        double R = sqrt(x * x + y * y);
        if (R > 70000)
            xs = 4;
        if (R > 100000)
            xs = 3;
        if (R > 120000)
            xs = 2;
        if (R > 150000)
            xs = 1;
        Px = Px - x;
        Py = Py - y;
        double vx = v * Px / sqrt(Px * Px + Py * Py);
        double vy = v * Py / sqrt(Px * Px + Py * Py);
        double threat = 100 * v / 850 + 20 * xs + 20 * xp;
        threat /= 3;
        // = (double)((rand() % (threat_max - threat_min)) + threat_min + 1);

        vector<double> t = {(double)i, threat, x, y, z, vx, vy, v};
        Target t1(t);
        res.push(t1);
    }
    return res;
}

map<int, leida> initizeL(int num)
{
    map<int, leida> res;
    int x_min = -2000;
    int x_max = 2000;
    int y_min = -2000;
    int y_max = 2000;
    srand((unsigned)time(NULL));
    for (int i = 1; i <= num; i++)
    {
        double x = (double)((rand() % (x_max - x_min)) + x_min + 1);
        double y = (double)((rand() % (y_max - y_min)) + y_min + 1);
        vector<double> t = {(double)i, x, y};
        leida t1(t);
        res[i] = t1;
    }
    return res;
}

map<int, zhikong> initizeZ(int num)
{
    map<int, zhikong> res;
    int x_min = -2000;
    int x_max = 2000;
    int y_min = -2000;
    int y_max = 2000;
    srand((unsigned)time(NULL));
    for (int i = 1; i <= num; i++)
    {
        double x = (double)((rand() % (x_max - x_min)) + x_min + 1);
        double y = (double)((rand() % (y_max - y_min)) + y_min + 1);
        vector<double> t = {(double)i, x, y};
        zhikong t1(t);
        res[i] = t1;
    }
    return res;
}

map<int, huoli> initizeH(int num)
{
    map<int, huoli> res;
    int x_min = -2000;
    int x_max = 2000;
    int y_min = -2000;
    int y_max = 2000;
    srand((unsigned)time(NULL));
    for (int i = 1; i <= num; i++)
    {
        double x = (double)((rand() % (x_max - x_min)) + x_min + 1);
        double y = (double)((rand() % (y_max - y_min)) + y_min + 1);
        vector<double> t = {(double)i, x, y};
        huoli t1(t);
        res[i] = t1;
    }
    return res;
}

double Jc(LinkCalculation &wbx, vector<double> &w)
{
    int times = 0;
    wbx.cur_weight = w;
    ofstream f1("20221103.txt", ios::app);
    f1 << times << "时刻下发杀伤链:";
    f1 << "目标"
       << "-"
       << "雷达"
       << "-"
       << "指控"
       << "-"
       << "火力"
       << "-"
       << "用时" << endl;
    f1.close();
    while (times < 200)
    {

        ofstream f1("20221103.txt", ios::app);
        f1 << times << endl;
        f1.close();
        wbx.issue_time = times;
        wbx.ManageKillLink();
        times += 2;
    }

    for (auto &x : wbx.ssl_history_set)
    {
        wbx.after_cal_ssl_set[x.first] = x.second;
    }
    double jc = 0;
    for (auto &x : wbx.after_cal_ssl_set)
    {
        jc += x.second.t.threat * x.second.prob;
    }
    //假设统计方法为：threat*p
    return jc;
}
