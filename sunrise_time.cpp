/************************************************
 * 日の出の時刻を求めるプログラム．
 * "hoge.exe YEAR MONTH DAY LONGITUDE LATITUDE HEIGHT TIME_DIFF"と実行することでYEAR年MONTH月DAY日の経度LONGITUDE緯度LATITUDEの高度HEIGHTの地点の太陽の日の出の時刻を(UTC+TIME_DIFF)の時刻で取得することができる．
 * 基本的な計算方法は，長沢工「日の出・日の入りの計算」地人書館
 *************************************************/

#include <iostream>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <string>

using namespace std;

#define PI 3.141592653589793238 // 円周率の定義

double UTC2JD(tm *UTC_time) // 協定世界時からユリウス通日へ変換する関数．数式は長谷川一郎著「天文計算入門-球面三角から軌道計算まで-」による．入力引数はstruct tmへのポインタ型で，変換したいグレゴリオ暦での日付を入力する．
{
    double JD; // ユリウス通日

    JD = UTC_time->tm_mday + 1721088.5 + (double)UTC_time->tm_hour / 24 + (double)UTC_time->tm_min / 1440 + (double)UTC_time->tm_sec / 86400;
    if (UTC_time->tm_mon == 0)
    {
        JD += floor(((double)UTC_time->tm_year + 1899.0) * 365.25) + floor(((double)UTC_time->tm_year + 1899.0) / 400.0) - floor(((double)UTC_time->tm_year + 1899.0) / 100.0) + floor(30.59 * 11.0);
    }
    else if (UTC_time->tm_mon == 1)
    {
        JD += floor(((double)UTC_time->tm_year + 1899.0) * 365.25) + floor(((double)UTC_time->tm_year + 1899.0) / 400.0) - floor(((double)UTC_time->tm_year + 1899.0) / 100.0) + floor(30.59 * 12.0);
    }
    else
    {
        JD += floor(((double)UTC_time->tm_year + 1900.0) * 365.25) + floor(((double)UTC_time->tm_year + 1900.0) / 400.0) - floor(((double)UTC_time->tm_year + 1900.0) / 100.0) + floor(30.59 * (UTC_time->tm_mon - 1));
    }

    return JD;
}

void JD2UTC(tm *UTC_time, double JD) // ユリウス通日から協定世界時へ変換する関数．数式は長谷川一郎著「天文計算入門-球面三角から軌道計算まで-」による．入力引数はstruct tmへのポインタ型（変換したグレゴリオ暦での日付を格納する）と変換したいユリウス通日．

{
    double JD_dec; // ユリウス通日の小数部

    double A, B, C, E, F, G, H;
    int D, M, Y;

    A = floor(JD + 68569.5);
    B = floor(A / 36524.25);
    C = A - floor(36524.25 * B + 0.75);
    E = floor((C + 1.0) / 365.25025);
    F = C - floor(365.25 * E) + 31.0;
    G = floor(F / 30.59);
    H = floor(G / 11.0);
    D = F - floor(30.59 * G);
    M = G - 12.0 * H + 2.0;
    Y = 100.0 * (B - 49.0) + E + H;

    if (M == 12 && D == 32)
    {
        Y += 1;
        M = 1;
        D = 1;
    }

    // 結果を構造体tm型の"UTC_time"に格納

    UTC_time->tm_year = Y - 1900;
    UTC_time->tm_mon = M - 1;
    UTC_time->tm_mday = D;

    JD_dec = JD + 0.5 - floor(JD + 0.5);
    UTC_time->tm_hour = floor(JD_dec * 24);
    JD_dec = JD_dec * 24 - floor(JD_dec * 24);
    UTC_time->tm_min = floor(JD_dec * 60);
    JD_dec = JD_dec * 60 - floor(JD_dec * 60);
    UTC_time->tm_sec = floor(JD_dec * 60);
}

double JD2TT(double JD) // ユリウス通日から地球時（TT）に変換する関数．入力引数はユリウス通日．地球時（TT）をdoubleで返す．
{
    double JC;          // ユリウス世紀
    double diff_TT_UTC; // UTCとTTの差
    double TT;          // TT（ユリウス世紀数）

    JC = (JD - 2451545) / 36525;

    // UTCとTTの差を求める．数式の参考文献は松本真一,「海上保安庁海洋情報部の式の援用による太陽視赤経と均時差の計算について」, 空気調和・衛生工学会大会学術講演論文集, G-5, 第5巻, 2019
    diff_TT_UTC = 45.50428 / (1 + 0.4729259 * exp(-3.913042 * JC)) + 32.184 + 86400 / (6.969290134 * pow(10, 10)) * (36525 * JC + 8611.9996275);
    TT = JC + diff_TT_UTC / (86400.0 * 365.0 * 100.0);
    return TT;
}

