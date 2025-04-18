// Keyboard class

#include <keyboard.h>

#ifdef PICOCALC
PicoCalcKeyBoard::PicoCalcKeyBoard()
    : KeyBoard(), _i2c_inited(1), _keycheck(0) 
{
#if 0
    Wire1.setSCL(I2C_KBD_SCL);
    Wire1.setSDA(I2C_KBD_SDA);
    Wire1.setClock(I2C_KBD_SPEED);

    Wire1.begin();
#else
    gpio_set_function(I2C_KBD_SCL, GPIO_FUNC_I2C);
    gpio_set_function(I2C_KBD_SDA, GPIO_FUNC_I2C);
    i2c_init(I2C_KBD_MOD, I2C_KBD_SPEED);
    gpio_pull_up(I2C_KBD_SCL);
    gpio_pull_up(I2C_KBD_SDA);
#endif
}

bool 
PicoCalcKeyBoard::wait_any_key()
{
    bool r = false;
    int k = _read_i2c_kbd();
    if (k > 0)
    {
        r = true;
    }
    return r;
}

bool 
PicoCalcKeyBoard::fetch_key(uint8_t &c)
{
    int k = _read_i2c_kbd();
    bool r = false;
    if (k > 0)
    {
        c = (uint8_t)k;
        r = true;
    }
    return r;
}

int 
PicoCalcKeyBoard::_write_i2c_kbd() 
{
    int retval;
    unsigned char msg[2];
    msg[0] = 0x09;

    if(_i2c_inited == 0) return -1;

    retval = i2c_write_timeout_us(I2C_KBD_MOD, I2C_KBD_ADDR, msg, 1, false, 500000);
    if ( retval == PICO_ERROR_GENERIC || retval == PICO_ERROR_TIMEOUT) 
    {
        Serial.printf( "i2c write error\r\n");
        return -1;
    }
    return 0;
}

int 
PicoCalcKeyBoard::_read_i2c_kbd() 
{
    int retval;
    static int ctrlheld = 0;
    uint16_t buff = 0;
    unsigned char msg[2];
    int c  = -1;
    msg[0] = 0x09;

    if(_i2c_inited == 0) return -1;

    if(_keycheck == 0)
    {
        retval = _write_i2c_kbd();
        _keycheck = 1;
        return retval;
    }
    else 
    {
        retval = i2c_read_timeout_us(I2C_KBD_MOD, I2C_KBD_ADDR, (unsigned char *) &buff, 2, false, 500000);
        if (retval == PICO_ERROR_GENERIC || retval == PICO_ERROR_TIMEOUT) 
        {
            Serial.printf("i2c read error read\n");
            return -1;
        }
        _keycheck = 0;
    }
    if(buff != 0) 
    {
        if (buff == 0xA503) ctrlheld = 0;
        else if (buff == 0xA502) 
        {
            ctrlheld = 1;
        }
        else if((buff & 0xff) == 1) 
        {//pressed
            c = buff >> 8;
            int realc = -1;
            switch (c) 
            {
            case 0xA1:
            case 0xA2:
            case 0xA3:
            case 0xA4:
            case 0xA5:
                realc = -1;//skip shift alt ctrl keys
                break;
            default:
                realc = c;
                break;
            }
            c = realc;
            if(c >= 'a' && c <= 'z' && ctrlheld) c = c - 'a' + 1;
        }
        return c;
    }
    return -1;
}
#endif

#ifdef USBKBD
#ifdef TUH_OPT_HIGH_SPEED
#undef TUH_OPT_HIGH_SPEED
#endif
#define TUH_OPT_HIGH_SPEED 0 // FULL speed mode

#include <tusb.h>
#include <queue>

#define MAX_REPORT 4
static struct {
  uint8_t report_count;
  tuh_hid_report_info_t report_info[MAX_REPORT];
} hid_info[CFG_TUH_HID];

// Adafruit_USBH_Host USBHost;
static uint8_t const keycode2ascii[128][2] = { HID_KEYCODE_TO_ASCII };
std::queue<uint8_t> keybuf; // キーコードを格納するキュー
//static semaphore_t keybuf_sem = { 0 };


