#include "jme/jme.h"

#include <math.h>

#define JME_DEG_TO_RAD 0.017453292519943295769236907684886127134428718885417
#define JME_RAD_TO_DEG 57.295779513082320876798154814105170332405472466565

static double sind(double x)
{
    return sin(x * JME_DEG_TO_RAD);
}

static double cosd(double x)
{
    return cos(x * JME_DEG_TO_RAD);
}

static double tand(double x)
{
    return tan(x * JME_DEG_TO_RAD);
}

static double atan2d(double y, double x)
{
    return atan2(y, x) * JME_RAD_TO_DEG;
}

static double asind(double x)
{
    return asin(x) * JME_RAD_TO_DEG;
}

static double clamp_unit(double x)
{
    if (x > 1.0) {
        return 1.0;
    }
    if (x < -1.0) {
        return -1.0;
    }
    return x;
}

void jme_ecliptic_to_equatorial(
    double lon,
    double lat,
    double eps,
    double *ra,
    double *dec
)
{
    double sin_lon = sind(lon);
    double cos_lon = cosd(lon);
    double tan_lat = tand(lat);
    double sin_lat = sind(lat);
    double cos_lat = cosd(lat);
    double sin_eps = sind(eps);
    double cos_eps = cosd(eps);
    double ra_value = atan2d((sin_lon * cos_eps) - (tan_lat * sin_eps), cos_lon);
    double dec_value = asind((sin_lat * cos_eps) + (cos_lat * sin_eps * sin_lon));

    if (ra != 0) {
        *ra = jme_degree_normalize(ra_value);
    }
    if (dec != 0) {
        *dec = dec_value;
    }
}

void jme_equatorial_to_ecliptic(
    double ra,
    double dec,
    double eps,
    double *lon,
    double *lat
)
{
    double sin_ra = sind(ra);
    double cos_ra = cosd(ra);
    double tan_dec = tand(dec);
    double sin_dec = sind(dec);
    double cos_dec = cosd(dec);
    double sin_eps = sind(eps);
    double cos_eps = cosd(eps);
    double lon_value = atan2d((sin_ra * cos_eps) + (tan_dec * sin_eps), cos_ra);
    double lat_value = asind((sin_dec * cos_eps) - (cos_dec * sin_eps * sin_ra));

    if (lon != 0) {
        *lon = jme_degree_normalize(lon_value);
    }
    if (lat != 0) {
        *lat = lat_value;
    }
}

int jme_ecliptic_to_equatorial_rectangular_state(
    const double *ecliptic,
    double eps,
    double *equatorial
)
{
    double sin_eps;
    double cos_eps;

    if (ecliptic == 0 || equatorial == 0) {
        return JME_ERR;
    }

    sin_eps = sind(eps);
    cos_eps = cosd(eps);

    equatorial[0] = ecliptic[0];
    equatorial[1] = (ecliptic[1] * cos_eps) - (ecliptic[2] * sin_eps);
    equatorial[2] = (ecliptic[1] * sin_eps) + (ecliptic[2] * cos_eps);
    equatorial[3] = ecliptic[3];
    equatorial[4] = (ecliptic[4] * cos_eps) - (ecliptic[5] * sin_eps);
    equatorial[5] = (ecliptic[4] * sin_eps) + (ecliptic[5] * cos_eps);

    return JME_OK;
}

int jme_equatorial_to_ecliptic_rectangular_state(
    const double *equatorial,
    double eps,
    double *ecliptic
)
{
    double sin_eps;
    double cos_eps;

    if (equatorial == 0 || ecliptic == 0) {
        return JME_ERR;
    }

    sin_eps = sind(eps);
    cos_eps = cosd(eps);

    ecliptic[0] = equatorial[0];
    ecliptic[1] = (equatorial[1] * cos_eps) + (equatorial[2] * sin_eps);
    ecliptic[2] = (-equatorial[1] * sin_eps) + (equatorial[2] * cos_eps);
    ecliptic[3] = equatorial[3];
    ecliptic[4] = (equatorial[4] * cos_eps) + (equatorial[5] * sin_eps);
    ecliptic[5] = (-equatorial[4] * sin_eps) + (equatorial[5] * cos_eps);

    return JME_OK;
}

