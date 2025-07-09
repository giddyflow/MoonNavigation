
#include "Service.h"

double rad2deg(double rad)
{
    return rad * 180 / M_PI;
}

double deg2rad(double deg)
{
    return deg * M_PI / 180;
}

double norm(BLH point1, BLH point2) {
    XYZ p1 = BLH2ECEF(point1);
    XYZ p2 = BLH2ECEF(point2);

    double dist = sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y) + (p1.z - p2.z) * (p1.z - p2.z));
    return dist;
}

double normrnd(double m, double sko) {
    //std::mt19937 generator;
    std::random_device rd;
    std::mt19937 generator(rd());
    std::normal_distribution<double> dist(m, sko);
    return dist(generator);
}

double pow2db(double x) {
    return 10 * log10(x);
}

double db2pow(double x) {
    return pow(10, (x / 10));
}

double rx_power(double txPower, double freq, double dist) {
    double L_fs = 0.;
    double lambda = EarthConstants::c / freq;
    L_fs = 20 * log10(lambda / (4 * M_PI * dist));
    double P_rx = txPower + L_fs;
    return P_rx;
}

double measure_time_std_by_snr(double q) {
    double df = 2;
    double T = 10e-3;
    double q_cn = db2pow(q);
    double tau_c = 1 / (511e3);
    //double Kn2 = 32 * df * df / 9;
    double D = tau_c * tau_c / (2 * q_cn) * (1 + 2 / (q_cn * T)) * df; // (15.47) GLONASS (Perov)
    return sqrt(D);
}

bool GroundVisability(const BLH point1, const BLH point2) {
    double r_max = sqrt(pow(point1.h + EarthConstants::radius, 2) - pow(EarthConstants::radius, 2)) + sqrt(pow(point2.h + EarthConstants::radius, 2) - pow(EarthConstants::radius, 2));
    auto pos1 = BLH2ECEF(point1);
    auto pos2 = BLH2ECEF(point2);
    auto r = sqrt(pow(pos1.x - pos2.x, 2) + pow(pos1.y - pos2.y, 2) + pow(pos1.z - pos2.z, 2));
    if (r < r_max)
        return(true);
    else
        return(false);
}

XYZ BLH2ECEF(BLH blh) {
    double cosB = cos(deg2rad(blh.lat));
    double sinB = sin(deg2rad(blh.lat));
    double cosL = cos(deg2rad(blh.lon));
    double sinL = sin(deg2rad(blh.lon));
    double H = blh.h;
    double ksi = EarthConstants::a / sqrt(1. - EarthConstants::e2 * sinB * sinB);
    return { (ksi + H) * cosB * cosL, (ksi + H) * cosB * sinL, (ksi * (1 - EarthConstants::e2) + H) * sinB };
}


BLH ECEF2BLH(XYZ ecef) {
    double b, ep2, lon, rho, beta, phi, betaNew, sinphi, N, h;

    b = (1 - EarthConstants::f) * EarthConstants::a;
    ep2 = EarthConstants::e2 / (1 - EarthConstants::e2);
    lon = atan2(ecef.y, ecef.x);
    rho = sqrt(ecef.x * ecef.x + ecef.y * ecef.y);
    beta = atan2(ecef.z, (1 - EarthConstants::f) * rho);
    phi = atan2(ecef.z + b * ep2 * pow(sin(beta), 3), rho - EarthConstants::a * EarthConstants::e2 * pow(cos(beta), 3));
    betaNew = atan2((1 - EarthConstants::f) * sin(phi), cos(phi));
    int count1 = 0;
    while (count1 < 5) {
        beta = betaNew;
        phi = atan2(ecef.z + b * ep2 * pow(sin(beta), 3), rho - EarthConstants::a * EarthConstants::e2 * pow(cos(beta), 3));
        betaNew = atan2((1 - EarthConstants::f) * sin(phi), cos(phi));
        count1++;
    }

    // Ellipsoidal height from final value for latitude
    sinphi = sin(phi);
    N = EarthConstants::a / sqrt(1 - EarthConstants::e2 * pow(sinphi, 2));
    h = rho * cos(phi) + (ecef.z + EarthConstants::e2 * N * sinphi) * sinphi - N;

    return { rad2deg(phi), rad2deg(lon), h };
}

