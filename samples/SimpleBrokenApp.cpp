#include "sdk/VehicleApp.h"

// SIMPLE SYNTAX ERROR - intentionally invalid C++
class BrokenApp {
public:
    BrokenApp() {
        // Missing semicolon
        int x = 5
    }
};

int main() {
    return 0;  // Missing semicolon here too