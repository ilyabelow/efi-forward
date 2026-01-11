#include <efi.h>
#include <efilib.h>

#ifndef PART_UUID
#define PART_UUID u"00000000-0000-0000-0000-000000000000"
#endif

#ifndef TARGET
#define TARGET u"unknown"
#endif


EFI_HANDLE get_partition_handle_by_guid(CHAR16 *reference_guid_str) {
  EFI_HANDLE result = NULL;

  UINTN handlesCount = 0;
  EFI_HANDLE *handles = NULL;
  uefi_call_wrapper(BS->LocateHandleBuffer, 5, ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &handlesCount, &handles);

  CHAR16 guid_str[36];

  for (UINTN i = 0; i < handlesCount && result == NULL; i++) {
    EFI_DEVICE_PATH *device_path = DevicePathFromHandle(handles[i]);
        
    while (!IsDevicePathEnd(device_path) && result == NULL) {
      if (device_path->Type == MEDIA_DEVICE_PATH && device_path->SubType == MEDIA_HARDDRIVE_DP) {
        HARDDRIVE_DEVICE_PATH *hd_path = (HARDDRIVE_DEVICE_PATH *)device_path;
        if (hd_path->SignatureType == SIGNATURE_TYPE_GUID) {
          EFI_GUID *guid = (EFI_GUID *)hd_path->Signature;
          // Yes, converting binary representation to a string and then comparing strings is ugly,
          // but gnu-efi only comes with GuidToString, StrToGuid from edk2 is absent unfortunately
          GuidToString(guid_str, guid);
          Print(u"Got GUID %s\n", guid_str);
          if (StrCmp(guid_str, reference_guid_str) == 0) {
            result = handles[i];
            Print(u"and it is what we are looking for!\n");
          }
        }
      }
      device_path = NextDevicePathNode(device_path);
    }
  }

  uefi_call_wrapper(BS->FreePool, 1, (VOID*)handles);

  return result;
}

EFI_STATUS
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  InitializeLib(ImageHandle, SystemTable);
  Print(u"Forwarding to %s/%s\n", PART_UUID, TARGET);

  EFI_HANDLE partition_handle = get_partition_handle_by_guid(PART_UUID);

  if (partition_handle == NULL) {
    Print(u"Partition with such GUID not found\n");
    return EFI_NO_MEDIA;
  }

  Print(u"Found handle\n");

  // TODO: enter into the file system
  // TODO: load another image

  return EFI_SUCCESS;
}