static inline bool 
find_key_in_report(hid_keyboard_report_t const *report, uint8_t keycode) 
{
    // 前回のレポートにキーコードが存在するかを確認する関数
    for (int i = 0; i < 6; i++) {
        if (report->keycode[i] == keycode) {
            return true;
        }
    }
    return false;
}

void 
process_kbd_report(hid_keyboard_report_t const *report) 
{
    // キーボードのレポートを処理する関数
    static hid_keyboard_report_t prev_report = { 0, 0, {0} }; // 前回のレポートを保存する変数
    // report->keycode[]にキーコードが格納されている
    for (int i = 0; i < 6; i++) 
    {
        if (report->keycode[i] != 0) 
        {
            //Serrial1.printf("Key Pressed: %d\r\n", report->keycode[i]);
            if (find_key_in_report(&prev_report, report->keycode[i])) 
            {
                // 前回のレポートに同じキーが存在する場合、キーは押され続けている
            } 
            else 
            {
                // 前回のレポートに同じキーが存在しない場合、キーが押された
                bool is_shift = report->modifier & (KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT);
                uint8_t ch = keycode2ascii[report->keycode[i]][is_shift ? 1 : 0];
                if (ch == 0) 
                {
                    // キーコードが無効な場合、何もしない
                    continue;
                }
                //sem_acquire_blocking(&keybuf_sem); // セマフォを取得
                keybuf.push(ch); // キーコードをキューに追加
                //sem_release(&keybuf_sem); // セマフォを解放
                //Serrial1.printf("Key Pressed: %c\r\n", ch);
            }
        }
    }
    prev_report = *report; // 現在のレポートを保存
}

void 
process_mouse_report(hid_mouse_report_t const * report) 
{
    // マウスのレポートを処理する関数
    static hid_mouse_report_t prev_report = { 0 };
    uint8_t button_changed_mask = report->buttons ^ prev_report.buttons;
    if (button_changed_mask & report->buttons) 
    {
        //Serrial1.printf("Mouse Button: %c%c%c\r\n",
        //    report->buttons & MOUSE_BUTTON_LEFT ? 'L' : '-',
        //    report->buttons & MOUSE_BUTTON_MIDDLE ? 'M' : '-',
        //    report->buttons & MOUSE_BUTTON_RIGHT ? 'R' : '-');
    }
    //Serrial1.printf("Mouse Movement: (%d, %d, %d)\r\n", report->x, report->y, report->wheel);
    prev_report = *report; // 現在のレポートを保存
}

void 
process_generic_report(uint8_t dev_addr,uint8_t instance, uint8_t const *report, uint16_t len)
{
    // 一般的なレポートを処理する関数
    uint8_t const rpt_count = hid_info[instance].report_count;
    tuh_hid_report_info_t *rpt_info_arr = hid_info[instance].report_info;
    tuh_hid_report_info_t *rpt_info = NULL;

    if (rpt_count == 1 && rpt_info_arr[0].report_id == 0) 
    {
        // 単純なレポート
        rpt_info = &rpt_info_arr[0];
    } 
    else 
    {
        // 複合レポート
        uint8_t const rpt_id = report[0];
        for (uint8_t i = 0; i < rpt_count; i++) 
        {
            if (rpt_id == rpt_info_arr[i].report_id) 
            {
                rpt_info = &rpt_info_arr[i];
                break;
            }
        }
        report++;
        len--;
    }

    if (!rpt_info) 
    {
        //Serrial1.printf("Couldn't find report info!\r\n");
        return;
    }

    // レポートの内容を表示
    //Serrial1.printf("Generic Report: ID %d, Length %d\r\n", rpt_info->report_id, len);
}