XYZ BLH2ENU(BLH blh_ref, BLH blh) {
    // blh_ref - geo-���������� �������� ������
    // blh - �������� geo-����������

    double cosB = cos(deg2rad(blh_ref.lat));
    double sinB = sin(deg2rad(blh_ref.lat));
    double cosL = cos(deg2rad(blh_ref.lon));
    double sinL = sin(deg2rad(blh_ref.lon));
    XYZ xyz_ref = BLH2ECEF(blh_ref);
    XYZ xyz_post = BLH2ECEF(blh);

    std::array<double, 3> x = { xyz_post.x - xyz_ref.x, xyz_post.y - xyz_ref.y, xyz_post.z - xyz_ref.z };
    std::vector<std::array<double, 3>> R = {
        {-sinL, cosL, 0.},
        {-sinB * cosL, -sinB * sinL, cosB},
        {cosB * cosL, cosB * sinL, sinB}
    };

    double enu00 = R[0][0] * x[0] + R[0][1] * x[1] + R[0][2] * x[2];
    double enu10 = R[1][0] * x[0] + R[1][1] * x[1] + R[1][2] * x[2];
    double enu20 = R[2][0] * x[0] + R[2][1] * x[1] + R[2][2] * x[2];

    return { enu00, enu10, enu20 };
}

XYZ ECEF2ENU(XYZ xyz_ref, XYZ xyz_post) {
    // xyz_ref - geo-���������� �������� ������
    // blh - �������� geo-����������

    BLH blh_ref = ECEF2BLH(xyz_ref);
    BLH blh = ECEF2BLH(xyz_post);
    double cosB = cos(deg2rad(blh_ref.lat));
    double sinB = sin(deg2rad(blh_ref.lat));
    double cosL = cos(deg2rad(blh_ref.lon));
    double sinL = sin(deg2rad(blh_ref.lon));

    std::array<double, 3> x = { xyz_post.x - xyz_ref.x, xyz_post.y - xyz_ref.y, xyz_post.z - xyz_ref.z };
    std::vector<std::array<double, 3>> R = {
        {-sinL, cosL, 0.},
        {-sinB * cosL, -sinB * sinL, cosB},
        {cosB * cosL, cosB * sinL, sinB}
    };

    double enu00 = R[0][0] * x[0] + R[0][1] * x[1] + R[0][2] * x[2];
    double enu10 = R[1][0] * x[0] + R[1][1] * x[1] + R[1][2] * x[2];
    double enu20 = R[2][0] * x[0] + R[2][1] * x[1] + R[2][2] * x[2];

    return { enu00, enu10, enu20 };
}

XYZ ENU2ECEF(BLH blh_ref, XYZ enu)
{
    double cosB = cos(blh_ref.lat * M_PI / 180);
    double sinB = sin(blh_ref.lat * M_PI / 180);
    double cosL = cos(blh_ref.lon * M_PI / 180);
    double sinL = sin(blh_ref.lon * M_PI / 180);

    Eigen::Matrix <double, 3, 3> R;
    R << -sinL, -sinB * cosL, cosB* cosL, cosL, -sinB * sinL, cosB* sinL, 0., cosB, sinB;
    Eigen::Vector3d menu;
    menu << enu.x, enu.y, enu.z;
    Eigen::Vector3d ecef;

    XYZ xyz_ref = BLH2ECEF(blh_ref);

    ecef = R * menu;

    XYZ output;
    output.x = ecef(0, 0) + xyz_ref.x;
    output.y = ecef(1, 0) + xyz_ref.y;
    output.z = ecef(2, 0) + xyz_ref.z;
    return output;

}

XYZ ECEFtoECI(XYZ ecef, const double S0, const double ti){
    XYZ eci;
    double s_zv = S0 * EarthConstants::sec_in_rad + EarthConstants::omega_z * ti;
    double cos_s = cos(s_zv);
    double sin_s = sin(s_zv);
    eci.x = ecef.x * cos_s - ecef.y * sin_s;
    eci.y = ecef.x * sin_s + ecef.y * cos_s;
    eci.z = ecef.z;
    return eci;
}

