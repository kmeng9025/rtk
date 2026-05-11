#include <iostream>
#include <unistd.h>
#include <cstring>
#include <termios.h>
#include <fcntl.h>
#include <vector>
#include <numeric>
#include <math.h>

std::vector<std::string> split(const char* str, const char* delim){
    size_t delimSize = strlen(delim);
    std::vector<std::string> result;
    const char* found;
    while ((found = strstr(str, delim))) {
        result.push_back(std::string(str, found - str));
        str = found + delimSize;
    }
    if (*str != '\0'){
        result.push_back(std::string(str));
    }
    return result;
}

const int statSize = 2048;

double statNoRTKLat[statSize];
int indexNoRTKLat = 0;
int filledNoRTKLat = 0;

double statNoRTKLong[statSize];
int indexNoRTKLong = 0;
int filledNoRTKLong = 0;

double statRTKLat[statSize];
int indexRTKLat = 0;
int filledRTKLat = 0;

double statRTKLong[statSize];
int indexRTKLong = 0;
int filledRTKLong = 0;

bool hasRTK = false;

void calculateStatistics(){
    double totalNoRTKLat = 0;
    for (double num : statNoRTKLat) {
        totalNoRTKLat += num;
    }
    double meanNoRTKLat = totalNoRTKLat/filledNoRTKLat;

    double stdDevNoRTKLat = 0;
    for (int i = 0; i < filledNoRTKLat; i++) {
        stdDevNoRTKLat += std::pow(meanNoRTKLat - statNoRTKLat[i], 2);
    }
    stdDevNoRTKLat /= filledNoRTKLat;
    stdDevNoRTKLat = std::sqrt(stdDevNoRTKLat);
    std::cout << "\nNoRTKLat Stats. Data Points: " << filledNoRTKLat << " Mean: " << meanNoRTKLat << " Std Dev: " << stdDevNoRTKLat << "\n";


    double totalNoRTKLong = 0;
    for (double num : statNoRTKLong) {
        totalNoRTKLong += num;
    }
    double meanNoRTKLong = totalNoRTKLong/filledNoRTKLong;

    double stdDevNoRTKLong = 0;
    for (int i = 0; i < filledNoRTKLong; i++) {
        stdDevNoRTKLong += std::pow(meanNoRTKLong - statNoRTKLong[i], 2);
    }
    stdDevNoRTKLong /= filledNoRTKLong;
    stdDevNoRTKLong = std::sqrt(stdDevNoRTKLong);
    std::cout << "NoRTKLong Stats. Data Points: " << filledNoRTKLong << " Mean: " << meanNoRTKLong << " Std Dev: " << stdDevNoRTKLong << "\n";


    double totalRTKLat = 0;
    for (double num : statRTKLat) {
        totalRTKLat += num;
    }
    double meanRTKLat = totalRTKLat/filledRTKLat;

    double stdDevRTKLat = 0;
    for (int i = 0; i < filledRTKLat; i++) {
        stdDevRTKLat += std::pow(meanRTKLat - statRTKLat[i], 2);
    }
    stdDevRTKLat /= filledRTKLat;
    stdDevRTKLat = std::sqrt(stdDevRTKLat);
    std::cout << "RTKLat Stats. Data Points: " << filledRTKLat << " Mean: " << meanRTKLat << " Std Dev: " << stdDevRTKLat << "\n";


    double totalRTKLong = 0;
    for (double num : statRTKLong) {
        totalRTKLong += num;
    }
    double meanRTKLong = totalRTKLong/filledRTKLong;

    double stdDevRTKLong = 0;
    for (int i = 0; i < filledRTKLong; i++) {
        stdDevRTKLong += std::pow(meanRTKLong - statRTKLong[i], 2);
    }
    stdDevRTKLong /= filledRTKLong;
    stdDevRTKLong = std::sqrt(stdDevRTKLong);
    std::cout << "RTKLong Stats. Data Points: " << filledRTKLong << " Mean: " << meanRTKLong << " Std Dev: " << stdDevRTKLong << "\n";

    std::cout << "\n";
}