void 
my_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len) 
{
    //Serrial1.printf("HID device address = %d, instance = %d is mounted\r\n", dev_addr, instance);
  
    // Interface protocol (hid_interface_protocol_enum_t)
    const char *protocol_str[] = {"None", "Keyboard", "Mouse"};
    uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
  
    //Serrial1.printf("HID Interface Protocol = %s\r\n", protocol_str[itf_protocol]);
  
    // By default host stack will use activate boot protocol on supported interface.
    // Therefore for this simple example, we only need to parse generic report descriptor (with built-in parser)
    if (itf_protocol == HID_ITF_PROTOCOL_NONE) 
    {
      hid_info[instance].report_count = tuh_hid_parse_report_descriptor(hid_info[instance].report_info, MAX_REPORT, desc_report, desc_len);
      //Serrial1.printf("HID has %u reports \r\n", hid_info[instance].report_count);
    }
  
    // request to receive report
    // tuh_hid_report_received_cb() will be invoked when report is available
    if (!tuh_hid_receive_report(dev_addr, instance)) 
    {
      //Serrial1.printf("Error: cannot request to receive report\r\n");
    }
}
  
// Invoked when device with hid interface is un-mounted
void 
my_hid_umount_cb(uint8_t dev_addr, uint8_t instance) 
{
    //Serrial1.printf("HID device address = %d, instance = %d is unmounted\r\n", dev_addr, instance);
}
  
void 
my_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len)
{
    uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
    if (itf_protocol == HID_ITF_PROTOCOL_KEYBOARD) 
    {
        // Process keyboard report
        process_kbd_report((hid_keyboard_report_t const *) report);
    } 
    else if (itf_protocol == HID_ITF_PROTOCOL_MOUSE) 
    {
        // Process mouse report
        process_mouse_report((hid_mouse_report_t const *) report);
    } 
    else 
    {
        // Process generic report
        process_generic_report(dev_addr, instance, report, len);
    }
    // Request to receive next report
    if (!tuh_hid_receive_report(dev_addr, instance)) 
    {
        //Serrial1.printf("Error: cannot request to receive report\r\n");
    }
}

//static void
//core1_entry()
//{
//    tuh_task();
//}

USBKeyBoard::USBKeyBoard()
    : KeyBoard()
{
    //Serial.println("USB keyboard initialized.");
    tuh_init(0);    // Initialize TinyUSB stack
    //sem_init(&keybuf_sem, 1, 1); // Initialize semaphore for key buffer
    //multicore_launch_core1(core1_entry);
}

void
USBKeyBoard::update()
{
    // Poll USB host stack
    tuh_task();
}

bool
USBKeyBoard::wait_any_key()
{
    update();
    if (keybuf.empty()) return false;
    //sem_acquire_blocking(&keybuf_sem); // Acquire semaphore for key buffer
    keybuf.pop(); // just drop one key stroke.
    //sem_release(&keybuf_sem); // Release semaphore for key buffer
    return true;
}

bool
USBKeyBoard::fetch_key(uint8_t &c)
{
    update();
    if (keybuf.empty()) return false;
    //sem_acquire_blocking(&keybuf_sem); // Acquire semaphore for key buffer
    c = keybuf.front();
    keybuf.pop();
    //sem_release(&keybuf_sem); // Release semaphore for key buffer
    return true;
}
#endif

#if defined(BLEKBD)

// Define ATT_ERROR_SECURITY_ERROR if not already defined
#ifndef ATT_ERROR_SECURITY_ERROR
#define ATT_ERROR_SECURITY_ERROR 0x0E // Replace 0x0E with the correct error code if needed
#endif

#include <queue>
#include <list>

#ifndef AD_DATA_ITERATOR_T_DEFINED
#define AD_DATA_ITERATOR_T_DEFINED
typedef struct 
{
    const uint8_t *data;
    uint16_t size;
    uint16_t offset;
} ad_data_iterator_t;
#endif

// 定数定義

static bd_addr_t keyboard_address;
static int found_hid_device = 0;
static hci_con_handle_t keyboard_handle = 0;
static gatt_client_service_t hid_service;
static bool hid_characteristics_scan_completed = false;
static std::list<gatt_client_characteristic_t> hid_characteristics;
static std::list<gatt_client_characteristic_descriptor_t> hid_descriptors;
static std::list<gatt_client_characteristic_t>::iterator hid_characteristic_it;
static std::list<gatt_client_characteristic_descriptor_t>::iterator hid_descriptor_it;
static gatt_client_characteristic_t cur_characteristic;

