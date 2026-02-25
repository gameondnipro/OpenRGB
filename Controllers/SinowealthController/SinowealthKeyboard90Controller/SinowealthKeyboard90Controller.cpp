/*------------------------------------------*\
|  SinowealthKeyboard90Controller.cpp        |
|                                            |
|  Definitions and types for Sinowealth      |
|  Keyboard with PID:0090,                   |
|  made spefically for Genesis Thor 300      |
|                                            |
|  Jan Baier 30/06/2022                      |
\*-----------------------------------------=*/

#include <cstring>
#include "LogManager.h"
#include "SinowealthKeyboard90Controller.h"
#include "StringUtils.h"

using namespace thor300;

SinowealthKeyboard90Controller::SinowealthKeyboard90Controller(hid_device* dev_handle, const char* path, const unsigned short pid, std::string dev_name)
{
    dev             = dev_handle;
    location        = path;
    name            = dev_name;
    usb_pid         = pid;
}

SinowealthKeyboard90Controller::~SinowealthKeyboard90Controller()
{
    hid_close(dev);
}

std::string SinowealthKeyboard90Controller::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string SinowealthKeyboard90Controller::GetNameString()
{
    return(name);
}

std::string SinowealthKeyboard90Controller::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

unsigned short SinowealthKeyboard90Controller::GetUSBPID()
{
    return(usb_pid);
}

void SinowealthKeyboard90Controller::SendFeatureReport
    (
    unsigned char cmd,
    unsigned char arg1,
    unsigned char arg2,
    unsigned char arg3,
    unsigned char arg4,
    unsigned char arg5
    )
{
    unsigned char usb_buf[8];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up control packet                                 |
    \*-----------------------------------------------------*/
    usb_buf[0x00] = 0x0A;
    usb_buf[0x01] = cmd;
    usb_buf[0x02] = arg1;
    usb_buf[0x03] = arg2;
    usb_buf[0x04] = arg3;
    usb_buf[0x05] = arg4;
    usb_buf[0x06] = arg5;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_send_feature_report(dev, (unsigned char *)usb_buf, sizeof(usb_buf));
}

void SinowealthKeyboard90Controller::SendMode
    (
    unsigned char mode,
    unsigned char brightness,
    unsigned char speed,
    unsigned char color
    )
{
    SendFeatureReport(0x03, 0x01);
    SendFeatureReport(0x0A, mode, brightness, speed, color);
}

void SinowealthKeyboard90Controller::SendSingleLED
    (
    unsigned char key,
    unsigned char red,
    unsigned char green,
    unsigned char blue
    )
{
    SendFeatureReport(0x0C, 0x01, key, red, green, blue);
}

void SinowealthKeyboard90Controller::SendCommit()
{
    SendSingleLED(0x89);
}

void SinowealthKeyboard90Controller::SendAllLeds(unsigned char* color_data, unsigned int data_size)
{
    // Буфер: 1 байт Report ID + 256 байт данных = 257
    std::vector<unsigned char> usb_buf(257, 0x00);

    // Заголовок из твоего лога (07 82 01 00 00 68 00 00 01)
    usb_buf[0] = 0x07; // Report ID
    usb_buf[1] = 0x82; // Command
    usb_buf[2] = 0x01; // Sub-command для статичного цвета
    usb_buf[3] = 0x00;
    usb_buf[4] = 0x00;
    usb_buf[5] = 0x68; // Это, вероятно, "длина" данных или тип эффекта
    usb_buf[6] = 0x00;
    usb_buf[7] = 0x00;
    usb_buf[8] = 0x01;

    // В логе софта цвета начинаются примерно с 60-го байта (где FF FF FF).
    // Давай для теста зальем массив с 9-го по 256-й байт красным цветом.
    // Если порядок RGB, то это будет FF 00 00.
    for(int i = 9; i < 250; i += 3) {
        usb_buf[i]     = 0xFF; // R
        usb_buf[i + 1] = 0x00; // G
        usb_buf[i + 2] = 0x00; // B
    }

    // ВАЖНО: Убедись, что в SinowealthControllerDetect.cpp стоит Interface 2
    int ret = hid_send_feature_report(dev, usb_buf.data(), usb_buf.size());

    if (ret < 0) {
        LOG_DEBUG("[Daylight 87] Failed! Error: %ls", hid_error(dev));
    }
}
