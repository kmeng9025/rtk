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


double statNoRTK[2048];
int indexNoRTK = 0;
int filledNoRTK = 0;
double statRTK[2048];
int indexRTK = 0;
int filledRTK = 0;

bool hasRTK = false;

void calculateStatistics(){
    double totalNoRTK = 0;
    for (double num : statNoRTK) {
        totalNoRTK += num;
    }
    double meanNoRTK = totalNoRTK/filledNoRTK;

    double stdDevNoRTK = 0;
    for (int i = 0; i < filledNoRTK; i++) {
        stdDevNoRTK += std::pow(meanNoRTK - statNoRTK[i], 2);
    }
    stdDevNoRTK /= filledNoRTK;
    stdDevNoRTK = std::sqrt(stdDevNoRTK);
    std::cout << "NoRTK Stats. Data Points: " << filledNoRTK << " Mean: " << meanNoRTK << " Std Dev: " << stdDevNoRTK << "\n";

    double totalRTK = 0;
    for (double num : statRTK) {
        totalRTK += num;
    }
    double meanRTK = totalRTK/filledRTK;

    double stdDevRTK = 0;
    for (int i = 0; i < filledRTK; i++) {
        stdDevRTK += std::pow(meanRTK - statRTK[i], 2);
    }
    stdDevRTK /= filledRTK;
    stdDevRTK = std::sqrt(stdDevRTK);
    std::cout << "RTK Stats. Data Points: " << filledRTK << " Mean: " << meanRTK << " Std Dev: " << stdDevRTK << "\n";

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
                        if (values.at(6) == "5") {
                            hasRTK = true;
                        } else {
                            hasRTK = false;
                        }
                    }
                    if (values.at(2).size() > 3) {
                        double degrees = std::stod(values.at(2).substr(0, 2)) + (std::stod(values.at(2).substr(2))/60.);
                        if (hasRTK){
                            filledRTK %= sizeof(statRTK);
                            statRTK[filledRTK] = degrees;
                            filledRTK ++;
                        } else {
                            filledNoRTK %= sizeof(statNoRTK);
                            statNoRTK[filledNoRTK] = degrees;
                            filledNoRTK ++;
                        }
                        calculateStatistics();
                        std::cout << values.at(2).substr(0, 2) << " " << values.at(2).substr(2) << " " << values.at(3) << std::endl;
                    }
                    if (values.at(4).size() > 3) {
                        double degrees = std::stod(values.at(4).substr(0, 3)) + (std::stod(values.at(4).substr(3))/60.);
                        // if (hasRTK){
                        //     if (filledNoRTK < sizeof(statNoRTK)){
                        //         filledNoRTK ++;
                        //     }
                        //     indexNoRTK %= sizeof(statRTK);
                        //     statRTK[indexNoRTK] = degrees;
                        //     indexNoRTK ++;
                        // } else {
                        //     if (filledRTK < sizeof(statRTK)) {
                        //         filledRTK ++;
                        //     }
                        //     indexNoRTK %= sizeof(statNoRTK);
                        //     statNoRTK[indexNoRTK] = degrees;
                        //     indexNoRTK ++;
                        // }
                        // calculateStatistics();
                        std::cout << values.at(4).substr(0, 3) << " " << values.at(4).substr(3) << " " << values.at(5) << std::endl;
                    }
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