static gatt_client_characteristic_descriptor_t gatt_cccd_descriptor;
static uint8_t notification_enable[] = {0x01, 0x00}; // 通知を有効化する値

static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_packet_callback_registration_t sm_event_callback_registration;

static gatt_client_notification_t notification_listener;
static bool secure_connection = true;

const uint32_t FIXED_PASSKEY = 123456U; // パスキーの固定値
const uint16_t HID_SERVICE_UUID = 0x1812;
const uint16_t HID_REPORT_ID = 0x2a4d; // HID Report ID

// callback
static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static void handle_gatt_descriptors_discovered(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

typedef union {
  struct __attribute__((__packed__))
  {
      uint8_t modifiers;
      uint8_t keys[10];
  } k1;
  struct __attribute__((__packed__))
  {
      uint8_t modifiers;
      uint8_t reserved;
      uint8_t keys[6];
      uint8_t padding[3];
  } k2;
  uint8_t raw[11];
} keyboard_t;

static keyboard_t keyboardReport;
static std::queue<uint8_t> keybuf;
static const int MAX_KEYCODE = 96;
const uint8_t keymap[][MAX_KEYCODE] = {
  {    0,   0,   0,   0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
     'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2',
     '3', '4', '5', '6', '7', '8', '9', '0',  13,  27,   8,   9, ' ', '-', '=', '[',
     ']','\\',   0, ';','\'', '`', ',', '.', '/',   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 127,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  },
  {
       0,   0,   0,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,
      13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  },
  {
       0,   0,   0,   0, 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
     'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@',
     '#', '$', '%', '^', '&', '*', '(', ')',  13,  27,   8,   9, ' ', '_', '+', '{',
     '}', '|',   0, ':', '"', '~', '<', '>', '?',   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 127,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  },
};

// Define the ad_data_iterator_next function
static int 
ad_data_iterator_next(ad_data_iterator_t *it) 
{
  if (it->offset >= it->size) return 0;
  uint8_t field_length = it->data[it->offset];
  it->offset += field_length + 1;
  return it->offset < it->size;
}

// 広告パケットを解析してHID UUIDを確認
static void 
scan_advertisements(const uint8_t *packet, uint16_t size) 
{
    ad_data_iterator_t ad_iter;
    uint16_t uuid;
    
    // 広告データの反復処理
    for (ad_iter.data = packet, ad_iter.size = size, ad_iter.offset = 0 ; ad_iter.offset < ad_iter.size ; ad_data_iterator_next(&ad_iter)) 
    {
      if (ad_iter.data[ad_iter.offset + 1] == BLUETOOTH_DATA_TYPE_SHORTENED_LOCAL_NAME||
          ad_iter.data[ad_iter.offset + 1] == BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME) 
      {
        // デバイス名を取得
        char device_name[32];
        memcpy(device_name, &ad_iter.data[ad_iter.offset + 2], ad_iter.data[ad_iter.offset] - 1);
        device_name[ad_iter.data[ad_iter.offset] - 1] = '\0';
        if (strstr(device_name, "M5-Keyboard") != NULL) 
        {
          secure_connection = false; // セキュアコネクションを無効化
          continue;
        }
      }
      // 16ビットUUIDを確認
      if (ad_iter.data[ad_iter.offset + 1] == BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS ||
          ad_iter.data[ad_iter.offset + 1] == BLUETOOTH_DATA_TYPE_INCOMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS) 
      {
        uuid = little_endian_read_16(&ad_iter.data[ad_iter.offset + 2], 0);
        if (uuid == HID_SERVICE_UUID) 
        {
          found_hid_device = 1;
          continue;
        }
      }
    }
}

// 通知を受け取るハンドラ
static void 
notification_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) 
{
  if (packet_type != HCI_EVENT_PACKET) return;
  switch (hci_event_packet_get_type(packet)) 
  {
    case GATT_EVENT_NOTIFICATION:
      {
        // 通知を受信した場合の処理
        uint16_t attribute_handle = gatt_event_notification_get_handle(packet);
        uint16_t value_length = gatt_event_notification_get_value_length(packet);
        const uint8_t *value = gatt_event_notification_get_value(packet);
        uint16_t value_handle = gatt_event_notification_get_value_handle(packet);
        uint16_t service_id = gatt_event_notification_get_service_id(packet);
        if (attribute_handle == 0x0040)
        {
          if (value_length == 0) return; // データがない場合は無視
          if (value_length == 8 || value_length == 11)
          {
            keyboard_t *newKeyReport = (keyboard_t*)value;
            int buflen = 6;
            uint8_t *buf = keyboardReport.k2.keys;
            uint8_t *input = newKeyReport->k2.keys;
            uint8_t mod = newKeyReport->k2.modifiers;
            if (value_length == 11)
            {
              buflen = 10;
              buf = keyboardReport.k1.keys;
              input = newKeyReport->k1.keys;
              mod = newKeyReport->k1.modifiers;     
            }
            for (int i = 0 ; i < buflen ; i++)
            {
              uint8_t c = input[i];
              if (c == 0) continue;
              if (mod == 3) mod = 1;
              uint8_t ch = keymap[mod][c];
              if (ch == 0) continue;
              if (memchr(buf, c, buflen) == NULL) keybuf.push(ch);
            }
            memcpy(&keyboardReport, value, value_length);
          }
        }
      }
      break;
    default:
      break;
  }
}

