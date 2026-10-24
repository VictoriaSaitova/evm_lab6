#include <iostream>
#include <cstdio>
#include <libusb.h>
#include <windows.h>

using namespace std;


const char *className(uint8_t cls) {
    switch (cls) {
        case 0x00: return "класс в дескрипторе интерфейса";
        case 0x01: return "аудиоустройство";
        case 0x02: return "коммуникационное устройство";
        case 0x03: return "устройство пользовательского интерфейса (HID)";
        case 0x05: return "физическое устройство";
        case 0x06: return "изображения";
        case 0x07: return "принтер";
        case 0x08: return "устройство хранения данных";
        case 0x09: return "концентратор";
        case 0x0A: return "CDC-Data";
        case 0x0B: return "Smart Card";
        case 0x0D: return "Content Security";
        case 0x0E: return "видеоустройство";
        case 0x0F: return "персональное медицинское устройство";
        case 0x10: return "аудио- и видеоустройство";
        case 0xDC: return "диагностическое устройство";
        case 0xE0: return "беспроводной контроллер";
        case 0xEF: return "различные устройства";
        case 0xFE: return "специфическое устройство";
        case 0xFF: return "определяется производителем";
        default:   return "неизвестный класс";
    }
}

// Печать серийного номера устройства (поле iSerialNumber дескриптора)
void printSerial(libusb_device *dev, const libusb_device_descriptor &desc) {
    // iSerialNumber — номер строкового дескриптора; 0 значит, что номера нет
    if (desc.iSerialNumber == 0) {
        printf("       серийный номер: отсутствует\n");
        return;
    }

    // чтобы прочитать строку, устройство нужно открыть
    libusb_device_handle *handle;
    int r = libusb_open(dev, &handle);
    if (r < 0) {
        printf("       серийный номер: не удалось открыть устройство (%s)\n",
               libusb_error_name(r));
        return;
    }

    unsigned char serial[256];   // буфер для строки
    r = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber,
                                           serial, sizeof(serial));
    if (r > 0)                   // r — длина полученной строки
        printf("       серийный номер: %s\n", serial);
    else
        printf("       серийный номер: не прочитан (%s)\n",
               libusb_error_name(r));

    libusb_close(handle);        // закрыть устройство
}

void printdev(libusb_device *dev, int index) {
    libusb_device_descriptor desc;
    int r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) {
        fprintf(stderr, "Ошибка: дескриптор устройства не получен: %s\n",
                libusb_error_name(r));
        return;
    }

    printf("%3d  %02Xh  %04x  %04x  %s\n",
           index,
           desc.bDeviceClass,
           desc.idVendor,
           desc.idProduct,
           className(desc.bDeviceClass));

    printSerial(dev, desc);

    // Если класс задан на уровне интерфейсов (00h) — выводим классы интерфейсов
    if (desc.bDeviceClass == LIBUSB_CLASS_PER_INTERFACE) {
        libusb_config_descriptor *config;
        if (libusb_get_config_descriptor(dev, 0, &config) == 0) {
            for (int i = 0; i < config->bNumInterfaces; i++) {
                const libusb_interface *inter = &config->interface[i];
                for (int j = 0; j < inter->num_altsetting; j++) {
                    const libusb_interface_descriptor *id = &inter->altsetting[j];
                    printf("       └ интерфейс %d: %02Xh  %s\n",
                           id->bInterfaceNumber,
                           id->bInterfaceClass,
                           className(id->bInterfaceClass));
                }
            }
            libusb_free_config_descriptor(config);
        }
    }
}

int main() {

    SetConsoleOutputCP(CP_UTF8);
    libusb_context *ctx = NULL;   // контекст сессии libusb
    libusb_device **devs;         // список устройств

    int r = libusb_init(&ctx);
    if (r < 0) {
        fprintf(stderr, "Ошибка: инициализация не выполнена: %s\n",
                libusb_error_name(r));
        return 1;
    }

    // уровень отладочных сообщений 3 (в новых версиях libusb_set_debug устарела)
#if defined(LIBUSB_API_VERSION) && (LIBUSB_API_VERSION >= 0x01000106)
    libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_WARNING);
#else
    libusb_set_debug(ctx, 3);
#endif

    ssize_t cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0) {
        fprintf(stderr, "Ошибка: список USB-устройств не получен: %s\n",
                libusb_error_name((int)cnt));
        libusb_exit(ctx);
        return 1;
    }

    printf("Найдено устройств: %zd\n", cnt);
    printf("===========================================================\n");
    printf("  №  Класс  VID   PID   Описание класса\n");
    printf("-----------------------------------------------------------\n");

    for (ssize_t i = 0; i < cnt; i++)
        printdev(devs[i], (int)i + 1);

    printf("===========================================================\n");

    libusb_free_device_list(devs, 1);
    libusb_exit(ctx);
    return 0;
}