double JD2right_ascension(double JD) // ユリウス通日から太陽視赤経を求める関数．入力引数はユリウス通日．太陽視赤経をdeg単位でdoubleで返す．
{
    double right_ascension; // 太陽視赤経
    double TT;              // 地球時（ユリウス世紀数）

    TT = JD2TT(JD);

    // TTから太陽視赤経を求める．数式の参考文献は松本真一, 「太陽視赤緯と均時差計算に関する一考察」, 日本建築学会東北支部研究報告集, 第68号, 2005, pp.89-96
    right_ascension = 18.69735 + 0.16419 * cos((72001.539 * TT + 290.92) * PI / 180) + 0.12764 * cos((35999.050 * TT + 267.52) * PI / 180) + 0.00549 * cos((36002.500 * TT + 113.40) * PI / 180) + 0.00549 * cos((108000.600 * TT + 288.50) * PI / 180) + 0.00353 * cos((144003.100 * TT + 311.90) * PI / 180) + 0.00133 * cos((71998.100 * TT + 265.10) * PI / 180) + 0.00032 * cos((1934.100 * TT + 145.00) * PI / 180) + 0.00024 * cos((108004.000 * TT + 134.00) * PI / 180) + 0.00024 * cos((180002.000 * TT + 309.00) * PI / 180) + 0.00015 * cos((144000.000 * TT + 286.00) * PI / 180) + 0.00013 * cos((32964.000 * TT + 158.00) * PI / 180) + 0.00012 * cos((445267.000 * TT + 208.00) * PI / 180) + 0.00012 * cos((19.000 * TT + 159.00) * PI / 180) + 0.00010 * cos((45038.000 * TT + 254.00) * PI / 180) + 0.00010 * cos((216005.000 * TT + 333.00) * PI / 180) + 0.00009 * cos((22519.000 * TT + 352.00) * PI / 180) + 0.00005 * cos((9038.000 * TT + 64.00) * PI / 180) + 0.00005 * cos((65929.000 * TT + 45.00) * PI / 180) + 0.00005 * cos((3035.000 * TT + 110.00) * PI / 180) + 0.00004 * cos((33718.000 * TT + 316.00) * PI / 180) + 0.00003 * cos((155.000 * TT + 118.00) * PI / 180) + 0.00003 * cos((73936.000 * TT + 166.00) * PI / 180) + 0.00003 * cos((3.000 * TT + 296.00) * PI / 180) + 0.00003 * cos((29930.000 * TT + 48.0) * PI / 180) + 0.00003 * cos((2281.000 * TT + 221.00) * PI / 180) + 0.00003 * cos((31557.000 * TT + 161.00) * PI / 180) + 2400.05130 * TT - 0.00032 * TT * cos((35999.000 * TT + 268.00) * PI / 180) - 0.00019 * TT * cos((72002.000 * TT + 291.00) * PI / 180) - 0.00002 * TT * cos((108001.000 * TT + 289.00) * PI / 180) - 0.00002 * TT * cos((36003.000 * TT + 113.00) * PI / 180);
    right_ascension = right_ascension / 24 * 360;

    // 計算上，0<太陽視赤経<360にしたいので，一周回る部分をカット．

    if (right_ascension >= 360)
    {
        while (right_ascension >= 360)
        {
            right_ascension -= 360;
        }
    }

    if (right_ascension < 0)
    {
        while (right_ascension > 0)
        {
            right_ascension += 360;
        }
    }

    return right_ascension;
}