void jme_equatorial_to_horizontal(
    double hour_angle,
    double dec,
    double geo_lat,
    double *azimuth,
    double *altitude
)
{
    double sin_h = sind(hour_angle);
    double cos_h = cosd(hour_angle);
    double sin_dec = sind(dec);
    double cos_dec = cosd(dec);
    double sin_lat = sind(geo_lat);
    double cos_lat = cosd(geo_lat);
    double east = -cos_dec * sin_h;
    double north = (sin_dec * cos_lat) - (cos_dec * cos_h * sin_lat);
    double up = (sin_dec * sin_lat) + (cos_dec * cos_h * cos_lat);

    if (azimuth != 0) {
        *azimuth = jme_degree_normalize(atan2d(east, north));
    }
    if (altitude != 0) {
        *altitude = asind(clamp_unit(up));
    }
}

void jme_horizontal_to_equatorial(
    double azimuth,
    double altitude,
    double geo_lat,
    double *hour_angle,
    double *dec
)
{
    double sin_alt = sind(altitude);
    double cos_alt = cosd(altitude);
    double sin_az = sind(azimuth);
    double cos_az = cosd(azimuth);
    double sin_lat = sind(geo_lat);
    double cos_lat = cosd(geo_lat);
    double east = cos_alt * sin_az;
    double north = cos_alt * cos_az;
    double up = sin_alt;
    double sin_dec = (up * sin_lat) + (north * cos_lat);
    double cos_dec_cos_h = (up * cos_lat) - (north * sin_lat);
    double hour_angle_value = atan2d(-east, cos_dec_cos_h);

    if (hour_angle != 0) {
        *hour_angle = jme_degree_normalize(hour_angle_value);
    }
    if (dec != 0) {
        *dec = asind(clamp_unit(sin_dec));
    }
}

double jme_spherical_angular_separation(double lon1, double lat1, double lon2, double lat2)
{
    double sin_lat1 = sind(lat1);
    double sin_lat2 = sind(lat2);
    double cos_lat1 = cosd(lat1);
    double cos_lat2 = cosd(lat2);
    double delta_lon = (lon2 - lon1) * JME_DEG_TO_RAD;
    double cos_sep = (sin_lat1 * sin_lat2) + (cos_lat1 * cos_lat2 * cos(delta_lon));

    return acos(clamp_unit(cos_sep)) * JME_RAD_TO_DEG;
}

double jme_spherical_position_angle(double lon1, double lat1, double lon2, double lat2)
{
    double delta_lon = (lon2 - lon1) * JME_DEG_TO_RAD;
    double y = sin(delta_lon);
    double x = (cosd(lat1) * tand(lat2)) - (sind(lat1) * cos(delta_lon));

    return jme_degree_normalize(atan2d(y, x));
}

double jme_refract(double altitude, double pressure, double temperature, int calc_flag)
{
    double refraction;

    if (pressure <= 0.0 || altitude < -1.0) {
        return altitude;
    }

    refraction = 1.02 / tand(altitude + (10.3 / (altitude + 5.11)));
    refraction *= (pressure / 1010.0) * (283.0 / (273.0 + temperature)) / 60.0;

    if (calc_flag == JME_COORD_APPARENT_TO_TRUE) {
        return altitude - refraction;
    }

    return altitude + refraction;
}

double jme_refract_extended(
    double altitude,
    double geoalt,
    double pressure,
    double temperature,
    double lapse_rate,
    int calc_flag,
    double *out
)
{
    double refracted;
    double local_pressure = pressure;
    double local_temperature = temperature;

    if (pressure > 0.0 && geoalt != 0.0) {
        const double t0 = 273.15 + temperature;
        const double g_over_r = 5.255877432444129;
        double h_km = geoalt / 1000.0;

        if (lapse_rate > 0.0 && t0 > 0.0) {
            double ratio = (t0 - lapse_rate * geoalt) / t0;
            if (ratio > 0.0) {
                local_pressure = pressure * pow(ratio, g_over_r);
                local_temperature = temperature - lapse_rate * geoalt;
            }
        } else {
            local_pressure = pressure * exp(-h_km / 7.996);
        }
    }

    refracted = jme_refract(altitude, local_pressure, local_temperature, calc_flag);
    if (out != 0) {
        out[0] = refracted;
        out[1] = refracted - altitude;
        out[2] = local_pressure;
        out[3] = local_temperature;
    }
    return refracted;
}

