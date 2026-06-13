#pragma once
#include <QString>
#include "core/Constants.h"

namespace GDT {

struct Calib5A42 {
    double adc_duc1_min = ADC_DUC1_MIN;
    double adc_duc1_max = ADC_DUC1_MAX;
    double adc_duc2_min = ADC_DUC2_MIN;
    double adc_duc2_max = ADC_DUC2_MAX;
    double adc_duc3_min = ADC_DUC3_MIN;
    double adc_duc3_max = ADC_DUC3_MAX;
    double adc_ml1_min  = ADC_ML1_MIN;
    double adc_ml1_max  = ADC_ML1_MAX;
    double adc_ml2_min  = ADC_ML2_MIN;
    double adc_ml2_max  = ADC_ML2_MAX;
    double adc_ml3_min  = ADC_ML3_MIN;
    double adc_ml3_max  = ADC_ML3_MAX;
    double adc_duk_min  = ADC_DUK_MIN;
    double adc_duk_max  = ADC_DUK_MAX;
};

struct ServerConfig {
    QString serverIp   = DEFAULT_SERVER_IP;
    int     serverPort = DEFAULT_SERVER_PORT;
    bool    isPrimary  = true;
    QString clientIp   = DEFAULT_CLIENT_IP;
    int     clientPort = DEFAULT_CLIENT_PORT;
    QString mcastIp    = MCAST_IP_5E15;
    int     mcastPort  = MCAST_PORT_5E15;
};

struct AppConfig {
    ServerConfig server;
    Calib5A42    calib;
    QString      flightName;
};

} // namespace GDT