ResultRangeMNK mnkRangeCoords(std::vector<double> d, std::vector<XYZ> posts, XYZ X0) {

    int N = d.size();

    XYZ crd;
    crd.x = 0;
    crd.y = 0;
    crd.z = 0;
    //std::pair<bool, XYZ> output;
    ResultRangeMNK output;
    output.flag = false;
    output.res = crd;
    output.iter = 0;
    output.nev = 0.;

    if (N < 4) return output;

    Eigen::VectorXd D(N);
    Eigen::MatrixXd POSTS(3, N);
    for (int i = 0; i < N; i++) {
        D.coeffRef(i) = d.at(i);
        POSTS(0, i) = posts.at(i).x;
        POSTS(1, i) = posts.at(i).y;
        POSTS(2, i) = posts.at(i).z;
    }

    double eps = 0.001;
    int max_iter = 10;

    Eigen::VectorXd Y(N);
    Eigen::MatrixXd H(N, 3);
    Eigen::VectorXd X(3);
    X(0) = X0.x;
    X(1) = X0.y;
    X(2) = X0.z;

    int iter = 0;
    double nev = 0.;
    while (true) {
        for (int i = 0; i < N; i++) {
            double d = sqrt(pow(X(0) - POSTS(0, i), 2) + pow(X(1) - POSTS(1, i), 2) + pow(X(2) - POSTS(2, i), 2));
            Y(i) = d;
            H(i, 0) = (X(0) - POSTS(0, i)) / d;
            H(i, 1) = (X(1) - POSTS(1, i)) / d;
            H(i, 2) = (X(2) - POSTS(2, i)) / d;
        }
        Eigen::VectorXd Xprev = X;
        X = X + (H.transpose() * H).inverse() * H.transpose() * (D - Y);
        iter++;
        if (iter > max_iter) break;
        nev = (X - Xprev).norm();
        if (nev < eps) break;
    }

    crd.x = X(0);
    crd.y = X(1);
    crd.z = X(2);
    output.flag = true;
    output.res = crd;
    output.iter = iter;
    output.nev = nev;
    return output;

}


ResultPseudoRangeMNK mnkPseudoRangeCoordsTime(std::vector<double> d, std::vector<XYZ> posts, XYZ X0) {

    int N = d.size();

    XYZT crd;
    crd.x = 0;
    crd.y = 0;
    crd.z = 0;
    crd.T = 0;
    //std::pair<bool, XYZ> output;
    ResultPseudoRangeMNK output;
    output.flag = false;
    output.res = crd;
    output.iter = 0;
    output.nev = 0.;

    if (N < 4) return output;

    Eigen::VectorXd D(N);
    Eigen::MatrixXd POSTS(3, N);
    for (int i = 0; i < N; i++) {
        D.coeffRef(i) = d.at(i);
        POSTS(0, i) = posts.at(i).x;
        POSTS(1, i) = posts.at(i).y;
        POSTS(2, i) = posts.at(i).z;
    }

    double eps = 0.001;
    int max_iter = 10;

    Eigen::VectorXd Y(N);
    Eigen::MatrixXd H(N, 4);
    Eigen::VectorXd X(4);
    X(0) = X0.x;
    X(1) = X0.y;
    X(2) = X0.z;
    X(3) = D(0);

    int iter = 0;
    double nev = 0.;
    while (true) {
        for (int i = 0; i < N; i++) {
            double d = sqrt(pow(X(0) - POSTS(0, i), 2) + pow(X(1) - POSTS(1, i), 2) + pow(X(2) - POSTS(2, i), 2));
            Y(i) = d + X(3);
            H(i, 0) = (X(0) - POSTS(0, i)) / d;
            H(i, 1) = (X(1) - POSTS(1, i)) / d;
            H(i, 2) = (X(2) - POSTS(2, i)) / d;
            H(i, 3) = 1;
        }
        Eigen::VectorXd Xprev = X;
        X = X + (H.transpose() * H).inverse() * H.transpose() * (D - Y);
        iter++;
        if (iter > max_iter) break;
        nev = (X - Xprev).norm();
        if (nev < eps) break;
    }

    crd.x = X(0);
    crd.y = X(1);
    crd.z = X(2);
    crd.T = X(3);
    output.flag = true;
    output.res = crd;
    output.iter = iter;
    output.nev = nev;
    return output;

}