int jme_rectangular_to_spherical_state(const double *rectangular, double *spherical)
{
    double x;
    double y;
    double z;
    double vx;
    double vy;
    double vz;
    double rho2;
    double rho;
    double r2;
    double r;
    double xyv;

    if (rectangular == 0 || spherical == 0) {
        return JME_ERR;
    }

    x = rectangular[0];
    y = rectangular[1];
    z = rectangular[2];
    vx = rectangular[3];
    vy = rectangular[4];
    vz = rectangular[5];
    rho2 = (x * x) + (y * y);
    r2 = rho2 + (z * z);

    if (rho2 <= 0.0 || r2 <= 0.0) {
        spherical[0] = 0.0;
        spherical[1] = 0.0;
        spherical[2] = 0.0;
        spherical[3] = 0.0;
        spherical[4] = 0.0;
        spherical[5] = 0.0;
        return JME_ERR;
    }

    rho = sqrt(rho2);
    r = sqrt(r2);
    xyv = (x * vx) + (y * vy);

    spherical[0] = jme_degree_normalize(atan2(y, x) * JME_RAD_TO_DEG);
    spherical[1] = atan2(z, rho) * JME_RAD_TO_DEG;
    spherical[2] = r;
    spherical[3] = ((x * vy) - (y * vx)) / rho2 * JME_RAD_TO_DEG;
    spherical[4] = ((rho * vz) - (z * xyv / rho)) / r2 * JME_RAD_TO_DEG;
    spherical[5] = ((x * vx) + (y * vy) + (z * vz)) / r;

    return JME_OK;
}

int jme_spherical_to_rectangular_state(const double *spherical, double *rectangular)
{
    double lon;
    double lat;
    double radius;
    double lon_rate;
    double lat_rate;
    double radius_rate;
    double sin_lon;
    double cos_lon;
    double sin_lat;
    double cos_lat;

    if (spherical == 0 || rectangular == 0) {
        return JME_ERR;
    }

    lon = spherical[0] * JME_DEG_TO_RAD;
    lat = spherical[1] * JME_DEG_TO_RAD;
    radius = spherical[2];
    lon_rate = spherical[3] * JME_DEG_TO_RAD;
    lat_rate = spherical[4] * JME_DEG_TO_RAD;
    radius_rate = spherical[5];

    if (radius <= 0.0) {
        rectangular[0] = 0.0;
        rectangular[1] = 0.0;
        rectangular[2] = 0.0;
        rectangular[3] = 0.0;
        rectangular[4] = 0.0;
        rectangular[5] = 0.0;
        return JME_ERR;
    }

    sin_lon = sin(lon);
    cos_lon = cos(lon);
    sin_lat = sin(lat);
    cos_lat = cos(lat);

    rectangular[0] = radius * cos_lat * cos_lon;
    rectangular[1] = radius * cos_lat * sin_lon;
    rectangular[2] = radius * sin_lat;
    rectangular[3] = (radius_rate * cos_lat * cos_lon)
        - (radius * sin_lat * lat_rate * cos_lon)
        - (radius * cos_lat * sin_lon * lon_rate);
    rectangular[4] = (radius_rate * cos_lat * sin_lon)
        - (radius * sin_lat * lat_rate * sin_lon)
        + (radius * cos_lat * cos_lon * lon_rate);
    rectangular[5] = (radius_rate * sin_lat) + (radius * cos_lat * lat_rate);

    return JME_OK;
}

double jme_state_distance(const double *state)
{
    if (state == 0) {
        return 0.0;
    }

    return sqrt((state[0] * state[0]) + (state[1] * state[1]) + (state[2] * state[2]));
}

static int jme_unit_distance_is_km(int unit)
{
    return unit == JME_VECTOR_KM_PER_DAY || unit == JME_VECTOR_KM_PER_SECOND;
}

static int jme_unit_velocity_is_per_second(int unit)
{
    return unit == JME_VECTOR_AU_PER_SECOND || unit == JME_VECTOR_KM_PER_SECOND;
}

static int jme_unit_is_valid(int unit)
{
    return unit == JME_VECTOR_AU_PER_DAY ||
        unit == JME_VECTOR_KM_PER_DAY ||
        unit == JME_VECTOR_AU_PER_SECOND ||
        unit == JME_VECTOR_KM_PER_SECOND;
}

