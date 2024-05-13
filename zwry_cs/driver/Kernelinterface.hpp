#pragma once

#include "Communications.hpp"
#include <iostream>

class KernelInterface
{
public:
    HANDLE hDriver;

    // Constructor
    KernelInterface(LPCSTR RegistryPath) : hDriver(INVALID_HANDLE_VALUE)
    {
        hDriver = CreateFileA(RegistryPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    }


    // Destructor
    ~KernelInterface()
    {
        if (hDriver != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hDriver);
        }
    }

    // Function to set image buffer
    bool SetImageBuffer(const wchar_t* ImageBuffer)
    {
        if (hDriver == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        DWORD Bytes;
        size_t Size = wcslen(ImageBuffer);

        if (DeviceIoControl(hDriver, IO_SET_IMAGEBUFFER, const_cast<wchar_t*>(ImageBuffer), Size * sizeof(wchar_t), nullptr, 0, &Bytes, nullptr))
        {
            return true;
        }

        // Add error handling if necessary
        std::cerr << "SetImageBuffer failed: " << GetLastError() << std::endl;
        return false;
    }

    // Function to read virtual memory
    template <typename type>
    type ReadVirtualMemory(ULONG ProcessId, DWORD ReadAddress, SIZE_T Size)
    {
        type Buffer = {};

        KERNEL_READ_REQUEST ReadReq;
        ReadReq.ProcessId = ProcessId;
        ReadReq.Address = ReadAddress;
        ReadReq.pBuff = &Buffer;
        ReadReq.Size = Size;

        DWORD Bytes;

        if (DeviceIoControl(hDriver, IO_READ_REQUEST, &ReadReq, sizeof(ReadReq), &ReadReq, sizeof(ReadReq), &Bytes, nullptr))
        {
            return Buffer;
        }

        // Add error handling if necessary
        std::cerr << "ReadVirtualMemory failed: " << GetLastError() << std::endl;
        return Buffer;
    }

    template<typename T>
    void ReadVirtualMemory(DWORD ProcessId, DWORD Address, T* Buffer, SIZE_T Size)
    {
        KERNEL_READ_REQUEST ReadReq;
        ReadReq.ProcessId = ProcessId;
        ReadReq.Address = Address;
        ReadReq.pBuff = Buffer;
        ReadReq.Size = Size;

        DeviceIoControl(hDriver, IO_READ_REQUEST, &ReadReq, sizeof(ReadReq), nullptr, 0, nullptr, nullptr);
    }


    // Function to write virtual memory
    template <typename type>
    bool WriteVirtualMemory(ULONG ProcessId, ULONG WriteAddress, type WriteValue, SIZE_T Size)
    {
        if (hDriver == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        DWORD Bytes;

        KERNEL_WRITE_REQUEST WriteReq;
        WriteReq.ProcessId = ProcessId;
        WriteReq.Address = WriteAddress;
        WriteReq.pBuff = &WriteValue;
        WriteReq.Size = Size;

        if (DeviceIoControl(hDriver, IO_WRITE_REQUEST, &WriteReq, sizeof(WriteReq), nullptr, 0, &Bytes, nullptr))
        {
            return true;
        }

        // Add error handling if necessary
        std::cerr << "WriteVirtualMemory failed: " << GetLastError() << std::endl;
        return false;
    }
};