ResultPseudoVeloMNK mnkPseudoVeloCoordsTime(std::vector<double> vr, std::vector<XYZ> posts, XYZ V0, XYZ X0) {

    int N = vr.size();

    XYZT crd;
    crd.x = 0;
    crd.y = 0;
    crd.z = 0;
    crd.T = 0;
    //std::pair<bool, XYZ> output;
    ResultPseudoVeloMNK output;
    Eigen::VectorXd Vr(N);
    output.flag = false;
    output.res = crd;
    output.iter = 0;
    output.nev = 0.;

    if (N < 4) return output;

    Eigen::VectorXd D(N);
    Eigen::MatrixXd POSTS(3, N);
    for (int i = 0; i < N; i++) {
        POSTS(0, i) = posts.at(i).x;
        POSTS(1, i) = posts.at(i).y;
        POSTS(2, i) = posts.at(i).z;
        Vr[i] = vr[i];
    }

    double eps = 0.001;
    int max_iter = 10;

    Eigen::VectorXd Y(N);
    Eigen::MatrixXd H(N, 4);
    Eigen::VectorXd X(4);
    X(0) = V0.x;
    X(1) = V0.y;
    X(2) = V0.z;
    X(3) = normrnd(0.0, 1e-11) * EarthConstants::c;

    int iter = 0;
    double nev = 0.;
    while (true) {
        for (int i = 0; i < N; i++) {
            double d = sqrt(pow(X0.x - POSTS(0, i), 2) + pow(X0.y - POSTS(1, i), 2) + pow(X0.z - POSTS(2, i), 2));
            Y(i) = ((-X0.x + POSTS(0, i)) * (-1) * X(0) + (-X0.y + POSTS(1, i)) * (-1) * X(1) + (-X0.z + POSTS(2, i)) * (-1) * X(2)) / d + X(3);

            H(i, 0) = (X0.x - POSTS(0, i)) / d;
            H(i, 1) = (X0.y - POSTS(1, i)) / d;
            H(i, 2) = (X0.z - POSTS(2, i)) / d;
            H(i, 3) = 1;
        }
        Eigen::VectorXd Xprev = X;
        X = X + (H.transpose() * H).inverse() * H.transpose() * (Vr - Y);
        iter++;
        if (iter > max_iter) break;
        nev = (X - Xprev).norm();
        if (nev < eps) break;
    }

    crd.x = X(0);
    crd.y = X(1);
    crd.z = X(2);
    crd.T = X(3);
    output.flag = true;
    output.res = crd;
    output.iter = iter;
    output.nev = nev;
    return output;

}



ResultPseudoVeloMNK mnkVeloCoordsTime(std::vector<double> vr, std::vector<XYZ> posts, XYZ V0, XYZ X0) {

    int N = vr.size();

    XYZT crd;
    crd.x = 0;
    crd.y = 0;
    crd.z = 0;
    crd.T = 0;
    //std::pair<bool, XYZ> output;
    ResultPseudoVeloMNK output;
    Eigen::VectorXd Vr(N);
    output.flag = false;
    output.res = crd;
    output.iter = 0;
    output.nev = 0.;

    if (N < 4) return output;

    Eigen::VectorXd D(N);
    Eigen::MatrixXd POSTS(3, N);
    for (int i = 0; i < N; i++) {
        POSTS(0, i) = posts.at(i).x;
        POSTS(1, i) = posts.at(i).y;
        POSTS(2, i) = posts.at(i).z;
        //D.coeffRef(i) = sqrt(pow(X0.x - POSTS(0, i), 2) + pow(X0.y - POSTS(1, i), 2) + pow(X0.z - POSTS(2, i), 2));
        Vr[i] = vr[i];
    }

    double eps = 0.001;
    int max_iter = 30;

    Eigen::VectorXd Y(N);
    Eigen::MatrixXd H(N, 3);
    Eigen::VectorXd X(3);
    X(0) = V0.x;
    X(1) = V0.y;
    X(2) = V0.z;

    int iter = 0;
    double nev = 0.;
    while (true) {
        for (int i = 0; i < N; i++) {
            double d = sqrt(pow(X0.x - POSTS(0, i), 2) + pow(X0.y - POSTS(1, i), 2) + pow(X0.z - POSTS(2, i), 2));
            Y(i) = ((-X0.x + POSTS(0, i)) * (-1) * X(0) + (-X0.y + POSTS(1, i)) * (-1) * X(1) + (-X0.z + POSTS(2, i)) * (-1) * X(2)) / d;

            H(i, 0) = (X0.x - POSTS(0, i)) / d;
            H(i, 1) = (X0.y - POSTS(1, i)) / d;
            H(i, 2) = (X0.z - POSTS(2, i)) / d;
        }
        Eigen::VectorXd Xprev = X;
        X = X + (H.transpose() * H).inverse() * H.transpose() * (Vr - Y);
        iter++;
        if (iter > max_iter) break;
        nev = (X - Xprev).norm();
        if (nev < eps) break;
    }

    crd.x = X(0);
    crd.y = X(1);
    crd.z = X(2);
    output.flag = true;
    output.res = crd;
    output.iter = iter;
    output.nev = nev;
    return output;

}




