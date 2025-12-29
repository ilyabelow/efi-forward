#include <efi.h>
#include <efilib.h>

#ifndef PART_UUID
#define PART_UUID L"unknown"
#endif

#ifndef TARGET
#define TARGET L"unknown"
#endif


EFI_STATUS
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  InitializeLib(ImageHandle, SystemTable);
  Print(L"Forwarding to %s/%s\n", PART_UUID, TARGET);

  UINTN handlesCount;
  EFI_HANDLE *handles;
  EFI_STATUS res = uefi_call_wrapper(BS->LocateHandleBuffer, 5,
    ByProtocol,
    &gEfiBlockIoProtocolGuid,
    NULL,
    &handlesCount,
    &handles
  );
  if (res != EFI_SUCCESS) {
    Print(L"fail! %d\n", res);
    return EFI_ABORTED;

  }
  Print(L"success! %d handles found\n", handlesCount);

  for (UINTN i = 0; i < handlesCount; i++) {
    EFI_DEVICE_PATH *path_protocol = DevicePathFromHandle(handles[i]);
    // Print
    EFI_DEVICE_PATH_TO_TEXT_PROTOCOL *text_protocol;
    uefi_call_wrapper(BS->LocateProtocol, 3, &gEfiDevicePathToTextProtocolGuid, NULL, (VOID**)&text_protocol);
    CHAR16* path = uefi_call_wrapper(text_protocol->ConvertDevicePathToText, 3, path_protocol, TRUE, TRUE);
    Print(L"path: %s\n", path);
    
    Print(L"Now iteratively\n", path);
    
    while (!IsDevicePathEnd(path_protocol)) {
      path = uefi_call_wrapper(text_protocol->ConvertDeviceNodeToText, 3, path_protocol, TRUE, TRUE);
      Print(L"path: %s\n", path);
      Print(L"%d: type %u subtype %u length %d\n", i, path_protocol->Type, path_protocol->SubType, DevicePathNodeLength(path_protocol));

      path_protocol = NextDevicePathNode(path_protocol);
    }
  }

  uefi_call_wrapper(BS->FreePool, 1, (VOID*)handles);
  return EFI_SUCCESS;
}
