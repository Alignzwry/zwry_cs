#pragma warning (disable : 4047 4024 4022)
#include "memory.h"

NTSTATUS KernelReadVirtualMemory(PEPROCESS Process, DWORD64 SourceAddress, PVOID TargetAdress, SIZE_T Size)
{
	PSIZE_T Bytes;

	return MmCopyVirtualMemory(Process, (PVOID)SourceAddress, PsGetCurrentProcess(), TargetAdress, Size, KernelMode, &Bytes);
}

NTSTATUS KernelWriteVirtualMemory(PEPROCESS Process, PVOID SourceAddress, DWORD64 TargetAdress, SIZE_T Size)
{
	PSIZE_T Bytes;

	return MmCopyVirtualMemory(PsGetCurrentProcess(), SourceAddress, Process, (PVOID)TargetAdress, Size, KernelMode, &Bytes);
}