// common header for driver and client
#pragma once

#define DRIVER_NAME                 L"capture_network_driver"
#define DRIVER_FULL_NAME            L"\\Device\\"DRIVER_NAME
#define DRIVER_SYMBOL_NAME          L"\\DosDevices"DRIVER_NAME
#define DRIVER_SYMBOL_NAME_FOR_USER L"\\\\.\\"DRIVER_NAME

// control code
#define FSCTL_NDISPROT_BASE      FILE_DEVICE_NETWORK

#define _NDISPROT_CTL_CODE(_Function, _Method, _Access)  \
            CTL_CODE(FSCTL_NDISPROT_BASE, _Function, _Method, _Access)

#define IOCTL_NDISPROT_OPEN_DEVICE   \
            _NDISPROT_CTL_CODE(0x200, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISPROT_QUERY_OID_VALUE   \
            _NDISPROT_CTL_CODE(0x201, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISPROT_SET_OID_VALUE   \
            _NDISPROT_CTL_CODE(0x205, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISPROT_QUERY_BINDING   \
            _NDISPROT_CTL_CODE(0x203, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISPROT_BIND_WAIT   \
            _NDISPROT_CTL_CODE(0x204, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

// struct for query binding adapters
#define MAX_ADAPTER_NAME_SIZE 123
typedef struct _AdapterInfo
{
    unsigned short AdapterName[MAX_ADAPTER_NAME_SIZE];
    unsigned short AdapterDescription[MAX_ADAPTER_NAME_SIZE];
}AdapterInfo;

typedef struct _AdapterInfoBuff
{
    unsigned int    nCount;
    AdapterInfo     adapterInfo[0];
}AdapterInfoBuff;

#define BUFF_SIZE_FOR_ADAPTER_INFO_BUFF(count) (sizeof(AdapterInfoBuff) + count * sizeof(AdapterInfo))

// Eth info
#define NPROT_MAC_ADDR_LEN      6
#define MAX_PACKET_SIZE         256