double JD2declination(double JD) // ユリウス通日から太陽視赤緯を求める関数．入力引数はユリウス通日．太陽視赤緯をdeg単位でdoubleで返す．
{
    double declination; // 太陽視赤緯
    double TT;          // 地球時（ユリウス世紀数）

    TT = JD2TT(JD);

    // TTから太陽視赤緯を求める．数式の参考文献は松本真一, 「太陽視赤緯と均時差計算に関する一考察」, 日本建築学会東北支部研究報告集, 第68号, 2005, pp.89-96
    declination = 23.2643 * cos((36000.7696 * TT + 190.4602) * PI / 180) + 0.3888 * cos((1.7200 * TT + 12.9400) * PI / 180) + 0.3886 * cos((71999.8200 * TT + 187.9900) * PI / 180) + 0.1646 * cos((108002.3000 * TT + 211.4000) * PI / 180) + 0.0082 * cos((72003.0000 * TT + 34.0000) * PI / 180) + 0.0082 * cos((144001.0000 * TT + 209.0000) * PI / 180) + 0.0073 * cos((107999.0000 * TT + 186.0000) * PI / 180) + 0.0031 * cos((180004.0000 * TT + 232.0000) * PI / 180) + 0.0022 * cos((37935.0000 * TT + 65.0000) * PI / 180) + 0.0008 * cos((35997.0000 * TT + 345.0000) * PI / 180) + 0.0004 * cos((68965.0000 * TT + 78.0000) * PI / 180) + 0.0004 * cos((3036.0000 * TT + 123.0000) * PI / 180) + 0.0003 * cos((481268.0000 * TT + 128.0000) * PI / 180) + 0.0003 * cos((35982.0000 * TT + 121.0000) * PI / 180) + 0.0003 * cos((36020.0000 * TT + 80.0000) * PI / 180) + 0.0003 * cos((409266.0000 * TT + 287.0000) * PI / 180) + 0.0003 * cos((13482.0000 * TT + 293.0000) * PI / 180) + 0.0003 * cos((9037.0000 * TT + 332.0000) * PI / 180) + 0.0003 * cos((180000.0000 * TT + 206.0000) * PI / 180) - 0.0127 * TT * cos((36001.0000 * TT + 190.0000) * PI / 180) - 0.0012 * TT * cos((72000.0000 * TT + 188.0000) * PI / 180) - 0.0012 * TT * cos((2.0000 * TT + 13.0000) * PI / 180) - 0.0003 * TT * cos((108002.0000 * TT + 211.0000) * PI / 180);

    return declination;
}

double JD2distance_sun_earth(double JD) // ユリウス通日から太陽・地球間距離を求める関数．入力引数はユリウス通日．太陽地球間距離をAU単位でdoubleで返す．
{
    double r;  // 太陽・地球間距離
    double TT; // 地球時（ユリウス世紀数）

    TT = JD2TT(JD);

    // TTから太陽・地球間距離を求める．数式の参考文献は株式会社気象データシステム「太陽位置の計算」, TE-8J ver.1
    r = 1.000140 + 0.016706 * cos((35999.05 * TT + 177.53) * PI / 180) + 0.000139 * cos((71998.00 * TT + 175.00) * PI / 180) + 0.000031 * cos((445267.00 * TT + 298.00) * PI / 180) + 0.000016 * cos((32964.00 * TT + 68.00) * PI / 180) + 0.000016 * cos((45038.00 * TT + 164.00) * PI / 180) + 0.000005 * cos((22519.00 * TT + 233.00) * PI / 180) + 0.000005 * cos((33718.00 * TT + 226.00) * PI / 180) - 0.000042 * TT * cos((35999.00 * TT + 178.00) * PI / 180);

    return r;
}

double JD2local_sidereal(double JD, double local_longitude) // ユリウス通日から地方恒星時を求める関数．入力引数はユリウス通日と求めたい場所の経度[deg]（東経が+, 西経が-）．地方恒星時をdeg単位でdoubleで返す．
{
    double TJD;                // 世界時1968年5月24日0時からの日数
    double Greenwich_sidereal; // グリニッジ恒星時
    double local_sidereal;     // 地方恒星時

    // 地方恒星時の計算法は https://ja.wikipedia.org/wiki/%E6%81%92%E6%98%9F%E6%99%82（2024/01/12最終閲覧） を参考にした．

    TJD = JD - 2440000.5;

    Greenwich_sidereal = ((0.671262 + 1.0027379094 * TJD) - floor(0.671262 + 1.0027379094 * TJD)) * 360;
    local_sidereal = Greenwich_sidereal + local_longitude;

    // 計算上0<地方恒星時<360にしたいので，一周回る部分をカット．

    if (local_sidereal >= 360)
    {
        while (local_sidereal >= 360)
        {
            local_sidereal -= 360;
        }
    }

    if (local_sidereal < 0)
    {
        while (local_sidereal > 0)
        {
            local_sidereal += 360;
        }
    }

    return local_sidereal;
}

double JD2sunrise_hour_angle(double JD, double local_height, double local_latitude, double distance_sun_earth, double declination) // 日の出時の太陽の時角を求める関数．入力引数はユリウス通日，求めたい場所の高度[m]，求めたい場所の緯度[deg]（北緯が+, 南緯が-），太陽と地球の距離[AU]，太陽視赤緯[deg]．日の出時の太陽の時角をdeg単位でdoubleで返す．
{
    double apparent_horizon;               // 見かけの地平線
    double apparent_radius;                // 太陽の視半径
    double atmospheric_difference;         // 大気差
    double equatorial_horizontal_parallax; // 赤道地平視差
    double sunrise_solar_altitude;         // 日の出時の太陽の出没高度
    double sunrise_hour_angle;             // 日の出時の太陽の時角

    // 日の出時の太陽の時角を求める．数式の参考文献は長沢工「日の出・日の入りの計算」地人書館．

    apparent_horizon = 2.12 / 60.0 * sqrt(local_height);
    apparent_radius = (16.0 / 60.0 + 1.18 / 60.0 / 60.0) / distance_sun_earth;
    atmospheric_difference = 35.0 / 60.0 + 8.0 / 60.0 / 60.0;
    equatorial_horizontal_parallax = (8.794148 / 60.0 / 60.0) / distance_sun_earth;

    sunrise_solar_altitude = -apparent_horizon - apparent_radius - atmospheric_difference + equatorial_horizontal_parallax;
    sunrise_hour_angle = -acos((sin(sunrise_solar_altitude * PI / 180) - sin(declination * PI / 180) * sin(local_latitude * PI / 180)) / (cos(declination * PI / 180) * cos(local_latitude * PI / 180))) * 180 / PI;

    return sunrise_hour_angle;
}