// CCCDにNotificationを要求し完了した
static void
handle_gatt_noification_activated(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
  if (packet_type != HCI_EVENT_PACKET) return;
  switch (hci_event_packet_get_type(packet)) 
  {
    case GATT_EVENT_QUERY_COMPLETE:
      {
        uint16_t status = gatt_event_query_complete_get_att_status(packet); // Replace with the correct function
        if (status != 0) {
          Serial.printf("GATT Notification activation failed with status %u\n", status);
        } else {
          uint16_t service_id = gatt_event_query_complete_get_service_id(packet); // Retrieve service ID
          uint16_t handle = gatt_event_query_complete_get_handle(packet); // Retrieve handle
          hci_con_handle_t connection_handle = gatt_event_query_complete_get_handle(packet); // Retrieve connection handle
          while (hid_descriptor_it != hid_descriptors.end())
          {
            uint16_t uuid16 = hid_descriptor_it->uuid16;
            if (uuid16 == 0)
            {
              uuid16 = little_endian_read_16(hid_descriptor_it->uuid128, 0);
            }
            if (uuid16 == 0x2902) // UUIDがClient Characteristic Configurationの場合
            {
              gatt_cccd_descriptor = *hid_descriptor_it++; // Client Characteristic Configuration Descriptorを保存
              // Notificationを有効化する値を書き込む
              gatt_client_write_value_of_characteristic(&handle_gatt_noification_activated, connection_handle, gatt_cccd_descriptor.handle, sizeof(notification_enable), notification_enable);
              break;
            }
            hid_descriptor_it++; // イテレータを保存
          }
          if (hid_descriptor_it == hid_descriptors.end()) // HID Reportのイテレータを進める
          {
            hid_descriptors.clear(); // HID Reportのイテレータをクリア
            while (hid_characteristic_it != hid_characteristics.end())
            {
              // HID ReportのUUIDを確認
              if (hid_characteristic_it->uuid16 == 0x2a4d) { // UUIDがHID_REPORT_DATAの場合
                uint16_t characteristic_handle = hid_characteristic_it->value_handle;
                uint8_t properties = hid_characteristic_it->properties;
                //display.printf("Found HID Report characteristic: %04x\n", it->value_handle);
                if (properties & ATT_PROPERTY_NOTIFY) { // Notificationをサポートしているか確認
                  cur_characteristic = *hid_characteristic_it; // HID Reportを保存
                  gatt_client_discover_characteristic_descriptors(&handle_gatt_descriptors_discovered, connection_handle, &*hid_characteristic_it++);
                  break;
                }
              }
              hid_characteristic_it++; // イテレータを保存
            }
            if (hid_characteristic_it == hid_characteristics.end())
            {
              gatt_client_listen_for_characteristic_value_updates(&notification_listener, 
                &notification_handler, 
                connection_handle, 
                nullptr);
            }
          }
        }
      }
      break;
    default:
      break;

  }
}