int main() {
    int serial = open("/dev/ttyS0", O_RDWR | O_NOCTTY);
    if (serial < 0) {
        std::cerr << "Error opening serial port" << std::endl;
        return 1;
    }
    termios tty{};
    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(serial, &tty) != 0) {
        std::cerr << "Error getting terminal attributes" << std::endl;
        close(serial);
        return 1;
    }
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;

    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_iflag = 0;
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(serial, TCSANOW, &tty) != 0) {
        std::cerr << "Error setting terminal attributes" << std::endl;
        close(serial);
        return 1;
    }
    
    std::cout << "Serial port configured successfully" << std::endl;

    std::string buffer;
    while (true) {
        char currentBuffer[256];
        int n = read(serial, currentBuffer, sizeof(currentBuffer) - 1);
        if (n > 0){
            currentBuffer[n] = '\0';
            buffer += currentBuffer;
        }
        std::vector<std::string> results = split(buffer.c_str(), "\n");
        for (int i = 0; i < results.size(); i++) {
            if(i == results.size() - 1){
                if (buffer.back() == '\n') {
                    buffer = "";
                } else {
                    buffer = results.back();
                    break;
                }
            }
            const char* result = results[i].c_str();
            if (std::string(result).find("$GNGGA") == 0) {
                // std::cout << "received gngga\n";
                // std::cout << result << "\n\n";
                std::vector<std::string> values = split(result, ",");
                if (values.size() >= 15) {
                    if (values.at(6).size() > 0) {
                        if (values.at(6) == "4") {
                            hasRTK = true;
                            std::cout << "\nRTK Fix\n";
                        } else if (values.at(6) == "5") {
                            std::cout << "\nFLOAT\n";
                            hasRTK = false;
                        } else if (values.at(6) == "3") {
                            std::cout << "\nPPS Fix\n";
                            hasRTK = false;
                        } else if (values.at(6) == "2") {
                            std::cout << "\nDifferential\n";
                            hasRTK = false;
                        } else {
                            hasRTK = false;
                        }
                    }
                    if (values.at(2).size() > 3) {
                        double degrees = std::stod(values.at(2).substr(0, 2)) + (std::stod(values.at(2).substr(2))/60.);
                        
                        if (hasRTK){
                            if (filledRTKLat < 2048){
                                filledRTKLat++;
                            }
                            indexRTKLat %= statSize;
                            statRTKLat[indexRTKLat] = degrees;
                            indexRTKLat ++;
                        } else {
                            if (filledNoRTKLat < 2048) {
                                filledNoRTKLat ++;
                            }
                            indexNoRTKLat %= statSize;
                            statNoRTKLat[indexNoRTKLat] = degrees;
                            indexNoRTKLat ++;
                        }
                        
                        std::cout << values.at(2).substr(0, 2) << " " << values.at(2).substr(2) << " " << values.at(3) << std::endl;
                    }
                    if (values.at(4).size() > 3) {
                        double degrees = std::stod(values.at(4).substr(0, 3)) + (std::stod(values.at(4).substr(3))/60.);
                        if (hasRTK){
                            if (filledRTKLong < 2048){
                                filledRTKLong++;
                            }
                            indexRTKLong %= statSize;
                            statRTKLong[indexRTKLong] = degrees;
                            indexRTKLong ++;
                        } else {
                            if (filledNoRTKLong < 2048) {
                                filledNoRTKLong ++;
                            }
                            indexNoRTKLong %= statSize;
                            statNoRTKLong[indexNoRTKLong] = degrees;
                            indexNoRTKLong ++;
                        }
                        std::cout << values.at(4).substr(0, 3) << " " << values.at(4).substr(3) << " " << values.at(5) << std::endl;
                    }
                    calculateStatistics();
                }                
            }

            // if (strstr(result, ",R,")) {
            //     std::cout << "\n***RTK FIX DETECTED***\n" << std::endl;
            //     hasRTK = true;
            // }

            // if (strstr(result, ",F,")) {
            //     std::cout << "\n***RTK FLOAT DETECTED***\n" << std::endl;
            // }

            // if (strstr(result, ",D,")) {
            //     std::cout << "\n***DGPS ACTIVE***\n" << std::endl;
            // }

            // if (strstr(result, ",A,")) {
            //     // std::cout << "\n***AUTONOMOUS ACTIVE***\n" << std::endl;
            //     hasRTK = false;
            // }
        }
    }

    close(serial);

    return 0;
}