double calc_solar_hour_angle(double local_sidereal, double right_ascension) // 地方恒星時と太陽の視赤経から太陽の時角を求める関数．入力引数は求めたい場所の経度[deg]（東経が+, 西経が-）と太陽視赤経[deg]．太陽の時角をdoubleで返す．
{
    double sun_hour_angle;

    sun_hour_angle = local_sidereal - right_ascension;

    // 計算の都合上，-180<時角<180が望ましいので，一回りする部分をカット．

    if (sun_hour_angle > 180)
    {
        while (sun_hour_angle > 180)
        {
            sun_hour_angle -= 360;
        }
    }

    if (sun_hour_angle < -180)
    {
        while (sun_hour_angle < -180)
        {
            sun_hour_angle += 360;
        }
    }

    return sun_hour_angle;
}

int main(int argc, char *argv[])
{
    double JD; // ユリウス通日

    long calc_year;         // 日の出を求める日付の年
    long calc_mon;          // 日の出を求める日付の月
    long calc_day;          // 日の出を求める日付の日
    double local_longitude; // 観測地点の経度
    double local_latitude;  // 観測地点の緯度
    double local_height;    // 観測地点の高度．単位はm．
    double time_diff;       // 観測地点のUTCとの時差．単位はh.

    double sunrise_hour_angle; // 日の出時の太陽の時角
    double sun_hour_angle;     // ある時刻での太陽の時角

    double declination;        // 太陽視赤緯
    double right_ascension;    // 太陽視赤経
    double distance_sun_earth; // 太陽‐地球間距離．単位はAU
    double local_sidereal;     // 観測地点の恒星時

    double delta_t = 1; // 逐次法で日の出時刻を求めるときのΔt

    // コマンドライン引数を数字へ変換

    calc_year = stol(argv[1]);
    calc_mon = stol(argv[2]);
    calc_day = stol(argv[3]);
    local_longitude = stod(argv[4]);
    local_latitude = stod(argv[5]);
    local_height = stod(argv[6]);
    time_diff = stod(argv[7]);

    // 日付時刻を収める構造体の宣言と初期化
    struct tm initializer;
    struct tm *UTC_time;

    UTC_time = &initializer;

    // 日付時刻を収める構造体へコマンドライン引数で得た数値を代入
    UTC_time->tm_year = calc_year - 1900;
    UTC_time->tm_mon = calc_mon - 1;
    UTC_time->tm_mday = calc_day;
    UTC_time->tm_hour = 6;
    UTC_time->tm_min = 0;
    UTC_time->tm_sec = 0;

    // ユリウス通日の計算．観測地点での朝5時を初期値とするために時差分を引く．
    JD = UTC2JD(UTC_time) - time_diff / 24;

    // 日の出時刻のユリウス通日を求める．逐次計算法の変化量が小さくなるまで繰り返す．詳しい計算方法は長沢工「日の出・日の入りの計算」地人書館．

    while (fabs(delta_t) > 0.00001)
    {
        local_sidereal = JD2local_sidereal(JD, local_longitude);
        distance_sun_earth = JD2distance_sun_earth(JD);
        declination = JD2declination(JD);
        right_ascension = JD2right_ascension(JD);

        sunrise_hour_angle = JD2sunrise_hour_angle(JD, local_height, local_latitude, distance_sun_earth, declination);
        sun_hour_angle = calc_solar_hour_angle(local_sidereal, right_ascension);

        delta_t = (sunrise_hour_angle - sun_hour_angle) / 360;

        JD += delta_t;
    }

    // 計算結果の表示

    JD2UTC(UTC_time, JD + time_diff / 24.0);

    cout << UTC_time->tm_year + 1900 << "\n"
         << UTC_time->tm_mon + 1 << "\n"
         << UTC_time->tm_mday << "\n"
         << UTC_time->tm_hour << "\n"
         << UTC_time->tm_min << "\n"
         << UTC_time->tm_sec << "\n";

    return 0;
}