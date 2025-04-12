// Tiny USB HID callback entry points to avoid wrong weak attribute usabe in Adafruit TinyUSB

#include <Arduino.h>

#if defined(USBKBD)
void my_hid_mount_cb(uint8_t, uint8_t, uint8_t const *, uint16_t);
void my_hid_umount_cb(uint8_t, uint8_t);
void my_hid_report_received_cb(uint8_t, uint8_t, uint8_t const *, uint16_t);

extern "C"
void 
tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len)
{
    my_hid_mount_cb(dev_addr, instance, desc_report, desc_len);
}

extern "C"
void 
tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{ 
    my_hid_umount_cb(dev_addr, instance);
}

extern "C"
void 
tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len)
{
    my_hid_report_received_cb(dev_addr, instance, report, len);
}
#endif