DOP CalcPseudoRangeDOP(std::vector<XYZ> posts, XYZ X0) {

    int N = posts.size();

    Eigen::MatrixXd H(N, 4);
    Eigen::VectorXd X(3);
    Eigen::MatrixXd POSTS(3, N);
    X(0) = X0.x;
    X(1) = X0.y;
    X(2) = X0.z;

    for (int i = 0; i < N; i++) {
        POSTS(0, i) = posts.at(i).x;
        POSTS(1, i) = posts.at(i).y;
        POSTS(2, i) = posts.at(i).z;
    }

    for (int i = 0; i < N; i++) {
        double d = sqrt(pow(X(0) - POSTS(0, i), 2) + pow(X(1) - POSTS(1, i), 2) + pow(X(2) - POSTS(2, i), 2));
        H(i, 0) = (X(0) - POSTS(0, i)) / d;
        H(i, 1) = (X(1) - POSTS(1, i)) / d;
        H(i, 2) = (X(2) - POSTS(2, i)) / d;
        H(i, 3) = 1;
    }

    Eigen::MatrixXd invHH = (H.transpose() * H).inverse();
    DOP dop;
    dop.XDOP = sqrt(fabs(invHH(0, 0)));
    dop.YDOP = sqrt(fabs(invHH(1, 1)));
    dop.VDOP = sqrt(fabs(invHH(2, 2)));
    dop.TDOP = sqrt(fabs(invHH(3, 3)));
    dop.HDOP = sqrt(dop.XDOP * dop.XDOP + dop.YDOP * dop.YDOP);
    dop.PDOP = sqrt(dop.XDOP * dop.XDOP + dop.YDOP * dop.YDOP + dop.VDOP * dop.VDOP);
    return dop;
}

Clock markovModelOrder1(Clock clock, double dt, double instability) {
    double noise = normrnd(0.0, instability);
    clock.drift += noise * dt;
    clock.shift += clock.drift * dt;
    
    return clock;
}

VisSat satVisabilityForViewPoint(const XYZ view_point_pos, 
    const XYZ veiw_point_vel, const XYZ sat_pos, const XYZ sat_vel, const double mask) {

    BLH BLHref;
    BLH BLHnka;
    XYZ SATenu;

    double elevation = 0, azimuth = 0;

    BLHref = ECEF2BLH(view_point_pos);
    BLHnka = ECEF2BLH(sat_pos);
    SATenu = BLH2ENU(BLHref, BLHnka);


    //������ ���� ���������� ���
    elevation = atan2(SATenu.z, sqrt(pow(SATenu.x, 2) + pow(SATenu.y, 2)));
    //������ ������� ���
    azimuth = atan2(SATenu.y, SATenu.x);
    if (azimuth < 0) {
        azimuth = azimuth + deg2rad(360.);
    }

    bool visible;

    //    if (BLHref.h > 1000.) visible = two_points_visability(BLHref, BLHnka);
    //    else visible = qRadiansToDegrees(elevation) > mask;

    visible = rad2deg(elevation) > mask;

    VisSat output;
    //�������� ���� �����
    if (visible)
    {
        Eigen::Vector3d cur_vel;
        Eigen::Vector3d delta;
        Eigen::Vector3d nka_enu;

        cur_vel << sat_vel.x - veiw_point_vel.x, sat_vel.y - veiw_point_vel.y, sat_vel.z - veiw_point_vel.z;
        delta << sat_pos.x - view_point_pos.x, sat_pos.y - view_point_pos.y, sat_pos.z - view_point_pos.z;
        nka_enu << SATenu.x, SATenu.y, SATenu.z;
        double vr = (cur_vel.transpose() * delta)(0) / delta.norm();

        output.visible = true;
        output.az = azimuth;
        output.el = elevation;
        output.dist = nka_enu.norm();
        output.vr = vr;
        output.posECEF = sat_pos;
        output.posENU = SATenu;
    }
    else output.visible = false;

    return output;
}

std::tm parseTime(const std::string& time_str) {
    std::tm tm = {};
    std::istringstream ss(time_str);
    ss >> std::get_time(&tm, "%Y %m %d %H %M %S");
    if (ss.fail()) {
        throw std::runtime_error("Error parsing time: " + time_str);
    }
    return tm;
}

std::pair<double, double> getStopTime(const json& config) {
    std::string start_time_str = config["start_time"];
    std::string stop_time_str = config["stop_time"];
    std::tm start_tm = parseTime(start_time_str);
    std::tm stop_tm = parseTime(stop_time_str);
    std::time_t start = std::mktime(&start_tm);
    std::time_t stop = std::mktime(&stop_tm);
    double time_eci = secondsFromStartOfDay(start_tm);
    return { difftime(stop, start),  time_eci };
}

double secondsFromStartOfDay(const std::tm& time) {
    return time.tm_hour * 3600 + time.tm_min * 60 + time.tm_sec;
}