// descriptor を見つけた
static void
handle_gatt_descriptors_discovered(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
  if (packet_type != HCI_EVENT_PACKET) return;
  switch (hci_event_packet_get_type(packet)) 
  {
    case GATT_EVENT_ALL_CHARACTERISTIC_DESCRIPTORS_QUERY_RESULT:
      {
        // Descriptorの情報を取得
        gatt_client_characteristic_descriptor_t descriptor;
        gatt_event_all_characteristic_descriptors_query_result_get_characteristic_descriptor(packet, &descriptor);
        hid_descriptors.push_back(descriptor); // HID Reportを保存
      }
      break;
    case GATT_EVENT_QUERY_COMPLETE:
      {
        uint16_t status = gatt_event_query_complete_get_att_status(packet); // Replace with the correct function
        if (status != 0) 
        {
          Serial.printf("GATT Descriptor query failed with status %u\n", status);
        } 
        else 
        {
          // Notificationを有効化する値を書き込む
          hci_con_handle_t connection_handle = gatt_event_query_complete_get_handle(packet);
          for(hid_descriptor_it = hid_descriptors.begin(); hid_descriptor_it != hid_descriptors.end(); ++hid_descriptor_it) 
          {
            uint16_t uuid16 = hid_descriptor_it->uuid16;
            if (uuid16 == 0)
            {
              uuid16 = little_endian_read_16(hid_descriptor_it->uuid128, 0);
            }
            if (uuid16 == 0x2902) 
            { // UUIDがClient Characteristic Configurationの場合
              gatt_cccd_descriptor = *hid_descriptor_it; // Client Characteristic Configuration Descriptorを保存
              hid_descriptor_it++; // イテレータを保存
              gatt_client_write_value_of_characteristic(&handle_gatt_noification_activated, connection_handle, gatt_cccd_descriptor.handle, sizeof(notification_enable), notification_enable);
              break;
            }
          }
          if (hid_descriptor_it == hid_descriptors.end()) 
          {
            hid_descriptors.clear(); // HID Reportのイテレータをクリア
            while (hid_characteristic_it != hid_characteristics.end())
            {
              if (hid_characteristic_it->uuid16 == 0x2a4d) { // UUIDがHID_REPORT_DATAの場合
                uint16_t characteristic_handle = hid_characteristic_it->value_handle;
                uint8_t properties = hid_characteristic_it->properties;
                if (properties & ATT_PROPERTY_NOTIFY) { // Notificationをサポートしているか確認
                  cur_characteristic = *hid_characteristic_it; // HID Reportを保存
                  hci_con_handle_t connection_handle = gatt_event_query_complete_get_handle(packet);
                  gatt_client_discover_characteristic_descriptors(&handle_gatt_descriptors_discovered, connection_handle, &*hid_characteristic_it++);
                  break;
                }
              }
              ++hid_characteristic_it; // イテレータを保存
            }
          }
        }
      }
      break;
    default:
      break;
  }
}

