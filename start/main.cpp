#include <Windows.h>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;
bool InstallAndStartDriver();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, int iCmdShow) {
    InstallAndStartDriver();
    return 0;
}

bool InstallAndStartDriver() {
    const char* serviceName = "RWDriver";
    fs::path driverPath = fs::current_path() / "Driver.sys";
    if (!fs::exists(driverPath)) {
        exit(-2);
        return false;
    }

    SC_HANDLE scManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (scManager == NULL) {
        exit(-3);
        return false;
    }

    SC_HANDLE scService = CreateService(
        scManager,
        serviceName,
        serviceName,
        SERVICE_START | DELETE | SERVICE_STOP,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_IGNORE,
        driverPath.generic_string().c_str(),
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );

    if (!StartService(scService, 0, NULL)) {
        CloseServiceHandle(scService);
        CloseServiceHandle(scManager);
        exit(-5);
        return false;
    }

    std::cout << "Service started successfully." << std::endl;

    CloseServiceHandle(scService);
    CloseServiceHandle(scManager);
    return true;
}
