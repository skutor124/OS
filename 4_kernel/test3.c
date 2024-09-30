#include <linux/module.h> // Нужен для работы с модулями ядра
#include <linux/proc_fs.h>// Для работы с /proc
#include <linux/timekeeping.h>// Для работы со временем
#include <linux/seq_file.h>// Для создания последовательных файлов для упрощения вывода данных в /proc

static struct proc_dir_entry *tsulab_file;  // Указатель на структуру, представляющую файл в /proc
static int read_count = 0;// счётчик чтений для определения чётности чтения файла

/**
 * tsulab_show - оснвная функция, вызываемая при чтении файла в /proc.
 * Она выводит либо количество минут с полудня, либо до полудня в зависимости от того,
 * сколько раз был прочитан файл.
 */
static int tsulab_show(struct seq_file *m, void *v)
{
    struct timespec64 now;
    ktime_get_real_ts64(&now);  // Получаем текущее время

    // Преобразуем текущее время в количество минут с полуночи UTC
    int current_minutes_utc = (now.tv_sec / 60) % 1440;

    int timezone = 7; // Часовой пояс UTC+7 для Томска/Новосибирска/Красноярска
    //int timezone = 3; // Часовой пояс UTC+3 для Москвы

    
    // Корректируем текущее время с учетом часового пояса
    int current_minutes = current_minutes_utc + (timezone * 60);
    
    // Приводим минуты к 24-часовому формату
    current_minutes = current_minutes % 1440;

    // Минуты с предыдущего полудня
    int minutes_since_noon = current_minutes - 720;
    
    // Если время отрицательное, то корректируем его (для случая, когда время в промежутке [0:00;11:59])
    if (minutes_since_noon < 0) {
        minutes_since_noon += 1440; 
    }

    // Минуты до следующего полудня
    int minutes_until_noon = 1440 - minutes_since_noon;

    read_count++;  // Увеличиваем счётчик чтений

    // В зависимости от чётности выводим время с предыдущего полудня / до следующего полудня 
    if (read_count % 2 == 0) {
        seq_printf(m, "Minutes since previous noon: %d\n", minutes_since_noon);
    } else {
        seq_printf(m, "Minutes until next noon: %d\n", minutes_until_noon);
    }

    return 0;
}

/**
 * tsulab_open - функция, вызываемая при открытии файла в /proc.
 * Использует seq_file для отображения данных.
 * Функция single_open подготавливает файл к чтению и обрабатывает его как последовательный файл с одноразовым выводом
 */
static int tsulab_open(struct inode *inode, struct file *file)
{
    return single_open(file, tsulab_show, NULL);
}

/** 
* Структура proc_ops, используемая для указания функций, которые ядро вызовет при взаимодействии с конкретным файлом в /proc.
* Каждый элемент структуры указывает на соответствующую функцию, обрабатывающую определенную операцию
*/
static const struct proc_ops tsulab_fops = {
    .proc_open = tsulab_open,    // Открытие файла
    .proc_read = seq_read,       // Чтение файла
    .proc_lseek = seq_lseek,     // Управление положением чтения в файле
    .proc_release = single_release,  // Закрытие файла
};

/**
 * tsulab_init - функция инициализации модуля.
 * Создаёт файл в /proc при загрузке модуля.
 * Выводит сообщение в dmesg о создании файла.
 */
static int __init tsulab_init(void)
{
    tsulab_file = proc_create("tsulab", 0, NULL, &tsulab_fops);  // Создание файла в /proc
    if (!tsulab_file) {
        return -ENOMEM;  // Возвращаем ошибку, если файл не был создан
    }
    pr_info("/proc/tsulab created\n");  // Выводим сообщение в dmesg
    return 0;
}

/**
 * tsulab_exit - функция очистки модуля.
 * Удаляет файл из /proc при выгрузке модуля.
 * Выводит сообщение в dmesg о удалении файла.
 */
static void __exit tsulab_exit(void)
{
    proc_remove(tsulab_file);  // Удаление файла из /proc
    pr_info("/proc/tsulab removed\n");  // Выводим сообщение в dmesg
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A proc file showing time since/until noon");

module_init(tsulab_init);
module_exit(tsulab_exit);