// Characteristicを見つけた。
static void
handle_gatt_characteristics_discovered(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
  if (packet_type != HCI_EVENT_PACKET) return;
  switch (hci_event_packet_get_type(packet)) {
    case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT:
      {
        gatt_client_characteristic_t characteristic;
        gatt_event_characteristic_query_result_get_characteristic(packet, &characteristic);
        uint16_t characteristic_handle = characteristic.value_handle;
        uint8_t properties = characteristic.properties;
        hid_characteristics.push_back(characteristic); // HID Reportを保存
      }
      break;
    case GATT_EVENT_QUERY_COMPLETE:
      {
        uint16_t status = gatt_event_query_complete_get_att_status(packet); // Replace with the correct function
        if (status != 0) {
          Serial.printf("GATT Characteristcis query failed with status %u\n", status);
        } else {
          for(hid_characteristic_it = hid_characteristics.begin(); hid_characteristic_it != hid_characteristics.end(); ++hid_characteristic_it) 
          {
            // HID ReportのUUIDを確認
            if (hid_characteristic_it->uuid16 == 0x2a4d) { // UUIDがHID_REPORT_DATAの場合
              uint16_t characteristic_handle = hid_characteristic_it->value_handle;
              uint8_t properties = hid_characteristic_it->properties;
              if (properties & ATT_PROPERTY_NOTIFY) { // Notificationをサポートしているか確認
                hci_con_handle_t connection_handle = gatt_event_query_complete_get_handle(packet);
                gatt_client_discover_characteristic_descriptors(&handle_gatt_descriptors_discovered, connection_handle, &*hid_characteristic_it++);
                break;
              }
            }
          }
        }
      }
      break;
    default:
      break;
  }
}

// GATT関連のイベントの入り口
static void
handle_gatt_client_event(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
  switch(hci_event_packet_get_type(packet)) {
    case GATT_EVENT_SERVICE_QUERY_RESULT:
        {
          // サービスのUUIDを取得
          gatt_client_service_t service;
          gatt_event_service_query_result_get_service(packet, &service);
          hci_con_handle_t connection_handle = gatt_event_service_query_result_get_handle(packet);
          uint16_t uuid16 = service.uuid16;
          const uint8_t *uuid128 = service.uuid128;
      
          if (uuid16 == 0)
          {
            uuid16 = little_endian_read_16(uuid128, 0);
          }
          if (uuid16 == HID_SERVICE_UUID) {
            // characteristicを探索するのはGATT_EVENT_QUERY_COMPLETEで行う必要がある。
            hid_service = service; // HIDサービスを保存
          }
        }
        break;
    case GATT_EVENT_QUERY_COMPLETE:
        {
          uint16_t status = gatt_event_query_complete_get_att_status(packet); // Replace with the correct function
          if (status != 0) {
            Serial.printf("GATT query failed with status %u\n", status);
          } else {
            if (hid_characteristics_scan_completed) break; // HID Reportの探索が完了している場合はスキップ
            // GATTクライアントのサービスを探索する
            hci_con_handle_t connection_handle = gatt_event_query_complete_get_handle(packet); // Retrieve connection handle
            gatt_client_discover_characteristics_for_service(&handle_gatt_characteristics_discovered, connection_handle, &hid_service); // GATT event handler
          }
        }
        break;
    default:
      break;
  }
}

// SMイベントハンドラ
static void
sm_event_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
  if (packet_type != HCI_EVENT_PACKET) return;

  switch (hci_event_packet_get_type(packet)) 
  {
    case SM_EVENT_JUST_WORKS_REQUEST:
      {
        // Just Works Confirmを自動的に承認
        bd_addr_t addr;
        sm_event_just_works_request_get_address(packet, addr); // アドレスを取得
        sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
        break;
      }
    case SM_EVENT_PAIRING_STARTED:
      break;
    case SM_EVENT_PAIRING_COMPLETE: 
    {
      uint8_t status = sm_event_pairing_complete_get_status(packet);
      if (status == ERROR_CODE_SUCCESS) 
      {
        hci_con_handle_t connection_handle = sm_event_pairing_complete_get_handle(packet);
        gatt_client_discover_primary_services_by_uuid16(&handle_gatt_client_event, connection_handle, HID_SERVICE_UUID);  // GATT event handler
      } 
      else 
      {
        Serial.printf("Pairing failed with status %u.\n", status);
      }
      break;
    }
    case SM_EVENT_REENCRYPTION_STARTED:
      {
        // 再暗号化が開始された場合の処理
        bd_addr_t addr;
        sm_event_reencryption_started_get_address(packet, addr); // アドレスを取得
        uint8_t addr_type = sm_event_reencryption_started_get_addr_type(packet);
        break;
      }
    case SM_EVENT_REENCRYPTION_COMPLETE:
      {
        // 再暗号化が完了した場合の処理
        uint8_t status = sm_event_reencryption_complete_get_status(packet);
        if(status != ERROR_CODE_SUCCESS) {
          Serial.printf("Re-encryption failed.(%d)\n", status);
        }
        break;
      }
    default:
      break;
  }
}

