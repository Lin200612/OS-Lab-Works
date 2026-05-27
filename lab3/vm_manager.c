#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define PAGE_SIZE       256     // размер страницы/фрейма (2^8)
#define PAGE_TABLE_SIZE 256     // всего страниц (2^8)
#define FRAME_COUNT     256     // всего фреймов (2^8)
#define TLB_SIZE        16      // количество записей в TLB
#define MEMORY_SIZE     (FRAME_COUNT * PAGE_SIZE)  // 65536 байт

// Запись в TLB
typedef struct {
    uint8_t page_number;
    uint8_t frame_number;
    bool valid;
} TLBEntry;

// Таблица страниц: для каждой страницы хранится номер фрейма и флаг присутствия
typedef struct {
    int frame_number;   // -1 означает отсутствие
    bool valid;
} PageTableEntry;

// Глобальные данные
TLBEntry tlb[TLB_SIZE];
int tlb_index = 0;           // для замещения FIFO

PageTableEntry page_table[PAGE_TABLE_SIZE];
signed char physical_memory[MEMORY_SIZE];
bool frame_allocated[FRAME_COUNT];   // отслеживание занятости фреймов
int next_free_frame = 0;            // для простого выделения, т.к. памяти достаточно

// Статистика
unsigned int tlb_hits = 0;
unsigned int page_faults = 0;
unsigned int total_addresses = 0;

// Прототипы
void init();
uint8_t extract_page(uint32_t logical);
uint8_t extract_offset(uint32_t logical);
int tlb_lookup(uint8_t page);
void tlb_update(uint8_t page, uint8_t frame);
int page_table_lookup(uint8_t page);
int handle_page_fault(uint8_t page);
signed char read_physical(int frame, uint8_t offset);

// Загрузка страницы из резервного хранилища
int load_page_from_backing_store(uint8_t page_number, int frame) {
    FILE *backing = fopen("BACKING_STORE.bin", "rb");
    if (!backing) {
        fprintf(stderr, "Ошибка: не удалось открыть BACKING_STORE.bin\n");
        exit(EXIT_FAILURE);
    }

    // Перемещаемся к нужной странице: page_number * PAGE_SIZE
    if (fseek(backing, page_number * PAGE_SIZE, SEEK_SET) != 0) {
        fprintf(stderr, "Ошибка позиционирования в BACKING_STORE.bin\n");
        fclose(backing);
        exit(EXIT_FAILURE);
    }

    // Читаем 256 байт в выделенный фрейм физической памяти
    size_t bytes_read = fread(physical_memory + frame * PAGE_SIZE, 1, PAGE_SIZE, backing);
    if (bytes_read != PAGE_SIZE) {
        fprintf(stderr, "Ошибка чтения страницы %u из BACKING_STORE.bin\n", page_number);
        fclose(backing);
        exit(EXIT_FAILURE);
    }

    fclose(backing);
    return 0;
}

// Выделение нового фрейма (всегда есть свободные, т.к. 256 фреймов на 256 страниц)
int allocate_frame() {
    while (frame_allocated[next_free_frame]) {
        next_free_frame = (next_free_frame + 1) % FRAME_COUNT;
    }
    frame_allocated[next_free_frame] = true;
    return next_free_frame++;
}

// Инициализация структур
void init() {
    // Очистка TLB
    for (int i = 0; i < TLB_SIZE; i++) {
        tlb[i].valid = false;
    }

    // Инициализация таблицы страниц (все страницы недействительны)
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
        page_table[i].valid = false;
        page_table[i].frame_number = -1;я   
    }

    // Все фреймы свободны
    memset(frame_allocated, 0, sizeof(frame_allocated));

    // Обнуляем физическую память (можно не делать, т.к. будем загружать страницы)
    memset(physical_memory, 0, MEMORY_SIZE);
}

// Извлечение 8-битного номера страницы из 16-битного адреса
uint8_t extract_page(uint32_t logical) {
    return (uint8_t)((logical >> 8) & 0xFF);
}

// Извлечение 8-битного смещения
uint8_t extract_offset(uint32_t logical) {
    return (uint8_t)(logical & 0xFF);
}

// Поиск в TLB: возвращает номер фрейма или -1 при промахе
int tlb_lookup(uint8_t page) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid && tlb[i].page_number == page) {
            return tlb[i].frame_number;
        }
    }
    return -1;
}

// Обновление TLB (FIFO замещение)
void tlb_update(uint8_t page, uint8_t frame) {
    tlb[tlb_index].page_number = page;
    tlb[tlb_index].frame_number = frame;
    tlb[tlb_index].valid = true;
    tlb_index = (tlb_index + 1) % TLB_SIZE;
}

// Поиск в таблице страниц
int page_table_lookup(uint8_t page) {
    if (page_table[page].valid) {
        return page_table[page].frame_number;
    }
    return -1;
}

// Обработка страничной ошибки: выделение фрейма и загрузка страницы
int handle_page_fault(uint8_t page) {
    page_faults++;
    int frame = allocate_frame();
    load_page_from_backing_store(page, frame);
    page_table[page].frame_number = frame;
    page_table[page].valid = true;
    return frame;
}

// Чтение знакового байта из физической памяти по физическому адресу
signed char read_physical(int frame, uint8_t offset) {
    return physical_memory[frame * PAGE_SIZE + offset];
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <файл с логическими адресами>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *addr_file = fopen(argv[1], "r");
    if (!addr_file) {
        perror("Ошибка открытия файла адресов");
        return EXIT_FAILURE;
    }

    init();
    uint32_t logical;
    total_addresses = 0;
    tlb_hits = 0;
    page_faults = 0;

    printf("Трансляция адресов:\n");
    while (fscanf(addr_file, "%u", &logical) == 1) {
        total_addresses++;
        uint16_t masked = (uint16_t)(logical & 0xFFFF);
        uint8_t page   = extract_page(masked);
        uint8_t offset = extract_offset(masked);
        int frame = -1;

        // 1. Поиск в TLB
        int tlb_result = tlb_lookup(page);
        if (tlb_result != -1) {
            tlb_hits++;
            frame = tlb_result;
        } else {
            // 2. Поиск в таблице страниц
            int pt_result = page_table_lookup(page);
            if (pt_result != -1) {
                frame = pt_result;
                // Обновляем TLB
                tlb_update(page, (uint8_t)frame);
            } else {
                // 3. Страничная ошибка
                frame = handle_page_fault(page);
                // Обновляем TLB
                tlb_update(page, (uint8_t)frame);
            }
        }

        // Вычисление физического адреса: (фрейм * 256) + смещение
        uint16_t physical = (uint16_t)((frame << 8) | offset);
        signed char value = read_physical(frame, offset);

        printf("Virtual address: %u Physical address: %u Value: %d\n",
               masked, physical, value);
    }

    fclose(addr_file);

    // Статистика
    double tlb_hit_rate = (total_addresses > 0) ? (100.0 * tlb_hits / total_addresses) : 0.0;
    double pf_rate = (total_addresses > 0) ? (100.0 * page_faults / total_addresses) : 0.0;

    printf("\nСтатистика:\n");
    printf("Частота попаданий в TLB: %.2f%%\n", tlb_hit_rate);
    printf("Частота страничных ошибок: %.2f%%\n", pf_rate);

    return 0;
}