int jme_state_convert_units(const double *input, int input_unit, int output_unit, double *output)
{
    int i;
    double position_factor = 1.0;
    double velocity_factor = 1.0;

    if (input == 0 || output == 0 || !jme_unit_is_valid(input_unit) || !jme_unit_is_valid(output_unit)) {
        return JME_ERR;
    }

    if (jme_unit_distance_is_km(input_unit) && !jme_unit_distance_is_km(output_unit)) {
        position_factor = 1.0 / JME_AU_KM;
    } else if (!jme_unit_distance_is_km(input_unit) && jme_unit_distance_is_km(output_unit)) {
        position_factor = JME_AU_KM;
    }

    velocity_factor = position_factor;
    if (jme_unit_velocity_is_per_second(input_unit) && !jme_unit_velocity_is_per_second(output_unit)) {
        velocity_factor *= JME_SECONDS_PER_DAY;
    } else if (!jme_unit_velocity_is_per_second(input_unit) && jme_unit_velocity_is_per_second(output_unit)) {
        velocity_factor /= JME_SECONDS_PER_DAY;
    }

    for (i = 0; i < 3; i++) {
        output[i] = input[i] * position_factor;
        output[i + 3] = input[i + 3] * velocity_factor;
    }

    return JME_OK;
}

double jme_state_light_time_days(const double *state, int unit)
{
    double distance;

    if (state == 0 || !jme_unit_is_valid(unit)) {
        return 0.0;
    }

    distance = jme_state_distance(state);
    if (!jme_unit_distance_is_km(unit)) {
        distance *= JME_AU_KM;
    }

    return distance / JME_SPEED_OF_LIGHT_KM_PER_SEC / JME_SECONDS_PER_DAY;
}

double jme_state_speed(const double *state)
{
    if (state == 0) {
        return 0.0;
    }

    return sqrt((state[3] * state[3]) + (state[4] * state[4]) + (state[5] * state[5]));
}

double jme_state_position_velocity_dot(const double *state)
{
    if (state == 0) {
        return 0.0;
    }

    return (state[0] * state[3]) + (state[1] * state[4]) + (state[2] * state[5]);
}

int jme_state_add(const double *left, const double *right, double *output)
{
    int i;

    if (left == 0 || right == 0 || output == 0) {
        return JME_ERR;
    }

    for (i = 0; i < 6; i++) {
        output[i] = left[i] + right[i];
    }

    return JME_OK;
}

int jme_state_subtract(const double *left, const double *right, double *output)
{
    int i;

    if (left == 0 || right == 0 || output == 0) {
        return JME_ERR;
    }

    for (i = 0; i < 6; i++) {
        output[i] = left[i] - right[i];
    }

    return JME_OK;
}

int jme_state_scale(const double *input, double factor, double *output)
{
    int i;

    if (input == 0 || output == 0) {
        return JME_ERR;
    }

    for (i = 0; i < 6; i++) {
        output[i] = input[i] * factor;
    }

    return JME_OK;
}

void jme_matrix_identity(double *m)
{
    int i;
    for (i = 0; i < 9; i++) {
        m[i] = 0.0;
    }
    m[0] = m[4] = m[8] = 1.0;
}

void jme_matrix_multiply(const double *a, const double *b, double *out)
{
    int i, j, k;
    double res[9];

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            res[i * 3 + j] = 0.0;
            for (k = 0; k < 3; k++) {
                res[i * 3 + j] += a[i * 3 + k] * b[k * 3 + j];
            }
        }
    }

    for (i = 0; i < 9; i++) {
        out[i] = res[i];
    }
}

void jme_matrix_rotate_x(double angle_rad, double *m)
{
    double c = cos(angle_rad);
    double s = sin(angle_rad);
    double res[9];

    jme_matrix_identity(res);
    res[4] = c;
    res[5] = s;
    res[7] = -s;
    res[8] = c;
    jme_matrix_multiply(res, m, m);
}

void jme_matrix_rotate_y(double angle_rad, double *m)
{
    double c = cos(angle_rad);
    double s = sin(angle_rad);
    double res[9];

    jme_matrix_identity(res);
    res[0] = c;
    res[2] = -s;
    res[6] = s;
    res[8] = c;
    jme_matrix_multiply(res, m, m);
}

void jme_matrix_rotate_z(double angle_rad, double *m)
{
    double c = cos(angle_rad);
    double s = sin(angle_rad);
    double res[9];

    jme_matrix_identity(res);
    res[0] = c;
    res[1] = s;
    res[3] = -s;
    res[4] = c;
    jme_matrix_multiply(res, m, m);
}

void jme_matrix_transform_state(const double *m, const double *input, double *output)
{
    int i, j;
    double res[6];

    for (i = 0; i < 3; i++) {
        res[i] = 0.0;
        res[i + 3] = 0.0;
        for (j = 0; j < 3; j++) {
            res[i] += m[i * 3 + j] * input[j];
            res[i + 3] += m[i * 3 + j] * input[j + 3];
        }
    }

    for (i = 0; i < 6; i++) {
        output[i] = res[i];
    }
}