// HCIイベントハンドラ
static void
packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) 
{
  if (packet_type != HCI_EVENT_PACKET) return;
  switch (hci_event_packet_get_type(packet)) 
  {
    case BTSTACK_EVENT_STATE:
      if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
        bd_addr_t local_address;
        gap_local_bd_addr(local_address);
        gap_set_scan_parameters(0, 0x0030, 0x0030);
        gap_start_scan();
      }
      break;
    case GAP_EVENT_ADVERTISING_REPORT:
      {
        gap_event_advertising_report_get_address(packet, keyboard_address);
        bd_addr_type_t address_type = static_cast<bd_addr_type_t>(gap_event_advertising_report_get_address_type(packet));
        uint8_t rssi = gap_event_advertising_report_get_rssi(packet);
        uint8_t data_length = gap_event_advertising_report_get_data_length(packet);
        scan_advertisements(gap_event_advertising_report_get_data(packet), gap_event_advertising_report_get_data_length(packet));
        if (found_hid_device) {
          gap_stop_scan();
          gap_connect(keyboard_address, address_type);
          found_hid_device = 0; // リセット
        }
        break;
      }
    case HCI_EVENT_DISCONNECTION_COMPLETE:
      gap_start_scan(); // 再スキャン開始
      break;
    case HCI_EVENT_LE_META:
      if (hci_event_le_meta_get_subevent_code(packet) == HCI_SUBEVENT_LE_CONNECTION_COMPLETE) 
      {
        // 暗号化を有効化
        hci_con_handle_t connection_handle = gap_subevent_le_connection_complete_get_connection_handle(packet);
        gap_request_security_level(connection_handle, LEVEL_2);
        if (secure_connection) 
        {
          // セキュアコネクションを有効化
          sm_request_pairing(connection_handle);
        } 
        else 
        {
          // get primary service
          gatt_client_discover_primary_services_by_uuid16(&handle_gatt_client_event, connection_handle, HID_SERVICE_UUID);  // GATT event handler
        }
      }
      break;
    case HCI_EVENT_ENCRYPTION_CHANGE:
      if (!hci_event_encryption_change_get_encryption_enabled(packet)) 
      {
        Serial.printf("Encryption failed.\n");
      }
      break;
    default: 
      break;
  }
}

bool BLEKeyBoard::_initialied = false;

void
BLEKeyBoard::begin()
{
  // プロトコルの初期化
  btstack_memory_init(); // Initialize BTstack memory
  //hci_init(hci_transport_cyw43_instance(), nullptr); // Initialize HCI
  l2cap_init();                     // L2CAPの初期化
  gatt_client_init();               // GATTクライアントの初期化
  sm_init();                        // セキュリティマネージャの初期化
  btstack_crypto_init();          	// 暗号化の初期化
  sm_set_request_security(true); 	// セキュリティ要求を有効化
  sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT); // IOキャパビリティ設定
  sm_set_authentication_requirements(SM_AUTHREQ_SECURE_CONNECTION); // 認証要件を設定
  sm_set_encryption_key_size_range(7, 16); // 暗号化キーサイズの範囲を設定
  
  // イベントハンドラの登録
  hci_event_callback_registration.callback = &packet_handler;
  hci_add_event_handler(&hci_event_callback_registration);
  sm_event_callback_registration.callback = &sm_event_handler;
  sm_add_event_handler(&sm_event_callback_registration);

  hci_power_control(HCI_POWER_ON);  // BLEモジュールの電源ON

  BLEKeyBoard::_initialied = true; // 初期化完了フラグを立てる
}

bool 
BLEKeyBoard::wait_any_key()
{
	if (keybuf.empty()) return false;
	keybuf.pop();
	return true;
}

bool 
BLEKeyBoard::fetch_key(uint8_t &c)
{
	if (keybuf.empty()) return false;
	c = keybuf.front();
	keybuf.pop();
	return true;
}

#endif