#include "jme/jme.h"

#include <string.h>

typedef struct jme_name_entry {
    int id;
    const char *name;
} jme_name_entry;

static const jme_name_entry body_names[] = {
    {JME_BODY_SUN, "Sun"},
    {JME_BODY_MOON, "Moon"},
    {JME_BODY_MERCURY, "Mercury"},
    {JME_BODY_VENUS, "Venus"},
    {JME_BODY_MARS, "Mars"},
    {JME_BODY_JUPITER, "Jupiter"},
    {JME_BODY_SATURN, "Saturn"},
    {JME_BODY_URANUS, "Uranus"},
    {JME_BODY_NEPTUNE, "Neptune"},
    {JME_BODY_PLUTO, "Pluto"},
    {JME_BODY_MEAN_NODE, "Mean Node"},
    {JME_BODY_TRUE_NODE, "True Node"},
    {JME_BODY_EARTH, "Earth"},
    {JME_BODY_SOLAR_SYSTEM_BARYCENTER, "Solar System Barycenter"},
    {JME_BODY_MERCURY_BARYCENTER, "Mercury Barycenter"},
    {JME_BODY_VENUS_BARYCENTER, "Venus Barycenter"},
    {JME_BODY_EARTH_MOON_BARYCENTER, "Earth-Moon Barycenter"},
    {JME_BODY_MARS_BARYCENTER, "Mars Barycenter"},
    {JME_BODY_JUPITER_BARYCENTER, "Jupiter Barycenter"},
    {JME_BODY_SATURN_BARYCENTER, "Saturn Barycenter"},
    {JME_BODY_URANUS_BARYCENTER, "Uranus Barycenter"},
    {JME_BODY_NEPTUNE_BARYCENTER, "Neptune Barycenter"},
    {JME_BODY_PLUTO_BARYCENTER, "Pluto Barycenter"},
    {JME_BODY_CERES, "Ceres"},
    {JME_BODY_PALLAS, "Pallas"},
    {JME_BODY_JUNO, "Juno"},
    {JME_BODY_VESTA, "Vesta"},
    {JME_BODY_CHIRON, "Chiron"},
    {JME_BODY_PHOLUS, "Pholus"},
    {JME_BODY_ERIS, "Eris"},
    {JME_BODY_MAKEMAKE, "Makemake"},
    {JME_BODY_HAUMEA, "Haumea"},
    {JME_BODY_SEDNA, "Sedna"},
    {JME_BODY_QUAOAR, "Quaoar"},
    {JME_BODY_ORCUS, "Orcus"},
    {JME_BODY_VARUNA, "Varuna"}
};

static void copy_string(char *dst, const char *src)
{
    int i = 0;

    if (dst == 0) {
        return;
    }
    if (src == 0) {
        dst[0] = '\0';
        return;
    }

    while (i < 255 && src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

const char *jme_body_name(int body)
{
    size_t i;

    for (i = 0; i < sizeof(body_names) / sizeof(body_names[0]); i++) {
        if (body_names[i].id == body) {
            return body_names[i].name;
        }
    }

    return "Unknown";
}

int jme_body_naif_id(int body)
{
    switch (body) {
    case JME_BODY_SUN: return 10;
    case JME_BODY_MOON: return 301;
    case JME_BODY_MERCURY: return 199;
    case JME_BODY_VENUS: return 299;
    case JME_BODY_MARS: return 499;
    case JME_BODY_JUPITER: return 599;
    case JME_BODY_SATURN: return 699;
    case JME_BODY_URANUS: return 799;
    case JME_BODY_NEPTUNE: return 899;
    case JME_BODY_PLUTO: return 999;
    case JME_BODY_EARTH: return 399;
    case JME_BODY_SOLAR_SYSTEM_BARYCENTER: return 0;
    case JME_BODY_MERCURY_BARYCENTER: return 1;
    case JME_BODY_VENUS_BARYCENTER: return 2;
    case JME_BODY_EARTH_MOON_BARYCENTER: return 3;
    case JME_BODY_MARS_BARYCENTER: return 4;
    case JME_BODY_JUPITER_BARYCENTER: return 5;
    case JME_BODY_SATURN_BARYCENTER: return 6;
    case JME_BODY_URANUS_BARYCENTER: return 7;
    case JME_BODY_NEPTUNE_BARYCENTER: return 8;
    case JME_BODY_PLUTO_BARYCENTER: return 9;
    default: return -1;
    }
}

int jme_body_id_from_name(const char *name)
{
    size_t i;

    if (name == 0) {
        return JME_ERR;
    }

    for (i = 0; i < sizeof(body_names) / sizeof(body_names[0]); i++) {
        if (strcmp(body_names[i].name, name) == 0) {
            return body_names[i].id;
        }
    }

    return JME_ERR;
}

const char *jme_house_system_name(int house_system)
{
    switch (house_system) {
        case 'P':
            return "Placidus";
        case 'K':
            return "Koch";
        case 'O':
            return "Porphyrius";
        case 'R':
            return "Regiomontanus";
        case 'C':
            return "Campanus";
        case 'E':
            return "Equal";
        case 'W':
            return "Whole Sign";
        case 'B':
            return "Alcabitius";
        case 'M':
            return "Morinus";
        case 'U':
            return "Krusinski";
        case 'T':
            return "Polich/Page";
        case 'V':
        case JME_HOUSE_VEHLOW_EQUAL:
            return "Vehlow Equal";
        case 'A':
        case JME_HOUSE_AZIMUTHAL:
        case JME_HOUSE_HORIZONTAL:
            return "Horizontal/Azimuthal";
        case 'I':
        case JME_HOUSE_SUNSHINE:
            return "Sunshine";
        default:
            return 0;
    }
}

const char *jme_library_path(void)
{
    return "jme";
}

char *jme_copy_body_name(int body, char *buffer)
{
    const char *name = jme_body_name(body);
    copy_string(buffer, name);
    return name == 0 ? 0 : buffer;
}
